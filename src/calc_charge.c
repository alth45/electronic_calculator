/*
 * calc_charge.c - Kalkulator muatan listrik & kapasitansi kapasitor
 * ------------------------------------------------------------------
 * Muatan:  Q = I × t,  I = Q / t,  t = Q / I
 * Kapasitor: C = Q / V,  Q = C × V,  V = Q / C
 * Logika perhitungan dipanggil dari core/ohm.asm (lihat circuit_asm.h).
 */
#include "app.h"
#include "circuit_asm.h"
#include <wchar.h>
#include <math.h>

/* ----------------------------------------------------- kalkulator muatan --- */
void do_charge_calc(HWND hwnd)
{
    double   q = 0.0, i = 0.0, t = 0.0, res;
    wchar_t  msg[128];

    if (g_chg_mode == CHG_Q) {
        if (!read_double(g_editCHGI, &i) || !read_double(g_editCHGT, &t)) {
            MessageBoxW(hwnd,
                L"Masukkan arus (A) dan waktu (s) yang valid.",
                L"Input kurang", MB_OK | MB_ICONWARNING);
            return;
        }
        res = calc_charge(i, t);
        swprintf(msg, 128, L"Q = %.6g C", res);
    } else if (g_chg_mode == CHG_I) {
        if (!read_double(g_editCHGQ, &q) || !read_double(g_editCHGT, &t)) {
            MessageBoxW(hwnd,
                L"Masukkan muatan (C) dan waktu (s) yang valid.",
                L"Input kurang", MB_OK | MB_ICONWARNING);
            return;
        }
        res = calc_current_from_charge(q, t);
        swprintf(msg, 128, L"I = %.6g A", res);
    } else {
        if (!read_double(g_editCHGQ, &q) || !read_double(g_editCHGI, &i)) {
            MessageBoxW(hwnd,
                L"Masukkan muatan (C) dan arus (A) yang valid.",
                L"Input kurang", MB_OK | MB_ICONWARNING);
            return;
        }
        res = calc_time_from_charge(q, i);
        swprintf(msg, 128, L"t = %.6g s", res);
    }

    if (isnan(res)) {
        SetWindowTextW(g_lblChgResult,
            L"Error: pembagian nol\n(periksa nilai Anda)");
    } else {
        SetWindowTextW(g_lblChgResult, msg);
    }
}

/* ---------------------------------------------- kalkulator kapasitor ------ */
void do_cap_calc(HWND hwnd)
{
    double   c = 0.0, q = 0.0, v = 0.0, res;
    wchar_t  msg[128];

    if (g_cap_mode == CAP_C) {
        if (!read_double(g_editCapQ, &q) || !read_double(g_editCapV, &v)) {
            MessageBoxW(hwnd,
                L"Masukkan muatan (C) dan tegangan (V) yang valid.",
                L"Input kurang", MB_OK | MB_ICONWARNING);
            return;
        }
        res = calc_capacitance(q, v);
        swprintf(msg, 128, L"C = %.6g F", res);
    } else if (g_cap_mode == CAP_Q) {
        if (!read_double(g_editCapC, &c) || !read_double(g_editCapV, &v)) {
            MessageBoxW(hwnd,
                L"Masukkan kapasitansi (F) dan tegangan (V) yang valid.",
                L"Input kurang", MB_OK | MB_ICONWARNING);
            return;
        }
        res = calc_charge_from_capacitance(c, v);
        swprintf(msg, 128, L"Q = %.6g C", res);
    } else {
        if (!read_double(g_editCapQ, &q) || !read_double(g_editCapC, &c)) {
            MessageBoxW(hwnd,
                L"Masukkan muatan (C) dan kapasitansi (F) yang valid.",
                L"Input kurang", MB_OK | MB_ICONWARNING);
            return;
        }
        res = calc_voltage_from_capacitance(q, c);
        swprintf(msg, 128, L"V = %.6g V", res);
    }

    if (isnan(res)) {
        SetWindowTextW(g_lblCapResult,
            L"Error: pembagian nol\n(periksa nilai Anda)");
    } else {
        SetWindowTextW(g_lblCapResult, msg);
    }
}
