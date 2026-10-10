/*
 * calc_energy.c - Kalkulator energi dalam kapasitor & induktor
 * ------------------------------------------------------------
 * Rumus kapasitor: Ec = ½·C·V²,  Ec = ½·Q·V,  Ec = Q²/(2C)
 * Rumus induktor:  El = ½·L·I²,  El = ½·Ψ·I,  El = Ψ²/(2L)
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

/* Hitung energi dalam induktor sesuai mode radio aktif (g_el_mode). */
void do_ind_energy_calc(HWND hwnd)
{
    double   l = 0.0, i = 0.0, psi = 0.0, res;
    wchar_t  msg[128];

    if (g_el_mode == EL_LI) {
        if (!read_double(g_editEL_L, &l) || !read_double(g_editEL_I, &i)) {
            MessageBoxW(hwnd,
                L"Masukkan induktansi (H) dan arus (A) yang valid.",
                L"Input kurang", MB_OK | MB_ICONWARNING);
            return;
        }
        res = calc_ind_energy_li(l, i);
    } else if (g_el_mode == EL_FI) {
        if (!read_double(g_editEL_PSI, &psi) || !read_double(g_editEL_I, &i)) {
            MessageBoxW(hwnd,
                L"Masukkan fluks magnet (Wb) dan arus (A) yang valid.",
                L"Input kurang", MB_OK | MB_ICONWARNING);
            return;
        }
        res = calc_ind_energy_fi(psi, i);
    } else {
        if (!read_double(g_editEL_PSI, &psi) || !read_double(g_editEL_L, &l)) {
            MessageBoxW(hwnd,
                L"Masukkan fluks magnet (Wb) dan induktansi (H) yang valid.",
                L"Input kurang", MB_OK | MB_ICONWARNING);
            return;
        }
        res = calc_ind_energy_fl(psi, l);
    }

    if (isnan(res)) {
        SetWindowTextW(g_lblElResult,
            L"Error: pembagian nol\n(L = 0 pada El = \u03A8\u00B2/2L)");
    } else {
        swprintf(msg, 128, L"El = %.6g J", res);
        SetWindowTextW(g_lblElResult, msg);
    }
}
