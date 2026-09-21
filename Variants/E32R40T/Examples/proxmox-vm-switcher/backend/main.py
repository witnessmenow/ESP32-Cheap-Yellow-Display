"""VM Switcher API for the homelab ESP32 panel.

Endpoints:
  GET  /healthz                           -> liveness, sin auth
  GET  /status                            -> [retrocompat] solo las VMs del grupo iGPU + cual activa
  GET  /machines                          -> TODAS las CTs+VMs (excluyendo EXCLUDE_VMIDS) + flag igpu_group
  POST /start/{vmid}                      -> arranca CT o VM (autodetecta kind)
  POST /stop/{vmid}                       -> apaga CT o VM (graceful: shutdown ACPI / pct shutdown)
  POST /switch/{vmid}                     -> [retrocompat] apaga la activa del grupo iGPU + arranca destino
  POST /shutdown/{vmid}                   -> [retrocompat] alias de /stop
  POST /reset/{vmid}                      -> reset duro de VM (solo VMs)

Auth: X-API-Key header (env API_TOKEN).
Config opcional via env:
  PVE_HOST=root@YOUR_PROXMOX_HOST
  EXCLUDE_VMIDS=100              (csv)
  IGPU_GROUP=102,103,105,107     (csv: VMs que comparten iGPU UHD 630, exclusivas)
"""
from __future__ import annotations

import os
import subprocess
import time
from typing import Optional

from fastapi import FastAPI, HTTPException, Header
from fastapi.middleware.cors import CORSMiddleware

API_TOKEN = os.environ.get("API_TOKEN", "")
HOST = os.environ.get("PVE_HOST", "root@YOUR_PROXMOX_HOST")


def _csv_ids(env_name: str, default: str) -> set[int]:
    raw = os.environ.get(env_name, default)
    out: set[int] = set()
    for tok in raw.split(","):
        tok = tok.strip()
        if not tok:
            continue
        try:
            out.add(int(tok))
        except ValueError:
            pass
    return out


EXCLUDE_VMIDS = _csv_ids("EXCLUDE_VMIDS", "")        # e.g. the VMID of the box running this API
IGPU_GROUP    = _csv_ids("IGPU_GROUP",    "")        # e.g. "102,103": VMs sharing one passthrough iGPU

# Optional short labels shown on the panel for some VMIDs. If a machine is not
# listed here, the firmware shows its Proxmox `name` directly. Example:
#   LABELS = {102: "GAME", 103: "OFFICE"}
LABELS: dict[int, str] = {}

app = FastAPI(title="VM Switcher API", version="0.2.0")
app.add_middleware(CORSMiddleware, allow_origins=["*"], allow_methods=["*"], allow_headers=["*"])


def _auth(x_api_key: Optional[str]) -> None:
    if not API_TOKEN:
        raise HTTPException(503, "API_TOKEN not configured on server")
    if x_api_key != API_TOKEN:
        raise HTTPException(401, "invalid api key")


def _ssh(cmd: str, timeout: int = 10) -> subprocess.CompletedProcess:
    return subprocess.run(
        ["ssh", "-o", "ConnectTimeout=5", "-o", "BatchMode=yes", HOST, cmd],
        capture_output=True, text=True, timeout=timeout,
    )


def _parse_qm_list(stdout: str) -> list[tuple[int, str, str]]:
    """`qm list` -> [(vmid, status, name)].

    Cabecera: 'VMID NAME STATUS MEM(MB) BOOTDISK(GB) PID' (6 columnas, fijas)."""
    out: list[tuple[int, str, str]] = []
    for line in stdout.splitlines()[1:]:
        parts = line.split()
        if len(parts) < 3:
            continue
        try:
            vmid = int(parts[0])
        except ValueError:
            continue
        out.append((vmid, parts[2].lower(), parts[1]))
    return out


def _parse_pct_list(stdout: str) -> list[tuple[int, str, str]]:
    """`pct list` -> [(vmid, status, name)].

    Cabecera: 'VMID Status Lock Name'. 'Lock' suele estar vacío -> con split()
    desaparece y la fila pasa de 4 a 3 tokens; el name siempre es el último."""
    out: list[tuple[int, str, str]] = []
    for line in stdout.splitlines()[1:]:
        parts = line.split()
        if len(parts) < 3:
            continue
        try:
            vmid = int(parts[0])
        except ValueError:
            continue
        out.append((vmid, parts[1].lower(), parts[-1]))
    return out


def _list_all() -> list[dict]:
    """[{kind, id, name, status, label?, igpu_group}] con TODAS las CTs+VMs visibles.
    Hace 2 llamadas SSH (qm list, pct list); coste total ~1-2 s."""
    items: list[dict] = []
    rq = _ssh("qm list")
    if rq.returncode == 0:
        for vmid, status, name in _parse_qm_list(rq.stdout):
            if vmid in EXCLUDE_VMIDS:
                continue
            items.append({
                "kind": "VM", "id": vmid, "name": name, "status": status,
                "label": LABELS.get(vmid),
                "igpu_group": vmid in IGPU_GROUP,
            })
    rp = _ssh("pct list")
    if rp.returncode == 0:
        for vmid, status, name in _parse_pct_list(rp.stdout):
            if vmid in EXCLUDE_VMIDS:
                continue
            items.append({
                "kind": "CT", "id": vmid, "name": name, "status": status,
                "label": LABELS.get(vmid),
                "igpu_group": False,
            })
    items.sort(key=lambda m: m["id"])
    return items


def _kind_of(vmid: int, listing: Optional[list[dict]] = None) -> Optional[str]:
    """Detecta si VMID corresponde a CT o VM, mirando la lista cacheada o re-listando."""
    if listing is None:
        listing = _list_all()
    for m in listing:
        if m["id"] == vmid:
            return m["kind"]
    return None


def _tool_for(kind: str) -> str:
    return "qm" if kind == "VM" else "pct"


# ---------- Endpoints ----------

@app.get("/healthz")
def healthz():
    return {"ok": True, "time": int(time.time()), "version": app.version}


@app.get("/machines")
def get_machines(x_api_key: Optional[str] = Header(None)):
    """Lista completa para el panel ESP32 (modo nuevo).
    `active_igpu`: VMID del grupo iGPU que esté running, o null."""
    _auth(x_api_key)
    items = _list_all()
    active_igpu: Optional[int] = None
    for m in items:
        if m["igpu_group"] and m["status"] == "running":
            active_igpu = m["id"]
            break
    return {"machines": items, "active_igpu": active_igpu, "n": len(items)}


@app.get("/status")
def get_status(x_api_key: Optional[str] = Header(None)):
    """[Retrocompat] formato antiguo: solo las VMs del grupo iGPU."""
    _auth(x_api_key)
    items = _list_all()
    out_vms = []
    active: Optional[int] = None
    for m in items:
        if not m["igpu_group"]:
            continue
        if m["status"] == "running":
            active = m["id"]
        out_vms.append({
            "id":     m["id"],
            "label":  m["label"] or m["name"],
            "name":   m["name"],
            "status": m["status"],
        })
    return {"vms": out_vms, "active": active}


def _run_detached(unit_prefix: str, vmid: int, shell_cmd: str) -> dict:
    unit = f"{unit_prefix}-{vmid}-{int(time.time())}"
    r = _ssh(f"systemd-run --no-block --unit={unit} bash -c {shell_cmd!r}", timeout=15)
    return {
        "ok": r.returncode == 0,
        "vmid": vmid,
        "unit": unit,
        "stderr": r.stderr.strip() if r.stderr else "",
    }


@app.post("/start/{vmid}")
def start(vmid: int, x_api_key: Optional[str] = Header(None)):
    """Arranca CT o VM (autodetecta kind)."""
    _auth(x_api_key)
    if vmid in EXCLUDE_VMIDS:
        raise HTTPException(403, f"vmid {vmid} excluded by config")
    kind = _kind_of(vmid)
    if kind is None:
        raise HTTPException(404, f"vmid {vmid} not found")
    res = _run_detached("api-start", vmid, f"{_tool_for(kind)} start {vmid}")
    res.update(kind=kind, action="start")
    return res


@app.post("/stop/{vmid}")
def stop(vmid: int, x_api_key: Optional[str] = Header(None)):
    """Apagado ACPI/graceful. VMs: `qm shutdown --timeout 120`. CTs: `pct shutdown --timeout 60`."""
    _auth(x_api_key)
    if vmid in EXCLUDE_VMIDS:
        raise HTTPException(403, f"vmid {vmid} excluded by config")
    kind = _kind_of(vmid)
    if kind is None:
        raise HTTPException(404, f"vmid {vmid} not found")
    if kind == "VM":
        cmd = f"qm shutdown {vmid} --timeout 120 --forceStop 0"
    else:
        cmd = f"pct shutdown {vmid} --timeout 60 --forceStop 0"
    res = _run_detached("api-stop", vmid, cmd)
    res.update(kind=kind, action="stop")
    return res


# ---- retrocompat ----

@app.post("/switch/{vmid}")
def switch(vmid: int, x_api_key: Optional[str] = Header(None)):
    """[Retrocompat] apaga la VM del grupo iGPU que esté activa y arranca `vmid`.
    Sólo válido para VMs del IGPU_GROUP."""
    _auth(x_api_key)
    if vmid not in IGPU_GROUP:
        raise HTTPException(400, f"vmid {vmid} not in IGPU_GROUP {sorted(IGPU_GROUP)}")
    items = _list_all()
    active: Optional[int] = None
    for m in items:
        if m["igpu_group"] and m["status"] == "running":
            active = m["id"]
            break
    if active == vmid:
        return {"ok": True, "msg": "already running", "from": active, "to": vmid}
    if active is None:
        cmd = f"qm start {vmid}"
    else:
        cmd = f"qm shutdown {active} --timeout 120 --forceStop 1 || true; qm start {vmid}"
    res = _run_detached("api-switch", vmid, cmd)
    res.update(action="switch", to=vmid, **({"from": active} if active else {}))
    return res


@app.post("/shutdown/{vmid}")
def shutdown_alias(vmid: int, x_api_key: Optional[str] = Header(None)):
    """[Retrocompat] alias de /stop."""
    return stop(vmid, x_api_key)  # type: ignore[arg-type]


@app.post("/reset/{vmid}")
def reset(vmid: int, x_api_key: Optional[str] = Header(None)):
    """Reset duro (qm reset). Solo VMs; no aplica a CTs."""
    _auth(x_api_key)
    kind = _kind_of(vmid)
    if kind != "VM":
        raise HTTPException(400, f"reset solo aplica a VMs (vmid {vmid} es {kind or 'desconocido'})")
    res = _run_detached("api-reset", vmid, f"qm reset {vmid}")
    res.update(kind=kind, action="reset")
    return res
