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

/* Sinkronkan edit energi kapasitor sesuai rumus aktif. */
static void sync_energy_mode(void)
{
    EnableWindow(g_editECC, g_e_mode != EC_QV);
    EnableWindow(g_editECQ, g_e_mode != EC_CV);
    EnableWindow(g_editECV, g_e_mode != EC_QC);
}

/* Sinkronkan edit energi induktor sesuai rumus aktif. */
static void sync_ind_mode(void)
{
    EnableWindow(g_editEL_L,   g_el_mode != EL_FI);
    EnableWindow(g_editEL_I,   g_el_mode != EL_FL);
    EnableWindow(g_editEL_PSI, g_el_mode != EL_LI);
}

/* --------------------------------------------------------- tab control --- */

/* Petakan ID kontrol ke indeks tab (0..TAB_COUNT-1); -1 = selalu terlihat. */
static int tab_of_id(int id)
{
    if (id >= ID_MODE_V        && id <= ID_LBL_R_OHM)  return 0;
    if (id >= ID_EDIT_RES_LIST && id <= ID_LBL_TOPO)   return 1;
    if (id >= ID_PWR_VI        && id <= ID_LBL_PR)     return 2;
    if (id >= ID_CHG_Q         && id <= ID_LBL_CHG_T)  return 3;
    if (id >= ID_CAP_C         && id <= ID_LBL_CAP_V)  return 4;
    if (id >= ID_EC_CV         && id <= ID_LBL_EC_V)   return 5;
    if (id >= ID_EL_LI         && id <= ID_LBL_EL_PSI) return 6;
    switch (id) {                                     /* group box tiap tab */
    case ID_GB_OHM: return 0;
    case ID_GB_NET: return 1;
    case ID_GB_PWR: return 2;
    case ID_GB_CHG: return 3;
    case ID_GB_CAP: return 4;
    case ID_GB_EC:  return 5;
    case ID_GB_EL:  return 6;
    }
    return -1;                                        /* tab & keterangan */
}

/* Tampilkan hanya kontrol milik tab aktif; sembunyikan sisanya. */
static void show_tab(HWND hwnd, int tab)
{
    HWND ch;
    for (ch = GetWindow(hwnd, GW_CHILD); ch; ch = GetWindow(ch, GW_HWNDNEXT)) {
        int t = tab_of_id(GetDlgCtrlID(ch));
        if (t >= 0)
            ShowWindow(ch, t == tab ? SW_SHOW : SW_HIDE);
    }
}

/* --------------------------------------------------------- window proc --- */
LRESULT CALLBACK wnd_proc(HWND hwnd, UINT msg, WPARAM wp, LPARAM lp)
{
    switch (msg) {
    case WM_CREATE: {
        static const wchar_t *TAB_NAMES[TAB_COUNT] = {
            L"Hukum Ohm", L"Hambatan", L"Daya", L"Muatan",
            L"Kapasitor", L"Energi C", L"Energi L"
        };
        TCITEMW ti;
        RECT    rc;
        int     i, gx, gy;

        /* ---- Tab control: tiap kalkulator satu tab (jendela ringkas) ---- */
        g_tab = make_ctrl(hwnd, WC_TABCONTROL, L"", WS_TABSTOP,
                          8, 8, 544, 208, ID_TAB);
        for (i = 0; i < TAB_COUNT; i++) {
            ZeroMemory(&ti, sizeof ti);
            ti.mask    = TCIF_TEXT;
            ti.pszText = (LPWSTR)TAB_NAMES[i];
            SendMessageW(g_tab, TCM_INSERTITEMW, (WPARAM)i, (LPARAM)&ti);
        }

        /* Area tampilan tab menjadi dasar koordinat tiap section */
        rc.left = 8; rc.top = 8; rc.right = 552; rc.bottom = 216;
        SendMessageW(g_tab, TCM_ADJUSTRECT, TRUE, (LPARAM)&rc);
        gx = rc.left + 8;                   /* origin group box section */
        gy = rc.top + 4;

        /* ---- Tab 1: Hukum Ohm ---- */
        make_ctrl(hwnd, L"BUTTON",
                  L"1. Hukum Ohm  (V = I × R,  I = V / R,  R = V / I)",
                  BS_GROUPBOX, gx, gy, 476, 160, ID_GB_OHM);

        make_ctrl(hwnd, L"BUTTON", L"Cari Tegangan (V)",
                  BS_AUTORADIOBUTTON | WS_TABSTOP | WS_GROUP,
                  gx + 16, gy + 26, 150, 18, ID_MODE_V);
        make_ctrl(hwnd, L"BUTTON", L"Cari Arus (A)",
                  BS_AUTORADIOBUTTON | WS_TABSTOP,
                  gx + 173, gy + 26, 110, 18, ID_MODE_I);
        make_ctrl(hwnd, L"BUTTON", L"Cari Hambatan (Ω)",
                  BS_AUTORADIOBUTTON | WS_TABSTOP,
                  gx + 293, gy + 26, 150, 18, ID_MODE_R);

        make_ctrl(hwnd, L"STATIC", L"Tegangan (V) :",
                  SS_LEFT, gx + 16, gy + 58, 110, 20, ID_LBL_V_OHM);
        g_editV = make_ctrl(hwnd, L"EDIT", L"",
                            ES_AUTOHSCROLL,
                            gx + 130, gy + 55, 150, 23, ID_EDIT_V);
        make_ctrl(hwnd, L"STATIC", L"Arus (A) :",
                  SS_LEFT, gx + 16, gy + 88, 110, 20, ID_LBL_I_OHM);
        g_editI = make_ctrl(hwnd, L"EDIT", L"",
                            ES_AUTOHSCROLL,
                            gx + 130, gy + 85, 150, 23, ID_EDIT_I);
        make_ctrl(hwnd, L"STATIC", L"Hambatan (Ω) :",
                  SS_LEFT, gx + 16, gy + 118, 110, 20, ID_LBL_R_OHM);
        g_editR = make_ctrl(hwnd, L"EDIT", L"",
                            ES_AUTOHSCROLL,
                            gx + 130, gy + 115, 150, 23, ID_EDIT_R);

        make_ctrl(hwnd, L"BUTTON", L"Hitung",
                  BS_DEFPUSHBUTTON, gx + 318, gy + 56, 100, 30, ID_BTN_OHM);
        g_lblOhmResult = make_ctrl(hwnd, L"STATIC",
                  L"Hasil akan\nmuncul di sini",
                  SS_LEFT | SS_SUNKEN,
                  gx + 318, gy + 94, 150, 60, ID_LBL_OHM_RESULT);

        /* ---- Tab 2: Gabungan hambatan ---- */
        make_ctrl(hwnd, L"BUTTON",
                  L"2. Gabungan Hambatan (Seri / Paralel)",
                  BS_GROUPBOX, gx, gy, 476, 122, ID_GB_NET);

        make_ctrl(hwnd, L"STATIC", L"Nilai (Ω), pisah koma :",
                  SS_LEFT, gx + 16, gy + 26, 130, 20, ID_LBL_LIST);
        g_editList = make_ctrl(hwnd, L"EDIT", L"",
                  ES_AUTOHSCROLL,
                  gx + 150, gy + 23, 300, 23, ID_EDIT_RES_LIST);

        make_ctrl(hwnd, L"STATIC", L"Jenis rangkaian :",
                  SS_LEFT, gx + 16, gy + 56, 130, 20, ID_LBL_TOPO);
        g_comboTopo = make_ctrl(hwnd, L"COMBOBOX", L"",
                  CBS_DROPDOWNLIST | WS_VSCROLL,
                  gx + 150, gy + 53, 165, 160, ID_COMBO_TOPO);
        SendMessageW(g_comboTopo, CB_ADDSTRING, 0,
                     (LPARAM)L"Seri  (Rs = R1+R2+...)");
        SendMessageW(g_comboTopo, CB_ADDSTRING, 0,
                     (LPARAM)L"Paralel  (1/Rp = 1/R1+...)");
        SendMessageW(g_comboTopo, CB_SETCURSEL, 0, 0);

        make_ctrl(hwnd, L"BUTTON", L"Hitung",
                  0, gx + 328, gy + 52, 120, 28, ID_BTN_NETWORK);
        g_lblNetResult = make_ctrl(hwnd, L"STATIC",
                  L"Hasil akan muncul di sini",
                  SS_LEFT, gx + 16, gy + 86, 440, 22, ID_LBL_NET_RESULT);

        /* ---- Tab 3: Daya listrik ---- */
        make_ctrl(hwnd, L"BUTTON",
                  L"3. Daya Listrik  (P = V\u00D7I = I\u00B2\u00D7R = V\u00B2/R)",
                  BS_GROUPBOX, gx, gy, 476, 160, ID_GB_PWR);

        make_ctrl(hwnd, L"BUTTON", L"P = V \u00D7 I",
                  BS_AUTORADIOBUTTON | WS_TABSTOP | WS_GROUP,
                  gx + 16, gy + 26, 105, 18, ID_PWR_VI);
        make_ctrl(hwnd, L"BUTTON", L"P = I\u00B2 \u00D7 R",
                  BS_AUTORADIOBUTTON | WS_TABSTOP,
                  gx + 133, gy + 26, 105, 18, ID_PWR_I2R);
        make_ctrl(hwnd, L"BUTTON", L"P = V\u00B2 / R",
                  BS_AUTORADIOBUTTON | WS_TABSTOP,
                  gx + 250, gy + 26, 105, 18, ID_PWR_V2R);

        make_ctrl(hwnd, L"STATIC", L"Tegangan (V) :",
                  SS_LEFT, gx + 16, gy + 58, 110, 20, ID_LBL_PV);
        g_editPV = make_ctrl(hwnd, L"EDIT", L"",
                             ES_AUTOHSCROLL,
                             gx + 130, gy + 55, 150, 23, ID_EDIT_PV);
        make_ctrl(hwnd, L"STATIC", L"Arus (A) :",
                  SS_LEFT, gx + 16, gy + 88, 110, 20, ID_LBL_PI);
        g_editPI = make_ctrl(hwnd, L"EDIT", L"",
                             ES_AUTOHSCROLL,
                             gx + 130, gy + 85, 150, 23, ID_EDIT_PI);
        make_ctrl(hwnd, L"STATIC", L"Hambatan (\u03A9) :",
                  SS_LEFT, gx + 16, gy + 118, 110, 20, ID_LBL_PR);
        g_editPR = make_ctrl(hwnd, L"EDIT", L"",
                             ES_AUTOHSCROLL,
                             gx + 130, gy + 115, 150, 23, ID_EDIT_PR);

        make_ctrl(hwnd, L"BUTTON", L"Hitung",
                  0, gx + 318, gy + 58, 100, 30, ID_BTN_POWER);
        g_lblPwrResult = make_ctrl(hwnd, L"STATIC",
                  L"Hasil akan\nmuncul di sini",
                  SS_LEFT | SS_SUNKEN,
                  gx + 318, gy + 96, 150, 60, ID_LBL_PWR_RESULT);

        /* ---- Tab 4: Muatan listrik ---- */
        make_ctrl(hwnd, L"BUTTON",
                  L"4. Muatan Listrik  (Q = I \u00D7 t)",
                  BS_GROUPBOX, gx, gy, 476, 160, ID_GB_CHG);

        make_ctrl(hwnd, L"BUTTON", L"Q = I \u00D7 t",
                  BS_AUTORADIOBUTTON | WS_TABSTOP | WS_GROUP,
                  gx + 16, gy + 26, 105, 18, ID_CHG_Q);
        make_ctrl(hwnd, L"BUTTON", L"I = Q / t",
                  BS_AUTORADIOBUTTON | WS_TABSTOP,
                  gx + 133, gy + 26, 105, 18, ID_CHG_I);
        make_ctrl(hwnd, L"BUTTON", L"t = Q / I",
                  BS_AUTORADIOBUTTON | WS_TABSTOP,
                  gx + 250, gy + 26, 105, 18, ID_CHG_T);

        make_ctrl(hwnd, L"STATIC", L"Muatan Q (C) :",
                  SS_LEFT, gx + 16, gy + 58, 110, 20, ID_LBL_CHG_Q);
        g_editCHGQ = make_ctrl(hwnd, L"EDIT", L"",
                               ES_AUTOHSCROLL,
                               gx + 130, gy + 55, 150, 23, ID_EDIT_CHG_Q);
        make_ctrl(hwnd, L"STATIC", L"Arus (A) :",
                  SS_LEFT, gx + 16, gy + 88, 110, 20, ID_LBL_CHG_I);
        g_editCHGI = make_ctrl(hwnd, L"EDIT", L"",
                               ES_AUTOHSCROLL,
                               gx + 130, gy + 85, 150, 23, ID_EDIT_CHG_I);
        make_ctrl(hwnd, L"STATIC", L"Waktu t (s) :",
                  SS_LEFT, gx + 16, gy + 118, 110, 20, ID_LBL_CHG_T);
        g_editCHGT = make_ctrl(hwnd, L"EDIT", L"",
                               ES_AUTOHSCROLL,
                               gx + 130, gy + 115, 150, 23, ID_EDIT_CHG_T);

        make_ctrl(hwnd, L"BUTTON", L"Hitung",
                  0, gx + 318, gy + 58, 100, 30, ID_BTN_CHARGE);
        g_lblChgResult = make_ctrl(hwnd, L"STATIC",
                  L"Hasil akan\nmuncul di sini",
                  SS_LEFT | SS_SUNKEN,
                  gx + 318, gy + 96, 150, 60, ID_LBL_CHG_RESULT);

        /* ---- Tab 5: Kapasitansi kapasitor ---- */
        make_ctrl(hwnd, L"BUTTON",
                  L"5. Kapasitansi Kapasitor  (C = Q / V)",
                  BS_GROUPBOX, gx, gy, 476, 160, ID_GB_CAP);

        make_ctrl(hwnd, L"BUTTON", L"C = Q / V",
                  BS_AUTORADIOBUTTON | WS_TABSTOP | WS_GROUP,
                  gx + 16, gy + 26, 105, 18, ID_CAP_C);
        make_ctrl(hwnd, L"BUTTON", L"Q = C \u00D7 V",
                  BS_AUTORADIOBUTTON | WS_TABSTOP,
                  gx + 133, gy + 26, 105, 18, ID_CAP_Q);
        make_ctrl(hwnd, L"BUTTON", L"V = Q / C",
                  BS_AUTORADIOBUTTON | WS_TABSTOP,
                  gx + 250, gy + 26, 105, 18, ID_CAP_V);

        make_ctrl(hwnd, L"STATIC", L"Kapasitansi C (F) :",
                  SS_LEFT, gx + 16, gy + 58, 110, 20, ID_LBL_CAP_C);
        g_editCapC = make_ctrl(hwnd, L"EDIT", L"",
                               ES_AUTOHSCROLL,
                               gx + 130, gy + 55, 150, 23, ID_EDIT_CAP_C);
        make_ctrl(hwnd, L"STATIC", L"Muatan Q (C) :",
                  SS_LEFT, gx + 16, gy + 88, 110, 20, ID_LBL_CAP_Q);
        g_editCapQ = make_ctrl(hwnd, L"EDIT", L"",
                               ES_AUTOHSCROLL,
                               gx + 130, gy + 85, 150, 23, ID_EDIT_CAP_Q);
        make_ctrl(hwnd, L"STATIC", L"Tegangan (V) :",
                  SS_LEFT, gx + 16, gy + 118, 110, 20, ID_LBL_CAP_V);
        g_editCapV = make_ctrl(hwnd, L"EDIT", L"",
                               ES_AUTOHSCROLL,
                               gx + 130, gy + 115, 150, 23, ID_EDIT_CAP_V);

        make_ctrl(hwnd, L"BUTTON", L"Hitung",
                  0, gx + 318, gy + 58, 100, 30, ID_BTN_CAP);
        g_lblCapResult = make_ctrl(hwnd, L"STATIC",
                  L"Hasil akan\nmuncul di sini",
                  SS_LEFT | SS_SUNKEN,
                  gx + 318, gy + 96, 150, 60, ID_LBL_CAP_RESULT);

        /* ---- Tab 6: Energi kapasitor ---- */
        make_ctrl(hwnd, L"BUTTON",
                  L"6. Energi Kapasitor  (Ec = \u00BD C\u00B7V\u00B2)",
                  BS_GROUPBOX, gx, gy, 476, 160, ID_GB_EC);

        make_ctrl(hwnd, L"BUTTON", L"Ec = \u00BD C\u00B7V\u00B2",
                  BS_AUTORADIOBUTTON | WS_TABSTOP | WS_GROUP,
                  gx + 16, gy + 26, 105, 18, ID_EC_CV);
        make_ctrl(hwnd, L"BUTTON", L"Ec = \u00BD Q\u00B7V",
                  BS_AUTORADIOBUTTON | WS_TABSTOP,
                  gx + 133, gy + 26, 105, 18, ID_EC_QV);
        make_ctrl(hwnd, L"BUTTON", L"Ec = Q\u00B2 / 2C",
                  BS_AUTORADIOBUTTON | WS_TABSTOP,
                  gx + 250, gy + 26, 105, 18, ID_EC_QC);

        make_ctrl(hwnd, L"STATIC", L"Kapasitansi C (F) :",
                  SS_LEFT, gx + 16, gy + 58, 110, 20, ID_LBL_EC_C);
        g_editECC = make_ctrl(hwnd, L"EDIT", L"",
                              ES_AUTOHSCROLL,
                              gx + 130, gy + 55, 150, 23, ID_EDIT_EC_C);
        make_ctrl(hwnd, L"STATIC", L"Muatan Q (C) :",
                  SS_LEFT, gx + 16, gy + 88, 110, 20, ID_LBL_EC_Q);
        g_editECQ = make_ctrl(hwnd, L"EDIT", L"",
                              ES_AUTOHSCROLL,
                              gx + 130, gy + 85, 150, 23, ID_EDIT_EC_Q);
        make_ctrl(hwnd, L"STATIC", L"Tegangan (V) :",
                  SS_LEFT, gx + 16, gy + 118, 110, 20, ID_LBL_EC_V);
        g_editECV = make_ctrl(hwnd, L"EDIT", L"",
                              ES_AUTOHSCROLL,
                              gx + 130, gy + 115, 150, 23, ID_EDIT_EC_V);

        make_ctrl(hwnd, L"BUTTON", L"Hitung",
                  0, gx + 318, gy + 58, 100, 30, ID_BTN_EC);
        g_lblEcResult = make_ctrl(hwnd, L"STATIC",
                  L"Hasil akan\nmuncul di sini",
                  SS_LEFT | SS_SUNKEN,
                  gx + 318, gy + 96, 150, 60, ID_LBL_EC_RESULT);

        /* ---- Tab 7: Energi induktor ---- */
        make_ctrl(hwnd, L"BUTTON",
                  L"7. Energi Induktor  (El = \u00BD L\u00B7I\u00B2)",
                  BS_GROUPBOX, gx, gy, 476, 160, ID_GB_EL);

        make_ctrl(hwnd, L"BUTTON", L"El = \u00BD L\u00B7I\u00B2",
                  BS_AUTORADIOBUTTON | WS_TABSTOP | WS_GROUP,
                  gx + 16, gy + 26, 105, 18, ID_EL_LI);
        make_ctrl(hwnd, L"BUTTON", L"El = \u00BD \u03A8\u00B7I",
                  BS_AUTORADIOBUTTON | WS_TABSTOP,
                  gx + 133, gy + 26, 105, 18, ID_EL_FI);
        make_ctrl(hwnd, L"BUTTON", L"El = \u03A8\u00B2 / 2L",
                  BS_AUTORADIOBUTTON | WS_TABSTOP,
                  gx + 250, gy + 26, 105, 18, ID_EL_FL);

        make_ctrl(hwnd, L"STATIC", L"Induktansi L (H) :",
                  SS_LEFT, gx + 16, gy + 58, 110, 20, ID_LBL_EL_L);
        g_editEL_L = make_ctrl(hwnd, L"EDIT", L"",
                               ES_AUTOHSCROLL,
                               gx + 130, gy + 55, 150, 23, ID_EDIT_EL_L);
        make_ctrl(hwnd, L"STATIC", L"Arus (A) :",
                  SS_LEFT, gx + 16, gy + 88, 110, 20, ID_LBL_EL_I);
        g_editEL_I = make_ctrl(hwnd, L"EDIT", L"",
                               ES_AUTOHSCROLL,
                               gx + 130, gy + 85, 150, 23, ID_EDIT_EL_I);
        make_ctrl(hwnd, L"STATIC", L"Fluks \u03A8 (Wb) :",
                  SS_LEFT, gx + 16, gy + 118, 110, 20, ID_LBL_EL_PSI);
        g_editEL_PSI = make_ctrl(hwnd, L"EDIT", L"",
                                 ES_AUTOHSCROLL,
                                 gx + 130, gy + 115, 150, 23, ID_EDIT_EL_PSI);

        make_ctrl(hwnd, L"BUTTON", L"Hitung",
                  0, gx + 318, gy + 58, 100, 30, ID_BTN_EL);
        g_lblElResult = make_ctrl(hwnd, L"STATIC",
                  L"Hasil akan\nmuncul di sini",
                  SS_LEFT | SS_SUNKEN,
                  gx + 318, gy + 96, 150, 60, ID_LBL_EL_RESULT);

        /* ---- Keterangan (di bawah tab, selalu terlihat) ---- */
        make_ctrl(hwnd, L"STATIC",
                  L"V = tegangan/voltase (volt)  \u2022  I = arus (ampere)  "
                  L"\u2022  R = hambatan (ohm)  \u2022  P = daya (watt)  "
                  L"\u2022  Q = muatan (coulomb)  \u2022  t = waktu (sekon)\n"
                  L"C = kapasitansi (farad)  \u2022  L = induktansi (henry)  "
                  L"\u2022  \u03A8 = fluks magnet (weber)  \u2022  E = energi (joule)\n"
                  L"Core: Assembly x86-64 (NASM)  \u2022  GUI: C (Win32 + tab control)",
                  SS_LEFT, 12, 222, 536, 44, ID_LBL_INFO);

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

        SetWindowTextW(g_editECC, L"2");
        SetWindowTextW(g_editECV, L"3");
        CheckRadioButton(hwnd, ID_EC_CV, ID_EC_QC, ID_EC_CV);
        g_e_mode = EC_CV;
        sync_energy_mode();

        SetWindowTextW(g_editEL_L, L"2");
        SetWindowTextW(g_editEL_I, L"3");
        CheckRadioButton(hwnd, ID_EL_LI, ID_EL_FL, ID_EL_LI);
        g_el_mode = EL_LI;
        sync_ind_mode();

        /* Tampilkan tab pertama; tab lain tersembunyi */
        show_tab(hwnd, 0);
        return 0;
    }

    case WM_NOTIFY:
        if (((LPNMHDR)lp)->hwndFrom == g_tab &&
            ((LPNMHDR)lp)->code == TCN_SELCHANGE) {
            int sel = (int)SendMessageW(g_tab, TCM_GETCURSEL, 0, 0);
            show_tab(hwnd, sel);
            return 0;
        }
        break;

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
        if (id == ID_EC_CV || id == ID_EC_QV || id == ID_EC_QC) {
            g_e_mode = id - ID_EC_CV;
            sync_energy_mode();
            return 0;
        }
        if (id == ID_BTN_EC) {
            do_energy_calc(hwnd);
            return 0;
        }
        if (id == ID_EL_LI || id == ID_EL_FI || id == ID_EL_FL) {
            g_el_mode = id - ID_EL_LI;
            sync_ind_mode();
            return 0;
        }
        if (id == ID_BTN_EL) {
            do_ind_energy_calc(hwnd);
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
