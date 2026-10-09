/*
 * calc_ohm.c - Kalkulator hukum Ohm & gabungan hambatan
 * ------------------------------------------------------
 * Logika perhitungan dipanggil dari core/ohm.asm (lihat circuit_asm.h).
 */
#include "app.h"
#include "circuit_asm.h"
#include <wchar.h>
#include <math.h>

/* ------------------------------------------------------ kalkulator Ohm --- */
void do_ohm_calc(HWND hwnd)
{
    double   v = 0.0, i = 0.0, r = 0.0, res;
    wchar_t  msg[128];

    if (g_mode == MODE_V) {
        if (!read_double(g_editI, &i) || !read_double(g_editR, &r)) {
            MessageBoxW(hwnd,
                L"Masukkan nilai arus (A) dan hambatan (Ω) yang valid.",
                L"Input kurang", MB_OK | MB_ICONWARNING);
            return;
        }
        res = calc_voltage(i, r);
        swprintf(msg, 128, L"V = %.6g V", res);
    } else if (g_mode == MODE_I) {
        if (!read_double(g_editV, &v) || !read_double(g_editR, &r)) {
            MessageBoxW(hwnd,
                L"Masukkan nilai tegangan (V) dan hambatan (Ω) yang valid.",
                L"Input kurang", MB_OK | MB_ICONWARNING);
            return;
        }
        res = calc_current(v, r);
        swprintf(msg, 128, L"I = %.6g A", res);
    } else {
        if (!read_double(g_editV, &v) || !read_double(g_editI, &i)) {
            MessageBoxW(hwnd,
                L"Masukkan nilai tegangan (V) dan arus (A) yang valid.",
                L"Input kurang", MB_OK | MB_ICONWARNING);
            return;
        }
        res = calc_resistance(v, i);
        swprintf(msg, 128, L"R = %.6g \u03A9", res);
    }

    if (isnan(res))
        SetWindowTextW(g_lblOhmResult,
            L"Error: pembagian nol\n(periksa nilai Anda)");
    else
        SetWindowTextW(g_lblOhmResult, msg);
}

/* --------------------------------------------- kalkulator rangkaian ------ */
void do_network_calc(HWND hwnd)
{
    double  vals[MAX_RESISTORS];
    wchar_t text[512];
    wchar_t msg[256];
    int     n, topo;
    double  res;

    GetWindowTextW(g_editList, text, 512);
    n = parse_resistor_list(text, vals, MAX_RESISTORS);
    if (n < 1) {
        MessageBoxW(hwnd,
            L"Masukkan minimal satu nilai hambatan > 0,\npisahkan "
            L"beberapa nilai dengan koma. Contoh: 100, 220, 330",
            L"Input tidak valid", MB_OK | MB_ICONWARNING);
        return;
    }

    topo = (int)SendMessageW(g_comboTopo, CB_GETCURSEL, 0, 0);
    if (topo != TOPO_PARALEL)
        topo = TOPO_SERI;

    res = (topo == TOPO_SERI)
        ? calc_series_resistance(vals, n)
        : calc_parallel_resistance(vals, n);

    if (isnan(res)) {
        SetWindowTextW(g_lblNetResult,
            L"Error: input tidak valid (nilai harus > 0)");
        return;
    }

    swprintf(msg, 256, L"%s = %.6g \u03A9  (dari %d hambatan)",
             (topo == TOPO_SERI) ? L"Rs (seri)" : L"Rp (paralel)", res, n);
    SetWindowTextW(g_lblNetResult, msg);
}
