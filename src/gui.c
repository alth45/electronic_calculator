/*
 * gui.c - Window procedure, kontrol UI, dan utilitas GUI
 * ------------------------------------------------------
 * Membangun seluruh kontrol jendela (WM_CREATE), menangani event
 * (WM_COMMAND), serta menyediakan utilitas baca input & sinkronisasi
 * enable/disable field sesuai mode kalkulator.
 */
#include "app.h"
#include <wchar.h>

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
BOOL read_double(HWND edit, double *out)
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
int parse_resistor_list(const wchar_t *text, double *out, int max_count)
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

/* --------------------------------------------------------- sync mode ----- */

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

/* Sinkronkan edit kapasitor sesuai magnitudo yang dicari. */
static void sync_cap_mode(void)
{
    EnableWindow(g_editCapC, g_cap_mode != CAP_C);
    EnableWindow(g_editCapQ, g_cap_mode != CAP_Q);
    EnableWindow(g_editCapV, g_cap_mode != CAP_V);
}

/* --------------------------------------------------------- window proc --- */
LRESULT CALLBACK wnd_proc(HWND hwnd, UINT msg, WPARAM wp, LPARAM lp)
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

        /* ---- Bagian 5: Kapasitansi kapasitor ---- */
        make_ctrl(hwnd, L"BUTTON",
                  L"5. Kapasitansi Kapasitor  (C = Q / V)",
                  BS_GROUPBOX, 12, 644, 476, 160, 0);

        make_ctrl(hwnd, L"BUTTON", L"C = Q / V",
                  BS_AUTORADIOBUTTON | WS_TABSTOP | WS_GROUP,
                  28, 670, 105, 18, ID_CAP_C);
        make_ctrl(hwnd, L"BUTTON", L"Q = C \u00D7 V",
                  BS_AUTORADIOBUTTON | WS_TABSTOP,
                  145, 670, 105, 18, ID_CAP_Q);
        make_ctrl(hwnd, L"BUTTON", L"V = Q / C",
                  BS_AUTORADIOBUTTON | WS_TABSTOP,
                  262, 670, 105, 18, ID_CAP_V);

        make_ctrl(hwnd, L"STATIC", L"Kapasitansi C (F) :",
                  SS_LEFT, 28, 702, 110, 20, 0);
        g_editCapC = make_ctrl(hwnd, L"EDIT", L"",
                               ES_AUTOHSCROLL,
                               142, 699, 150, 23, ID_EDIT_CAP_C);
        make_ctrl(hwnd, L"STATIC", L"Muatan Q (C) :",
                  SS_LEFT, 28, 732, 110, 20, 0);
        g_editCapQ = make_ctrl(hwnd, L"EDIT", L"",
                               ES_AUTOHSCROLL,
                               142, 729, 150, 23, ID_EDIT_CAP_Q);
        make_ctrl(hwnd, L"STATIC", L"Tegangan (V) :",
                  SS_LEFT, 28, 762, 110, 20, 0);
        g_editCapV = make_ctrl(hwnd, L"EDIT", L"",
                               ES_AUTOHSCROLL,
                               142, 759, 150, 23, ID_EDIT_CAP_V);

        make_ctrl(hwnd, L"BUTTON", L"Hitung",
                  0, 330, 704, 100, 30, ID_BTN_CAP);
        g_lblCapResult = make_ctrl(hwnd, L"STATIC",
                  L"Hasil akan\nmuncul di sini",
                  SS_LEFT | SS_SUNKEN,
                  330, 742, 150, 60, ID_LBL_CAP_RESULT);

        /* ---- Keterangan ---- */
        make_ctrl(hwnd, L"STATIC",
                  L"V = tegangan/voltase (volt)  \u2022  I = arus (ampere)  "
                  L"\u2022  R = hambatan (ohm)  \u2022  P = daya (watt)  "
                  L"\u2022  Q = muatan (coulomb)  \u2022  t = waktu (sekon)\n"
                  L"C = kapasitansi (farad)  \u2022  Core: Assembly x86-64 "
                  L"(NASM)  \u2022  GUI: C (Win32)",
                  SS_LEFT, 12, 810, 476, 40, 0);

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

        SetWindowTextW(g_editCapC, L"2");
        SetWindowTextW(g_editCapQ, L"6");
        SetWindowTextW(g_editCapV, L"3");
        CheckRadioButton(hwnd, ID_CAP_C, ID_CAP_V, ID_CAP_C);
        g_cap_mode = CAP_C;
        sync_cap_mode();
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
        if (id == ID_CAP_C || id == ID_CAP_Q || id == ID_CAP_V) {
            g_cap_mode = id - ID_CAP_C;
            sync_cap_mode();
            return 0;
        }
        if (id == ID_BTN_CAP) {
            do_cap_calc(hwnd);
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
