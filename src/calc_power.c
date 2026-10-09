/*
 * calc_power.c - Kalkulator daya listrik
 * ---------------------------------------
 * Rumus: P = V × I,  P = I² × R,  P = V² / R
 * Logika perhitungan dipanggil dari core/ohm.asm (lihat circuit_asm.h).
 */
#include "app.h"
#include "circuit_asm.h"
#include <wchar.h>
#include <math.h>

void do_power_calc(HWND hwnd)
{
    double   v = 0.0, i = 0.0, r = 0.0, res;
    wchar_t  msg[128];

    if (g_pwr_mode == PWR_VI) {
        if (!read_double(g_editPV, &v) || !read_double(g_editPI, &i)) {
            MessageBoxW(hwnd,
                L"Masukkan tegangan (V) dan arus (A) yang valid.",
                L"Input kurang", MB_OK | MB_ICONWARNING);
            return;
        }
        res = calc_power(v, i);
    } else if (g_pwr_mode == PWR_I2R) {
        if (!read_double(g_editPI, &i) || !read_double(g_editPR, &r)) {
            MessageBoxW(hwnd,
                L"Masukkan arus (A) dan hambatan (\u03A9) yang valid.",
                L"Input kurang", MB_OK | MB_ICONWARNING);
            return;
        }
        res = calc_power_i2r(i, r);
    } else {
        if (!read_double(g_editPV, &v) || !read_double(g_editPR, &r)) {
            MessageBoxW(hwnd,
                L"Masukkan tegangan (V) dan hambatan (\u03A9) yang valid.",
                L"Input kurang", MB_OK | MB_ICONWARNING);
            return;
        }
        res = calc_power_v2r(v, r);
    }

    if (isnan(res)) {
        SetWindowTextW(g_lblPwrResult,
            L"Error: pembagian nol\n(R = 0 pada P = V\u00B2/R)");
    } else {
        swprintf(msg, 128, L"P = %.6g W", res);
        SetWindowTextW(g_lblPwrResult, msg);
    }
}
