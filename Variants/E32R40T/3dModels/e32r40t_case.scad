// ============================================================================
//  Carcasa para placa LCDWIKI E32R40T (ESP32-32E + LCD 4.0" ST7796 320x480)
//  Panel vm-switcher de Chema.  Medidas del plano oficial LCDWIKI V1.0.
//
//  Dos piezas atornilladas con 4 x M3 (aprovechan los taladros Ø3.2 del PCB):
//    - base  : bandeja trasera que aloja el PCB y sus componentes.
//    - bezel : marco frontal con ventana para la pantalla.
//
//  Render:  openscad -D 'part="base"'  -o base.stl  e32r40t_case.scad
//           openscad -D 'part="bezel"' -o bezel.stl e32r40t_case.scad
//  (o abre el archivo en OpenSCAD y cambia `part` abajo)
// ============================================================================

part = "both";   // "base" | "bezel" | "both"  (both = vista montada para revisar)

// ---------------- Medidas de la PLACA (plano LCDWIKI E32R40T) ----------------
pcb_L   = 111.11;   // largo PCB
pcb_W   = 60.88;    // ancho PCB
pcb_t   = 1.60;     // grosor PCB
back_h  = 5.09;     // altura MAX de componentes por la cara trasera

glass_L = 94.57;    // largo del cristal (borde exterior LCD BL), centrado
glass_W = 60.48;    // ancho del cristal
glass_h = 4.05;     // cuanto sobresale el cristal sobre la cara frontal del PCB

hole_dx = 104.11;   // separacion entre taladros (eje largo)
hole_dy = 53.28;    // separacion entre taladros (eje corto)
hole_d  = 3.20;     // diametro taladro del PCB

// ---------------- Parametros de la CARCASA (ajustables) ----------------------
clr        = 0.40;  // holgura PCB<->pared (por lado)
wall       = 2.40;  // grosor de pared
floor_th   = 2.00;  // grosor del suelo de la base
back_gap   = 6.50;  // hueco bajo el PCB para los componentes traseros (>back_h)
lid_gap    = 0.50;  // el bezel queda 0.5 mm por encima del cristal (no lo roza)
corner_r   = 3.00;  // radio de las esquinas exteriores
win_margin = 0.80;  // la ventana descubre el cristal + este margen por lado

post_d      = 7.0;  // diametro de los pilares de sujecion
post_pilot  = 2.55; // pretaladro para autorroscante M3 en el pilar
screw_clr   = 3.40; // paso libre del M3 en el bezel
screw_head  = 6.20; // diametro cabeza (avellanado en el bezel)
head_deep   = 2.20; // profundidad del avellanado

conn_open_w = 48.0; // ancho de la apertura del borde USB-C / botones
vent        = true; // rejillas de ventilacion en el suelo

$fn = 48;

// ---------------- Derivadas ----------------
inner_L = pcb_L + 2*clr;
inner_W = pcb_W + 2*clr;
outer_L = inner_L + 2*wall;
outer_W = inner_W + 2*wall;

base_h  = floor_th + back_gap + pcb_t;   // altura de la base (hasta cara frontal PCB, z=0)
z_pcb_back = -pcb_t;                      // cara trasera del PCB
z_floor    = -(pcb_t + back_gap);         // cara superior del suelo interior
z_bottom   = z_floor - floor_th;          // fondo exterior

bezel_h = glass_h + lid_gap;              // grosor del marco frontal
win_L   = glass_L + 2*win_margin;
win_W   = min(glass_W + 2*win_margin, pcb_W + 2*clr);  // no mas ancho que el hueco

hx = hole_dx/2;   // +-hx , +-hy  posicion de taladros/pilares/tornillos
hy = hole_dy/2;

// ---------------- Utilidades ----------------
// Prisma de esquinas redondeadas centrado en XY, base en z=z0, altura h.
module rbox(l, w, r, h, z0=0) {
    translate([0,0,z0]) linear_extrude(h)
        offset(r=r) square([max(l-2*r,0.01), max(w-2*r,0.01)], center=true);
}

// ============================================================================
//  BASE (bandeja trasera)
// ============================================================================
module base() {
    difference() {
        union() {
            // cuerpo exterior macizo desde el fondo hasta z=0 (cara frontal PCB)
            rbox(outer_L, outer_W, corner_r, base_h, z_bottom);
            // pilares de sujecion (desde el suelo hasta la cara trasera del PCB)
            for (sx=[-1,1], sy=[-1,1])
                translate([sx*hx, sy*hy, z_floor])
                    cylinder(d=post_d, h=back_gap);  // top en z_pcb_back
        }

        // cavidad interior para el PCB y sus componentes (de z_floor a z=0)
        rbox(inner_L, inner_W, corner_r-wall, back_gap+pcb_t+0.1, z_floor);

        // pretaladros de los pilares
        for (sx=[-1,1], sy=[-1,1])
            translate([sx*hx, sy*hy, z_floor-0.1])
                cylinder(d=post_pilot, h=back_gap+0.2);

        // apertura del borde corto (-X) para USB-C + botones RESET/BOOT + salida de cable:
        // ranura en la pared corta, desde el nivel del suelo hasta la cara frontal del PCB.
        translate([-(inner_L/2)-wall-1, -conn_open_w/2, z_floor-0.1])
            cube([wall+2, conn_open_w, back_gap+pcb_t+1.1]);

        // rejillas de ventilacion en el suelo
        if (vent)
            for (i=[-3:3])
                translate([i*12, 0, z_bottom-1])
                    rbox(3, inner_W*0.55, 1.4, floor_th+2);
    }
}

// ============================================================================
//  BEZEL (marco frontal)
// ============================================================================
module bezel() {
    difference() {
        union() {
            // placa frontal (de z=0 a z=bezel_h)
            rbox(outer_L, outer_W, corner_r, bezel_h, 0);
            // labio/faldon interior que encaja dentro de las paredes de la base
            translate([0,0,-3])
                difference() {
                    rbox(inner_L-0.3, inner_W-0.3, corner_r-wall, 3.0, 0);
                    rbox(inner_L-0.3-2*1.4, inner_W-0.3-2*1.4, corner_r-wall, 3.2, -0.1);
                }
        }
        // ventana de la pantalla
        rbox(win_L, win_W, corner_r*0.6, bezel_h+1, -0.5);

        // 4 tornillos M3 con avellanado (pasan por los taladros del PCB a los pilares)
        for (sx=[-1,1], sy=[-1,1]) {
            translate([sx*hx, sy*hy, -0.1])
                cylinder(d=screw_clr, h=bezel_h+0.2);
            translate([sx*hx, sy*hy, bezel_h-head_deep])
                cylinder(d=screw_head, h=head_deep+0.1);
        }

        // recorte del faldon en el borde del USB-C para no tapar la apertura
        translate([-(inner_L/2)-1, -conn_open_w/2, -3.1])
            cube([wall+2, conn_open_w, 3.4]);
    }
}

// ============================================================================
//  Seleccion de pieza
// ============================================================================
if (part == "base")  base();
else if (part == "bezel") color("gray") bezel();
else {  // both: vista montada (la base abajo, el bezel arriba)
    base();
    color("gray") bezel();
    // PCB de referencia (no se imprime)
    %translate([0,0,-pcb_t]) cube([pcb_L, pcb_W, pcb_t], center=true);
    %translate([0,0,0]) cube([glass_L, glass_W, glass_h], center=true);
}
