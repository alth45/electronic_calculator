/*
 * gui.h - Deklarasi bersama modul GUI (internal)
 * ----------------------------------------------
 * Dipakai antar file di src/ yang membangun antarmuka:
 *   gui_widgets.c  - utilitas kontrol & parsing input
 *   gui_pages.c    - pembuatan halaman (tab) tiap kalkulator
 *   gui.c          - window procedure, event, show/hide tab
 */
#ifndef GUI_H
#define GUI_H

#include "app.h"

/* ------------------------------------------------- gui_widgets.c -------- */

/* Widget helper: buat kontrol child + set font GUI default. */
HWND make_ctrl(HWND parent, const wchar_t *cls, const wchar_t *txt,
               DWORD style, int x, int y, int w, int h, int id);

/* ------------------------------------------------- gui_pages.c ----------- */

/* Ukuran group box halaman tab (kecuali network, lebih pendek) */
#define PG_GRP_W    688
#define PG_GRP_H    200
#define PG_NET_H    140

/* Koordinat lokal di dalam group box (grid halaman) */
#define PG_RAD_Y    28                       /* baris radio button          */
#define PG_ROW1_Y   64                       /* baris input pertama         */
#define PG_ROW_DY   36                       /* jarak antar baris input     */
#define PG_LBL_X    20                       /* x label                     */
#define PG_LBL_W    150                      /* lebar label                 */
#define PG_EDT_X    176                      /* x edit box                  */
#define PG_EDT_W    220                      /* lebar edit box              */
#define PG_EDT_H    26                       /* tinggi edit box             */
#define PG_BTN_X    430                      /* x tombol Hitung             */
#define PG_BTN_Y    64
#define PG_BTN_W    140
#define PG_BTN_H    36
#define PG_RES_X    430                      /* x label hasil               */
#define PG_RES_Y    108
#define PG_RES_W    230
#define PG_RES_H    72

/* Pembuat halaman; (gx, gy) = origin area tampilan tab control. */
void create_ohm_page(HWND hwnd, int gx, int gy);
void create_network_page(HWND hwnd, int gx, int gy);
void create_power_page(HWND hwnd, int gx, int gy);
void create_charge_page(HWND hwnd, int gx, int gy);
void create_cap_page(HWND hwnd, int gx, int gy);
void create_capenergy_page(HWND hwnd, int gx, int gy);
void create_indenergy_page(HWND hwnd, int gx, int gy);

/* Label keterangan simbol di bawah tab control. */
void create_info_label(HWND hwnd, int x, int y, int w);

#endif /* GUI_H */