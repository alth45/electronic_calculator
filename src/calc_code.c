/*
 * calc_code.c - Kalkulator Kode Kapasitor & Induktor (SMD / keramik)
 * ------------------------------------------------------------------
 * Menerjemahkan kode angka yang tercetak pada badan komponen:
 *   Kapasitor : "104" -> 10 x 10^4 pF = 100.000 pF = 100 nF = 0,1 uF
 *               "22"  -> 22 pF (kode 2 digit tanpa eksponen)
 *   Induktor  : "101" -> 10 x 10^1 nH = 100 nH = 0,1 uH
 * Toleransi (opsional, 1 huruf EIA):
 *   B/E=0,5%  C=0,1%  D=0,25%  F=1%  G=2%  J=5%  K=10%  M=15/20%
 * Aritmetika inti (pangkat 10 & tabel toleransi) ada di core/code.asm;
 * file ini memvalidasi/mengurai teks kode lalu memformat hasilnya.
 */
#include "app.h"
#include "circuit_asm.h"
#include <wchar.h>
#include <math.h>

/* Petakan nilai persen x100 (keluaran asm) ke teks, mis. 50 -> "0,5". */
static void fmt_percent(long long pct100, wchar_t *buf, int bufsz)
{
    if (pct100 % 100 == 0)
        swprintf(buf, bufsz, L"%lld", pct100 / 100);
    else if (pct100 % 10 == 0)
        swprintf(buf, bufsz, L"%lld,%d", pct100 / 100,
                 (int)((pct100 / 10) % 10));
    else
        swprintf(buf, bufsz, L"%lld,%02d", pct100 / 100, (int)(pct100 % 100));
}

/* Susun teks nilai dengan pemisah ribuan gaya Indonesia (100.000). */
static void fmt_grouped(long long v, wchar_t *buf, int bufsz)
{
    wchar_t raw[32];
    int     n, i, j = 0;

    swprintf(raw, 32, L"%lld", v);
    n = (int)wcslen(raw);
    for (i = 0; i < n && j < bufsz - 4; i++) {
        if (i > 0 && (n - i) % 3 == 0)
            buf[j++] = L'.';
        buf[j++] = raw[i];
    }
    buf[j] = L'\0';
}

void do_code_calc(HWND hwnd)
{
    wchar_t  kod[32] = L"", tol[8] = L"";
    wchar_t  val[64], msg[256], pcttxt[16];
    long long pf = 0, nh = 0, pct100;
    int      digits = 0, exp = 0, len, i, is_cap;

    GetWindowTextW(g_editCodeKod, kod, 32);
    GetWindowTextW(g_editCodeTol, tol, 8);

    /* ------------------------------------------------- urai & validasi --- */
    len = (int)wcslen(kod);
    if (len < 2 || len > 4) {
        MessageBoxW(hwnd,
            L"Masukkan kode 2 sampai 4 digit, misalnya \"104\"\n"
            L"(kapasitor) atau \"101\" (induktor).",
            L"Input kurang", MB_OK | MB_ICONWARNING);
        return;
    }
    for (i = 0; i < len; i++) {
        if (kod[i] < L'0' || kod[i] > L'9') {
            MessageBoxW(hwnd,
                L"Kode hanya boleh berisi angka (0-9).",
                L"Input salah", MB_OK | MB_ICONWARNING);
            return;
        }
    }
    if (len == 2) {                    /* "22"  -> 22 pF / 22 nH langsung */
        digits = (kod[0] - L'0') * 10 + (kod[1] - L'0');
        exp    = 0;
    } else if (len == 3) {             /* "ABE" -> digit AB x 10^E        */
        digits = (kod[0] - L'0') * 10 + (kod[1] - L'0');
        exp    = kod[2] - L'0';
    } else {                           /* "ABCE" -> digit ABC x 10^E      */
        digits = (kod[0] - L'0') * 100 + (kod[1] - L'0') * 10 +
                 (kod[2] - L'0');
        exp    = kod[3] - L'0';
    }

    is_cap = (g_code_mode == CODE_CAP);

    /* --------------------------------------- terjemahkan via core asm --- */
    if (is_cap) {
        pf = decode_cap_code(digits, exp);
        if (pf < 0) {
            MessageBoxW(hwnd, L"Gagal menerjemahkan kode.",
                        L"Input salah", MB_OK | MB_ICONWARNING);
            return;
        }
    } else {
        nh = decode_ind_code(digits, exp);
        if (nh < 0) {
            MessageBoxW(hwnd, L"Gagal menerjemahkan kode.",
                        L"Input salah", MB_OK | MB_ICONWARNING);
            return;
        }
    }

    /* ------------------------------ baris hasil: nilai + konversi ------- */
    if (is_cap) {
        fmt_grouped(pf, val, 64);
        if (pf >= 1000000000LL)                  /* >= 1.000 uF -> mF */
            swprintf(msg, 256, L"= %s pF (%.4g mF)", val, pf / 1e9);
        else if (pf >= 1000000LL)                /* >= 1.000 nF -> uF */
            swprintf(msg, 256, L"= %s pF (%.4g uF)", val, pf / 1e6);
        else if (pf >= 1000LL)                   /* >= 1 nF     -> nF */
            swprintf(msg, 256, L"= %s pF (%.4g nF)", val, pf / 1e3);
        else
            swprintf(msg, 256, L"= %s pF", val);
    } else {
        fmt_grouped(nh, val, 64);
        if (nh >= 1000000LL)                     /* >= 1 H */
            swprintf(msg, 256, L"= %s nH (%.4g H)", val, nh / 1e6);
        else if (nh >= 1000LL)                   /* >= 1 uH */
            swprintf(msg, 256, L"= %s nH (%.4g uH)", val, nh / 1e3);
        else
            swprintf(msg, 256, L"= %s nH", val);
    }

    /* ------------------------------------------------- toleransi (opt.) -- */
    if (tol[0] != L'\0') {
        if (tol[1] != L'\0') {
            MessageBoxW(hwnd,
                L"Toleransi cukup 1 huruf, misalnya J, K, atau M.",
                L"Input salah", MB_OK | MB_ICONWARNING);
            return;
        }
        pct100 = tolerance_letter_percent((int)tol[0]);
        if (pct100 < 0) {
            MessageBoxW(hwnd,
                L"Huruf toleransi tidak dikenal.\n"
                L"Pilihan: B(0,5%) C(0,1%) D(0,25%) E(0,5%) F(1%) "
                L"G(2%) J(5%) K(10%) M(15%) N(20%) P(100%)",
                L"Input salah", MB_OK | MB_ICONWARNING);
            return;
        }
        fmt_percent(pct100, pcttxt, 16);
        if (is_cap)
            swprintf(msg + wcslen(msg), 256 - (int)wcslen(msg),
                     L"\n\u00B1%s%% = \u00B1%.4g pF", pcttxt,
                     pf * pct100 / 10000.0);
        else
            swprintf(msg + wcslen(msg), 256 - (int)wcslen(msg),
                     L"\n\u00B1%s%% = \u00B1%.4g nH", pcttxt,
                     nh * pct100 / 10000.0);
    }

    SetWindowTextW(g_lblCodeResult, msg);
}
