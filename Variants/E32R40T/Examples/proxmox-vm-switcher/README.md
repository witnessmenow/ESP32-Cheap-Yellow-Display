# Proxmox VM Switcher — touch panel

Use the E32R40T as a **physical control panel for a Proxmox VE host**: list every VM and
container and start/stop them from the touch screen. It also supports a special "iGPU group"
switch, where only one of several GPU-passthrough VMs may run at a time (tapping one shuts down
the active one and boots the target).

```
  ESP32 panel (WiFi)  ──HTTP /machines,/start,/stop,/switch──►  vm-switcher-api  ──SSH──►  Proxmox host
   touch UI, polls                (X-API-Key auth)               (FastAPI, :8088)          qm / pct
```

The panel is a thin HTTP client; all the Proxmox logic lives in a small FastAPI backend that runs
on any always-on box (or on the Proxmox host itself) and talks to the host over SSH using `qm` and
`pct`.

## Contents
- `firmware/` — PlatformIO project for the E32R40T (TFT_eSPI). Dynamic list of all VMs/CTs with
  scrolling, tap-to-select + ON/OFF, long-press for the iGPU switch, and OTA updates.
- `backend/` — `vm-switcher-api`, a FastAPI service exposing `/machines`, `/start/{vmid}`,
  `/stop/{vmid}`, `/switch/{vmid}`, `/reset/{vmid}`, secured with an `X-API-Key` header.

> No IPs, Wi-Fi credentials or tokens are included — every secret is a placeholder you fill in.

## Backend setup
```bash
cd backend
python3 -m venv venv && ./venv/bin/pip install -r requirements.txt
cp .env.example .env          # then edit .env
./venv/bin/uvicorn main:app --host 0.0.0.0 --port 8088
```
Requirements:
- Passwordless SSH from the backend box to the Proxmox host as a user that can run `qm`/`pct`
  (i.e. `ssh root@YOUR_PROXMOX_HOST "qm list"` must work).
- Set `API_TOKEN` (a random shared secret) and `PVE_HOST` in `.env`.
- To run it as a service, see `backend/vm-switcher-api.service.example`.

## Firmware setup
```bash
cd firmware
cp include/secrets.example.h include/secrets.h   # then edit secrets.h
pio run -t upload                                 # first flash over USB
```
In `secrets.h` set your Wi-Fi, the backend's `API_HOST`/`API_PORT`, and the same `API_TOKEN` as the
backend. TFT_eSPI is configured by the project-local `include/User_Setup.h` (the E32R40T pin map).
After the first USB flash you can update over WiFi with `pio run -t upload --upload-port <panel-ip>`.

## Security notes
- The `API_TOKEN` is a shared secret — keep the backend on a trusted LAN/VPN, not exposed to the
  internet. It gives power to start/stop your VMs.
- The backend runs `qm`/`pct` over SSH as root on the Proxmox host; treat that box accordingly.
- `secrets.h` and `.env` are meant to be gitignored — only the `.example` templates are shared.

---
Contributed by [@chemazener](https://github.com/chemazener).
