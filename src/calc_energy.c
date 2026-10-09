/*
 * calc_energy.c - Kalkulator energi dalam kapasitor
 * --------------------------------------------------
 * Rumus: Ec = ½·C·V²,  Ec = ½·Q·V,  Ec = Q²/(2C)
 * Logika perhitungan dipanggil dari core/ohm.asm (lihat circuit_asm.h).
 */
#include "app.h"
#include "circuit_asm.h"
#include <wchar.h>
#include <math.h>

void do_energy_calc(HWND hwnd)
{
    double   c = 0.0, q = 0.0, v = 0.0, res;
    wchar_t  msg[128];

    if (g_e_mode == EC_CV) {
        if (!read_double(g_editECC, &c) || !read_double(g_editECV, &v)) {
            MessageBoxW(hwnd,
                L"Masukkan kapasitansi (F) dan tegangan (V) yang valid.",
                L"Input kurang", MB_OK | MB_ICONWARNING);
            return;
        }
        res = calc_cap_energy_cv(c, v);
    } else if (g_e_mode == EC_QV) {
        if (!read_double(g_editECQ, &q) || !read_double(g_editECV, &v)) {
            MessageBoxW(hwnd,
                L"Masukkan muatan (C) dan tegangan (V) yang valid.",
                L"Input kurang", MB_OK | MB_ICONWARNING);
            return;
        }
        res = calc_cap_energy_qv(q, v);
    } else {
        if (!read_double(g_editECQ, &q) || !read_double(g_editECC, &c)) {
            MessageBoxW(hwnd,
                L"Masukkan muatan (C) dan kapasitansi (F) yang valid.",
                L"Input kurang", MB_OK | MB_ICONWARNING);
            return;
        }
        res = calc_cap_energy_qc(q, c);
    }

    if (isnan(res)) {
        SetWindowTextW(g_lblEcResult,
            L"Error: pembagian nol\n(C = 0 pada Ec = Q\u00B2/2C)");
    } else {
        swprintf(msg, 128, L"Ec = %.6g J", res);
        SetWindowTextW(g_lblEcResult, msg);
    }
}
