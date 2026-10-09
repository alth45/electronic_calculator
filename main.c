/*
 * main.c - GUI Kalkulator Rangkaian Elektronika
 * ------------------------------------------------
 * GUI ditulis dalam bahasa C memakai Win32 API (Unicode, tanpa framework
 * eksternal). Seluruh logika perhitungan inti dijalankan oleh fungsi
 * Assembly x86-64 dari ohm.asm (lihat circuit_asm.h).
 *
 * Fitur:
 *   1. Hukum Ohm  - cari tegangan (V), arus (I), atau hambatan (R).
 *   2. Gabungan hambatan - rangkaian seri (Rs) dan paralel (Rp).
 *
 * Build: jalankan build.bat (butuh NASM + GCC MinGW-w64).
 */
#ifndef UNICODE
#define UNICODE
#endif
#ifndef _UNICODE
#define _UNICODE
#endif
#include <windows.h>
#include <stdlib.h>
#include <stdio.h>
#include <wchar.h>
#include <math.h>
#include "circuit_asm.h"

/* ------------------------------------------------------------------ ID ---- */
enum {
    ID_MODE_V = 100,
    ID_MODE_I,
    ID_MODE_R,
    ID_EDIT_V,
    ID_EDIT_I,
    ID_EDIT_R,
    ID_BTN_OHM,
    ID_LBL_OHM_RESULT,
    ID_EDIT_RES_LIST,
    ID_COMBO_TOPO,
    ID_BTN_NETWORK,
    ID_LBL_NET_RESULT,
    ID_PWR_VI,
    ID_PWR_I2R,
    ID_PWR_V2R,
    ID_EDIT_PV,
    ID_EDIT_PI,
    ID_EDIT_PR,
    ID_BTN_POWER,
    ID_LBL_PWR_RESULT,
    ID_CHG_Q,
    ID_CHG_I,
    ID_CHG_T,
    ID_EDIT_CHG_Q,
    ID_EDIT_CHG_I,
    ID_EDIT_CHG_T,
    ID_BTN_CHARGE,
    ID_LBL_CHG_RESULT
};

#define MODE_V 0                 /* magnitudo yang dicari: tegangan */
#define MODE_I 1                 /* arus                             */
#define MODE_R 2                 /* hambatan                         */

#define TOPO_SERI 0
#define TOPO_PARALEL 1

#define MAX_RESISTORS 64         /* maksimum jumlah hambatan input */

#define PWR_VI  0                /* rumus daya: P = V × I   */
#define PWR_I2R 1                /* rumus daya: P = I² × R  */
#define PWR_V2R 2                /* rumus daya: P = V² / R  */

#define CHG_Q 0                  /* cari muatan: Q = I × t  */
#define CHG_I 1                  /* cari arus:   I = Q / t  */
#define CHG_T 2                  /* cari waktu:  t = Q / I  */

/* -------------------------------------------------------------- state ---- */
static int   g_mode = MODE_V;
static HWND  g_editV, g_editI, g_editR, g_lblOhmResult;
static HWND  g_editList, g_comboTopo, g_lblNetResult;
static int   g_pwr_mode = PWR_VI;
static HWND  g_editPV, g_editPI, g_editPR, g_lblPwrResult;
static int   g_chg_mode = CHG_Q;
static HWND  g_editCHGQ, g_editCHGI, g_editCHGT, g_lblChgResult;

/* ------------------------------------------------------------- helpers --- */

/* Widget helper: buat kontrol child + set font GUI default. */
static HWND make_ctrl(HWND parent, const wchar_t *cls, const wchar_t *txt,
                      DWORD style, int x, int y, int w, int h, int id)
{
    HWND hw = CreateWindowExW(0, cls, txt,
                              WS_CHILD | WS_VISIBLE | style,
                              x, y, w, h, parent, (HMENU)(INT_PTR)id,
                              NULL, NULL);
    SendMessageW(hw, WM_SETFONT, (WPARAM)GetStockObject(DEFAULT_GUI_FONT),
                 TRUE);
    return hw;
}

/* Baca satu nilai double dari edit box. FALSE jika kosong / bukan angka. */
static BOOL read_double(HWND edit, double *out)
{
    wchar_t  buf[128];
    wchar_t *end;
    double   v;

    GetWindowTextW(edit, buf, 128);
    if (buf[0] == L'\0')
        return FALSE;

    v = wcstod(buf, &end);
    if (end == buf)
        return FALSE;

    while (*end == L' ' || *end == L'\t' || *end == L'\r' || *end == L'\n')
        end++;
    if (*end != L'\0')
        return FALSE;

    *out = v;
    return TRUE;
}

/*
 * Parse daftar hambatan dipisah koma/titik-koma, contoh: "100, 220, 330".
 * Mengembalikan jumlah nilai valid (harus > 0), atau -1 jika ada token
 * yang bukan angka positif atau jumlah token melebihi max_count.
 */
static int parse_resistor_list(const wchar_t *text, double *out, int max_count)
{
    wchar_t  buf[512];
    wchar_t *ctx = NULL;
    wchar_t *tok;
    int      count = 0;

    wcsncpy(buf, text, 511);
    buf[511] = L'\0';

    for (tok = wcstok(buf, L",;", &ctx); tok != NULL;
         tok = wcstok(NULL, L",;", &ctx)) {
        wchar_t *end;
        double   v;

        while (*tok == L' ' || *tok == L'\t')
            tok++;
        if (*tok == L'\0')
            continue;

        v = wcstod(tok, &end);
        if (end == tok)
            return -1;
        while (*end == L' ' || *end == L'\t')
            end++;
        if (*end != L'\0')
            return -1;

        if (!(v > 0.0))             /* <= 0, NaN, atau -inf */
            return -1;
        if (count >= max_count)
            return -1;

        out[count++] = v;
    }
    return count;
}

/* Sinkronkan status enable/disable edit sesuai magnitudo yang dicari. */
static void sync_mode(void)
{
    EnableWindow(g_editV, g_mode != MODE_V);
    EnableWindow(g_editI, g_mode != MODE_I);
    EnableWindow(g_editR, g_mode != MODE_R);
}

/* Sinkronkan edit daya sesuai rumus aktif (field tak terpakai dinonaktifkan). */
static void sync_power_mode(void)
{
    EnableWindow(g_editPV, g_pwr_mode != PWR_I2R);
    EnableWindow(g_editPI, g_pwr_mode != PWR_V2R);
    EnableWindow(g_editPR, g_pwr_mode != PWR_VI);
}

/* Sinkronkan edit muatan sesuai magnitudo yang dicari. */
static void sync_charge_mode(void)
{
    EnableWindow(g_editCHGQ, g_chg_mode != CHG_Q);
    EnableWindow(g_editCHGI, g_chg_mode != CHG_I);
    EnableWindow(g_editCHGT, g_chg_mode != CHG_T);
}

/* ------------------------------------------------------ kalkulator Ohm --- */
static void do_ohm_calc(HWND hwnd)
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
static void do_network_calc(HWND hwnd)
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

/* ------------------------------------------------------ kalkulator daya --- */
static void do_power_calc(HWND hwnd)
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

/* ----------------------------------------------------- kalkulator muatan --- */
static void do_charge_calc(HWND hwnd)
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

/* --------------------------------------------------------- window proc --- */
static LRESULT CALLBACK wnd_proc(HWND hwnd, UINT msg, WPARAM wp, LPARAM lp)
{
    switch (msg) {
    case WM_CREATE: {
        /* ---- Bagian 1: Hukum Ohm ---- */
        make_ctrl(hwnd, L"BUTTON",
                  L"1. Hukum Ohm  (V = I × R,  I = V / R,  R = V / I)",
                  BS_GROUPBOX, 12, 10, 476, 160, 0);

        make_ctrl(hwnd, L"BUTTON", L"Cari Tegangan (V)",
                  BS_AUTORADIOBUTTON | WS_TABSTOP | WS_GROUP,
                  28, 36, 150, 18, ID_MODE_V);
        make_ctrl(hwnd, L"BUTTON", L"Cari Arus (A)",
                  BS_AUTORADIOBUTTON | WS_TABSTOP,
                  185, 36, 110, 18, ID_MODE_I);
        make_ctrl(hwnd, L"BUTTON", L"Cari Hambatan (Ω)",
                  BS_AUTORADIOBUTTON | WS_TABSTOP,
                  305, 36, 150, 18, ID_MODE_R);

        make_ctrl(hwnd, L"STATIC", L"Tegangan (V) :",
                  SS_LEFT, 28, 68, 110, 20, 0);
        g_editV = make_ctrl(hwnd, L"EDIT", L"",
                            ES_AUTOHSCROLL,
                            142, 65, 150, 23, ID_EDIT_V);
        make_ctrl(hwnd, L"STATIC", L"Arus (A) :",
                  SS_LEFT, 28, 98, 110, 20, 0);
        g_editI = make_ctrl(hwnd, L"EDIT", L"",
                            ES_AUTOHSCROLL,
                            142, 95, 150, 23, ID_EDIT_I);
        make_ctrl(hwnd, L"STATIC", L"Hambatan (Ω) :",
                  SS_LEFT, 28, 128, 110, 20, 0);
        g_editR = make_ctrl(hwnd, L"EDIT", L"",
                            ES_AUTOHSCROLL,
                            142, 125, 150, 23, ID_EDIT_R);

        make_ctrl(hwnd, L"BUTTON", L"Hitung",
                  BS_DEFPUSHBUTTON, 330, 66, 100, 30, ID_BTN_OHM);
        g_lblOhmResult = make_ctrl(hwnd, L"STATIC",
                  L"Hasil akan\nmuncul di sini",
                  SS_LEFT | SS_SUNKEN,
                  330, 104, 150, 60, ID_LBL_OHM_RESULT);

        /* ---- Bagian 2: Gabungan hambatan ---- */
        make_ctrl(hwnd, L"BUTTON",
                  L"2. Gabungan Hambatan (Seri / Paralel)",
                  BS_GROUPBOX, 12, 178, 476, 122, 0);

        make_ctrl(hwnd, L"STATIC", L"Nilai (Ω), pisah koma :",
                  SS_LEFT, 28, 204, 130, 20, 0);
        g_editList = make_ctrl(hwnd, L"EDIT", L"",
                  ES_AUTOHSCROLL,
                  162, 201, 300, 23, ID_EDIT_RES_LIST);

        make_ctrl(hwnd, L"STATIC", L"Jenis rangkaian :",
                  SS_LEFT, 28, 234, 130, 20, 0);
        g_comboTopo = make_ctrl(hwnd, L"COMBOBOX", L"",
                  CBS_DROPDOWNLIST | WS_VSCROLL,
                  162, 231, 165, 160, ID_COMBO_TOPO);
        SendMessageW(g_comboTopo, CB_ADDSTRING, 0,
                     (LPARAM)L"Seri  (Rs = R1+R2+...)");
        SendMessageW(g_comboTopo, CB_ADDSTRING, 0,
                     (LPARAM)L"Paralel  (1/Rp = 1/R1+...)");
        SendMessageW(g_comboTopo, CB_SETCURSEL, 0, 0);

        make_ctrl(hwnd, L"BUTTON", L"Hitung",
                  0, 340, 230, 120, 28, ID_BTN_NETWORK);
        g_lblNetResult = make_ctrl(hwnd, L"STATIC",
                  L"Hasil akan muncul di sini",
                  SS_LEFT, 28, 264, 440, 22, ID_LBL_NET_RESULT);

        /* ---- Bagian 3: Daya listrik ---- */
        make_ctrl(hwnd, L"BUTTON",
                  L"3. Daya Listrik  (P = V\u00D7I = I\u00B2\u00D7R = V\u00B2/R)",
                  BS_GROUPBOX, 12, 306, 476, 160, 0);

        make_ctrl(hwnd, L"BUTTON", L"P = V \u00D7 I",
                  BS_AUTORADIOBUTTON | WS_TABSTOP | WS_GROUP,
                  28, 332, 105, 18, ID_PWR_VI);
        make_ctrl(hwnd, L"BUTTON", L"P = I\u00B2 \u00D7 R",
                  BS_AUTORADIOBUTTON | WS_TABSTOP,
                  145, 332, 105, 18, ID_PWR_I2R);
        make_ctrl(hwnd, L"BUTTON", L"P = V\u00B2 / R",
                  BS_AUTORADIOBUTTON | WS_TABSTOP,
                  262, 332, 105, 18, ID_PWR_V2R);

        make_ctrl(hwnd, L"STATIC", L"Tegangan (V) :",
                  SS_LEFT, 28, 364, 110, 20, 0);
        g_editPV = make_ctrl(hwnd, L"EDIT", L"",
                             ES_AUTOHSCROLL,
                             142, 361, 150, 23, ID_EDIT_PV);
        make_ctrl(hwnd, L"STATIC", L"Arus (A) :",
                  SS_LEFT, 28, 394, 110, 20, 0);
        g_editPI = make_ctrl(hwnd, L"EDIT", L"",
                             ES_AUTOHSCROLL,
                             142, 391, 150, 23, ID_EDIT_PI);
        make_ctrl(hwnd, L"STATIC", L"Hambatan (\u03A9) :",
                  SS_LEFT, 28, 424, 110, 20, 0);
        g_editPR = make_ctrl(hwnd, L"EDIT", L"",
                             ES_AUTOHSCROLL,
                             142, 421, 150, 23, ID_EDIT_PR);

        make_ctrl(hwnd, L"BUTTON", L"Hitung",
                  0, 330, 366, 100, 30, ID_BTN_POWER);
        g_lblPwrResult = make_ctrl(hwnd, L"STATIC",
                  L"Hasil akan\nmuncul di sini",
                  SS_LEFT | SS_SUNKEN,
                  330, 404, 150, 60, ID_LBL_PWR_RESULT);

        /* ---- Bagian 4: Muatan listrik ---- */
        make_ctrl(hwnd, L"BUTTON",
                  L"4. Muatan Listrik  (Q = I \u00D7 t)",
                  BS_GROUPBOX, 12, 472, 476, 160, 0);

        make_ctrl(hwnd, L"BUTTON", L"Q = I \u00D7 t",
                  BS_AUTORADIOBUTTON | WS_TABSTOP | WS_GROUP,
                  28, 498, 105, 18, ID_CHG_Q);
        make_ctrl(hwnd, L"BUTTON", L"I = Q / t",
                  BS_AUTORADIOBUTTON | WS_TABSTOP,
                  145, 498, 105, 18, ID_CHG_I);
        make_ctrl(hwnd, L"BUTTON", L"t = Q / I",
                  BS_AUTORADIOBUTTON | WS_TABSTOP,
                  262, 498, 105, 18, ID_CHG_T);

        make_ctrl(hwnd, L"STATIC", L"Muatan Q (C) :",
                  SS_LEFT, 28, 530, 110, 20, 0);
        g_editCHGQ = make_ctrl(hwnd, L"EDIT", L"",
                               ES_AUTOHSCROLL,
                               142, 527, 150, 23, ID_EDIT_CHG_Q);
        make_ctrl(hwnd, L"STATIC", L"Arus (A) :",
                  SS_LEFT, 28, 560, 110, 20, 0);
        g_editCHGI = make_ctrl(hwnd, L"EDIT", L"",
                               ES_AUTOHSCROLL,
                               142, 557, 150, 23, ID_EDIT_CHG_I);
        make_ctrl(hwnd, L"STATIC", L"Waktu t (s) :",
                  SS_LEFT, 28, 590, 110, 20, 0);
        g_editCHGT = make_ctrl(hwnd, L"EDIT", L"",
                               ES_AUTOHSCROLL,
                               142, 587, 150, 23, ID_EDIT_CHG_T);

        make_ctrl(hwnd, L"BUTTON", L"Hitung",
                  0, 330, 532, 100, 30, ID_BTN_CHARGE);
        g_lblChgResult = make_ctrl(hwnd, L"STATIC",
                  L"Hasil akan\nmuncul di sini",
                  SS_LEFT | SS_SUNKEN,
                  330, 570, 150, 60, ID_LBL_CHG_RESULT);

        /* ---- Keterangan ---- */
        make_ctrl(hwnd, L"STATIC",
                  L"V = tegangan/voltase (volt)  \u2022  I = arus (ampere)  "
                  L"\u2022  R = hambatan (ohm)  \u2022  P = daya (watt)  "
                  L"\u2022  Q = muatan (coulomb)  \u2022  t = waktu (sekon)\n"
                  L"Core perhitungan: Assembly x86-64 (NASM)  \u2022  "
                  L"GUI: C (Win32)",
                  SS_LEFT, 12, 638, 476, 40, 0);

        /* Nilai awal contoh + mode default */
        SetWindowTextW(g_editI, L"0.5");
        SetWindowTextW(g_editR, L"100");
        CheckRadioButton(hwnd, ID_MODE_V, ID_MODE_R, ID_MODE_V);
        g_mode = MODE_V;
        sync_mode();

        SetWindowTextW(g_editPV, L"220");
        SetWindowTextW(g_editPI, L"0.5");
        CheckRadioButton(hwnd, ID_PWR_VI, ID_PWR_V2R, ID_PWR_VI);
        g_pwr_mode = PWR_VI;
        sync_power_mode();

        SetWindowTextW(g_editCHGQ, L"6");
        SetWindowTextW(g_editCHGI, L"2");
        SetWindowTextW(g_editCHGT, L"3");
        CheckRadioButton(hwnd, ID_CHG_Q, ID_CHG_T, ID_CHG_Q);
        g_chg_mode = CHG_Q;
        sync_charge_mode();
        return 0;
    }

    case WM_COMMAND: {
        int id = LOWORD(wp);

        if (id == ID_MODE_V || id == ID_MODE_I || id == ID_MODE_R) {
            g_mode = id - ID_MODE_V;
            sync_mode();
            return 0;
        }
        if (id == ID_BTN_OHM) {
            do_ohm_calc(hwnd);
            return 0;
        }
        if (id == ID_BTN_NETWORK) {
            do_network_calc(hwnd);
            return 0;
        }
        if (id == ID_PWR_VI || id == ID_PWR_I2R || id == ID_PWR_V2R) {
            g_pwr_mode = id - ID_PWR_VI;
            sync_power_mode();
            return 0;
        }
        if (id == ID_BTN_POWER) {
            do_power_calc(hwnd);
            return 0;
        }
        if (id == ID_CHG_Q || id == ID_CHG_I || id == ID_CHG_T) {
            g_chg_mode = id - ID_CHG_Q;
            sync_charge_mode();
            return 0;
        }
        if (id == ID_BTN_CHARGE) {
            do_charge_calc(hwnd);
            return 0;
        }
        break;
    }

    case WM_DESTROY:
        PostQuitMessage(0);
        return 0;
    }
    return DefWindowProcW(hwnd, msg, wp, lp);
}

/* -------------------------------------------------------------- entry ---- */
int WINAPI wWinMain(HINSTANCE inst, HINSTANCE prev, PWSTR cmd, int show)
{
    const wchar_t *CLASS_NAME = L"CircuitCalcWnd";
    WNDCLASSW wc;
    HWND      hwnd;
    RECT      rc = { 0, 0, 500, 690 };
    MSG       msg;

    (void)prev; (void)cmd;

    wc.style         = CS_HREDRAW | CS_VREDRAW;
    wc.lpfnWndProc   = wnd_proc;
    wc.cbClsExtra    = 0;
    wc.cbWndExtra    = 0;
    wc.hInstance     = inst;
    wc.hIcon         = LoadIconW(NULL, MAKEINTRESOURCEW(IDI_APPLICATION));
    wc.hCursor       = LoadCursorW(NULL, MAKEINTRESOURCEW(IDC_ARROW));
    wc.hbrBackground = (HBRUSH)(COLOR_BTNFACE + 1);
    wc.lpszMenuName  = NULL;
    wc.lpszClassName = CLASS_NAME;
    RegisterClassW(&wc);

    AdjustWindowRectEx(&rc, WS_OVERLAPPED | WS_CAPTION | WS_SYSMENU |
                            WS_MINIMIZEBOX, FALSE, 0);

    hwnd = CreateWindowExW(0, CLASS_NAME,
                           L"Kalkulator Rangkaian Elektronika (ASM + C)",
                           WS_OVERLAPPED | WS_CAPTION | WS_SYSMENU |
                           WS_MINIMIZEBOX,
                           CW_USEDEFAULT, CW_USEDEFAULT,
                           rc.right - rc.left, rc.bottom - rc.top,
                           NULL, NULL, inst, NULL);
    if (!hwnd)
        return 1;

    ShowWindow(hwnd, show);
    UpdateWindow(hwnd);

    while (GetMessageW(&msg, NULL, 0, 0) > 0) {
        TranslateMessage(&msg);
        DispatchMessageW(&msg);
    }
    return (int)msg.wParam;
}

