/*
 * calc_ac.c - Kalkulator Reaktansi & Impedansi AC
 * -----------------------------------------------
 * Rumus (dipilih lewat radio button):
 *   XL  = 2*pi*f*L              (reaktansi induktif, ohm)
 *   XC  = 1 / (2*pi*f*C)        (reaktansi kapasitif, ohm; f*C = 0 -> error)
 *   RL  : Z = sqrt(R^2 + XL^2)  (impedansi seri RL, ohm)
 *   RC  : Z = sqrt(R^2 + XC^2)  (impedansi seri RC, ohm)
 *   RLC : Z = sqrt(R^2 + (XL - XC)^2)  (impedansi seri RLC, ohm)
 * Logika perhitungan inti ada di core/ohm.asm (lihat circuit_asm.h).
 */
#include "app.h"
#include "circuit_asm.h"
#include <wchar.h>
#include <math.h>

void do_ac_calc(HWND hwnd)
{
    double   f = 0.0, l = 0.0, c = 0.0, r = 0.0;
    double   xl, xc, res;
    wchar_t  msg[128];

    /* Frekuensi selalu dibutuhkan pada semua mode. */
    if (!read_double(g_editAC_F, &f) || f <= 0.0) {
        MessageBoxW(hwnd,
            L"Masukkan frekuensi (Hz) yang valid dan lebih dari nol.",
            L"Input kurang", MB_OK | MB_ICONWARNING);
        return;
    }

    switch (g_ac_mode) {
    case AC_XL:                          /* XL = 2*pi*f*L */
        if (!read_double(g_editAC_L, &l) || l <= 0.0) {
            MessageBoxW(hwnd,
                L"Masukkan induktansi (H) yang valid dan lebih dari nol.",
                L"Input kurang", MB_OK | MB_ICONWARNING);
            return;
        }
        res = calc_xl(f, l);
        break;

    case AC_XC:                          /* XC = 1 / (2*pi*f*C) */
        if (!read_double(g_editAC_C, &c) || c <= 0.0) {
            MessageBoxW(hwnd,
                L"Masukkan kapasitansi (F) yang valid dan lebih dari nol.",
                L"Input kurang", MB_OK | MB_ICONWARNING);
            return;
        }
        res = calc_xc(f, c);
        break;

    case AC_ZRL:                         /* Z = sqrt(R^2 + XL^2) */
        if (!read_double(g_editAC_R, &r) || r < 0.0 ||
            !read_double(g_editAC_L, &l) || l <= 0.0) {
            MessageBoxW(hwnd,
                L"Masukkan hambatan (\u03A9) dan induktansi (H) yang valid.",
                L"Input kurang", MB_OK | MB_ICONWARNING);
            return;
        }
        res = calc_z_series(r, calc_xl(f, l));
        break;

    case AC_ZRC:                         /* Z = sqrt(R^2 + XC^2) */
        if (!read_double(g_editAC_R, &r) || r < 0.0 ||
            !read_double(g_editAC_C, &c) || c <= 0.0) {
            MessageBoxW(hwnd,
                L"Masukkan hambatan (\u03A9) dan kapasitansi (F) yang valid.",
                L"Input kurang", MB_OK | MB_ICONWARNING);
            return;
        }
        res = calc_z_series(r, calc_xc(f, c));
        break;

    default:                             /* AC_ZRLC: Z = sqrt(R^2 + (XL-XC)^2) */
        if (!read_double(g_editAC_R, &r) || r < 0.0 ||
            !read_double(g_editAC_L, &l) || l <= 0.0 ||
            !read_double(g_editAC_C, &c) || c <= 0.0) {
            MessageBoxW(hwnd,
                L"Masukkan hambatan (\u03A9), induktansi (H), dan "
                L"kapasitansi (F) yang valid.",
                L"Input kurang", MB_OK | MB_ICONWARNING);
            return;
        }
        xl = calc_xl(f, l);
        xc = calc_xc(f, c);
        res = calc_z_series(r, xl - xc);
        break;
    }

    if (isnan(res)) {
        SetWindowTextW(g_lblAcResult,
            L"Error: pembagian nol\n(C atau f = 0 pada XC = 1/2\u03C0fC)");
    } else if (g_ac_mode == AC_XL) {
        swprintf(msg, 128, L"XL = %.6g \u03A9", res);
        SetWindowTextW(g_lblAcResult, msg);
    } else if (g_ac_mode == AC_XC) {
        swprintf(msg, 128, L"XC = %.6g \u03A9", res);
        SetWindowTextW(g_lblAcResult, msg);
    } else {
        swprintf(msg, 128, L"Z = %.6g \u03A9", res);
        SetWindowTextW(g_lblAcResult, msg);
    }
}
