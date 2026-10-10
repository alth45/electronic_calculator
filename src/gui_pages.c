/*
 * gui_pages.c - Pembuatan halaman (tab) tiap kalkulator
 * ------------------------------------------------------
 * Setiap create_*_page membangun seluruh kontrol satu tab pada
 * koordinat relatif origin area tampilan tab control (gx, gy).
 * Grid memakai konstan PG_* dari gui.h agar letak seragam.
 */
#include "gui.h"

/* ------------------------------------------------------------------ ohm - */
void create_ohm_page(HWND hwnd, int gx, int gy)
{
    int ox = gx + 2, oy = gy + 2;           /* origin group box */

    make_ctrl(hwnd, L"BUTTON",
              L"1. Hukum Ohm  (V = I \u00D7 R,  I = V / R,  R = V / I)",
              BS_GROUPBOX, ox, oy, PG_GRP_W, PG_GRP_H, ID_GB_OHM);

    make_ctrl(hwnd, L"BUTTON", L"Cari Tegangan (V)",
              BS_AUTORADIOBUTTON | WS_TABSTOP | WS_GROUP,
              ox + PG_LBL_X, oy + PG_RAD_Y, 170, 18, ID_MODE_V);
    make_ctrl(hwnd, L"BUTTON", L"Cari Arus (A)",
              BS_AUTORADIOBUTTON | WS_TABSTOP,
              ox + PG_LBL_X + 180, oy + PG_RAD_Y, 130, 18, ID_MODE_I);
    make_ctrl(hwnd, L"BUTTON", L"Cari Hambatan (\u03A9)",
              BS_AUTORADIOBUTTON | WS_TABSTOP,
              ox + PG_LBL_X + 330, oy + PG_RAD_Y, 170, 18, ID_MODE_R);

    make_ctrl(hwnd, L"STATIC", L"Tegangan (V) :", SS_LEFT,
              ox + PG_LBL_X, oy + PG_ROW1_Y, PG_LBL_W, 20, ID_LBL_V_OHM);
    g_editV = make_ctrl(hwnd, L"EDIT", L"", ES_AUTOHSCROLL,
              ox + PG_EDT_X, oy + PG_ROW1_Y - 3, PG_EDT_W, PG_EDT_H, ID_EDIT_V);
    make_ctrl(hwnd, L"STATIC", L"Arus (A) :", SS_LEFT,
              ox + PG_LBL_X, oy + PG_ROW1_Y + PG_ROW_DY, PG_LBL_W, 20, ID_LBL_I_OHM);
    g_editI = make_ctrl(hwnd, L"EDIT", L"", ES_AUTOHSCROLL,
              ox + PG_EDT_X, oy + PG_ROW1_Y + PG_ROW_DY - 3, PG_EDT_W, PG_EDT_H, ID_EDIT_I);
    make_ctrl(hwnd, L"STATIC", L"Hambatan (\u03A9) :", SS_LEFT,
              ox + PG_LBL_X, oy + PG_ROW1_Y + 2 * PG_ROW_DY, PG_LBL_W, 20, ID_LBL_R_OHM);
    g_editR = make_ctrl(hwnd, L"EDIT", L"", ES_AUTOHSCROLL,
              ox + PG_EDT_X, oy + PG_ROW1_Y + 2 * PG_ROW_DY - 3, PG_EDT_W, PG_EDT_H, ID_EDIT_R);

    make_ctrl(hwnd, L"BUTTON", L"Hitung", BS_DEFPUSHBUTTON,
              ox + PG_BTN_X, oy + PG_BTN_Y, PG_BTN_W, PG_BTN_H, ID_BTN_OHM);
    g_lblOhmResult = make_ctrl(hwnd, L"STATIC", L"Hasil akan\nmuncul di sini",
              SS_LEFT | SS_SUNKEN,
              ox + PG_RES_X, oy + PG_RES_Y, PG_RES_W, PG_RES_H, ID_LBL_OHM_RESULT);
}

/* -------------------------------------------------------------- network - */
void create_network_page(HWND hwnd, int gx, int gy)
{
    int ox = gx + 2, oy = gy + 2;

    make_ctrl(hwnd, L"BUTTON",
              L"2. Gabungan Hambatan (Seri / Paralel)",
              BS_GROUPBOX, ox, oy, PG_GRP_W, PG_NET_H, ID_GB_NET);

    make_ctrl(hwnd, L"STATIC", L"Nilai (\u03A9), dipisah koma :", SS_LEFT,
              ox + PG_LBL_X, oy + 32, 170, 20, ID_LBL_LIST);
    g_editList = make_ctrl(hwnd, L"EDIT", L"", ES_AUTOHSCROLL,
              ox + PG_EDT_X, oy + 29, 360, PG_EDT_H, ID_EDIT_RES_LIST);

    make_ctrl(hwnd, L"STATIC", L"Jenis rangkaian :", SS_LEFT,
              ox + PG_LBL_X, oy + 70, 170, 20, ID_LBL_TOPO);
    g_comboTopo = make_ctrl(hwnd, L"COMBOBOX", L"",
              CBS_DROPDOWNLIST | WS_VSCROLL,
              ox + PG_EDT_X, oy + 67, 240, 180, ID_COMBO_TOPO);
    SendMessageW(g_comboTopo, CB_ADDSTRING, 0,
                 (LPARAM)L"Seri  (Rs = R1+R2+...)");
    SendMessageW(g_comboTopo, CB_ADDSTRING, 0,
                 (LPARAM)L"Paralel  (1/Rp = 1/R1+...)");
    SendMessageW(g_comboTopo, CB_SETCURSEL, 0, 0);

    make_ctrl(hwnd, L"BUTTON", L"Hitung", 0,
              ox + PG_BTN_X, oy + 48, PG_BTN_W, PG_BTN_H, ID_BTN_NETWORK);
    g_lblNetResult = make_ctrl(hwnd, L"STATIC",
              L"Hasil akan muncul di sini",
              SS_LEFT | SS_SUNKEN,
              ox + PG_LBL_X, oy + 104, 640, 26, ID_LBL_NET_RESULT);
}

/* --------------------------------------------------------------- power - */
void create_power_page(HWND hwnd, int gx, int gy)
{
    int ox = gx + 2, oy = gy + 2;

    make_ctrl(hwnd, L"BUTTON",
              L"3. Daya Listrik  (P = V\u00D7I = I\u00B2\u00D7R = V\u00B2/R)",
              BS_GROUPBOX, ox, oy, PG_GRP_W, PG_GRP_H, ID_GB_PWR);

    make_ctrl(hwnd, L"BUTTON", L"P = V \u00D7 I",
              BS_AUTORADIOBUTTON | WS_TABSTOP | WS_GROUP,
              ox + PG_LBL_X, oy + PG_RAD_Y, 130, 18, ID_PWR_VI);
    make_ctrl(hwnd, L"BUTTON", L"P = I\u00B2 \u00D7 R",
              BS_AUTORADIOBUTTON | WS_TABSTOP,
              ox + PG_LBL_X + 140, oy + PG_RAD_Y, 130, 18, ID_PWR_I2R);
    make_ctrl(hwnd, L"BUTTON", L"P = V\u00B2 / R",
              BS_AUTORADIOBUTTON | WS_TABSTOP,
              ox + PG_LBL_X + 280, oy + PG_RAD_Y, 130, 18, ID_PWR_V2R);

    make_ctrl(hwnd, L"STATIC", L"Tegangan (V) :", SS_LEFT,
              ox + PG_LBL_X, oy + PG_ROW1_Y, PG_LBL_W, 20, ID_LBL_PV);
    g_editPV = make_ctrl(hwnd, L"EDIT", L"", ES_AUTOHSCROLL,
              ox + PG_EDT_X, oy + PG_ROW1_Y - 3, PG_EDT_W, PG_EDT_H, ID_EDIT_PV);
    make_ctrl(hwnd, L"STATIC", L"Arus (A) :", SS_LEFT,
              ox + PG_LBL_X, oy + PG_ROW1_Y + PG_ROW_DY, PG_LBL_W, 20, ID_LBL_PI);
    g_editPI = make_ctrl(hwnd, L"EDIT", L"", ES_AUTOHSCROLL,
              ox + PG_EDT_X, oy + PG_ROW1_Y + PG_ROW_DY - 3, PG_EDT_W, PG_EDT_H, ID_EDIT_PI);
    make_ctrl(hwnd, L"STATIC", L"Hambatan (\u03A9) :", SS_LEFT,
              ox + PG_LBL_X, oy + PG_ROW1_Y + 2 * PG_ROW_DY, PG_LBL_W, 20, ID_LBL_PR);
    g_editPR = make_ctrl(hwnd, L"EDIT", L"", ES_AUTOHSCROLL,
              ox + PG_EDT_X, oy + PG_ROW1_Y + 2 * PG_ROW_DY - 3, PG_EDT_W, PG_EDT_H, ID_EDIT_PR);

    make_ctrl(hwnd, L"BUTTON", L"Hitung", 0,
              ox + PG_BTN_X, oy + PG_BTN_Y, PG_BTN_W, PG_BTN_H, ID_BTN_POWER);
    g_lblPwrResult = make_ctrl(hwnd, L"STATIC", L"Hasil akan\nmuncul di sini",
              SS_LEFT | SS_SUNKEN,
              ox + PG_RES_X, oy + PG_RES_Y, PG_RES_W, PG_RES_H, ID_LBL_PWR_RESULT);
}

/* -------------------------------------------------------------- charge - */
void create_charge_page(HWND hwnd, int gx, int gy)
{
    int ox = gx + 2, oy = gy + 2;

    make_ctrl(hwnd, L"BUTTON",
              L"4. Muatan Listrik  (Q = I \u00D7 t)",
              BS_GROUPBOX, ox, oy, PG_GRP_W, PG_GRP_H, ID_GB_CHG);

    make_ctrl(hwnd, L"BUTTON", L"Q = I \u00D7 t",
              BS_AUTORADIOBUTTON | WS_TABSTOP | WS_GROUP,
              ox + PG_LBL_X, oy + PG_RAD_Y, 120, 18, ID_CHG_Q);
    make_ctrl(hwnd, L"BUTTON", L"I = Q / t",
              BS_AUTORADIOBUTTON | WS_TABSTOP,
              ox + PG_LBL_X + 130, oy + PG_RAD_Y, 120, 18, ID_CHG_I);
    make_ctrl(hwnd, L"BUTTON", L"t = Q / I",
              BS_AUTORADIOBUTTON | WS_TABSTOP,
              ox + PG_LBL_X + 260, oy + PG_RAD_Y, 120, 18, ID_CHG_T);

    make_ctrl(hwnd, L"STATIC", L"Muatan Q (C) :", SS_LEFT,
              ox + PG_LBL_X, oy + PG_ROW1_Y, PG_LBL_W, 20, ID_LBL_CHG_Q);
    g_editCHGQ = make_ctrl(hwnd, L"EDIT", L"", ES_AUTOHSCROLL,
              ox + PG_EDT_X, oy + PG_ROW1_Y - 3, PG_EDT_W, PG_EDT_H, ID_EDIT_CHG_Q);
    make_ctrl(hwnd, L"STATIC", L"Arus (A) :", SS_LEFT,
              ox + PG_LBL_X, oy + PG_ROW1_Y + PG_ROW_DY, PG_LBL_W, 20, ID_LBL_CHG_I);
    g_editCHGI = make_ctrl(hwnd, L"EDIT", L"", ES_AUTOHSCROLL,
              ox + PG_EDT_X, oy + PG_ROW1_Y + PG_ROW_DY - 3, PG_EDT_W, PG_EDT_H, ID_EDIT_CHG_I);
    make_ctrl(hwnd, L"STATIC", L"Waktu t (s) :", SS_LEFT,
              ox + PG_LBL_X, oy + PG_ROW1_Y + 2 * PG_ROW_DY, PG_LBL_W, 20, ID_LBL_CHG_T);
    g_editCHGT = make_ctrl(hwnd, L"EDIT", L"", ES_AUTOHSCROLL,
              ox + PG_EDT_X, oy + PG_ROW1_Y + 2 * PG_ROW_DY - 3, PG_EDT_W, PG_EDT_H, ID_EDIT_CHG_T);

    make_ctrl(hwnd, L"BUTTON", L"Hitung", 0,
              ox + PG_BTN_X, oy + PG_BTN_Y, PG_BTN_W, PG_BTN_H, ID_BTN_CHARGE);
    g_lblChgResult = make_ctrl(hwnd, L"STATIC", L"Hasil akan\nmuncul di sini",
              SS_LEFT | SS_SUNKEN,
              ox + PG_RES_X, oy + PG_RES_Y, PG_RES_W, PG_RES_H, ID_LBL_CHG_RESULT);
}

/* ----------------------------------------------------------------- cap - */
void create_cap_page(HWND hwnd, int gx, int gy)
{
    int ox = gx + 2, oy = gy + 2;

    make_ctrl(hwnd, L"BUTTON",
              L"5. Kapasitansi Kapasitor  (C = Q / V)",
              BS_GROUPBOX, ox, oy, PG_GRP_W, PG_GRP_H, ID_GB_CAP);

    make_ctrl(hwnd, L"BUTTON", L"C = Q / V",
              BS_AUTORADIOBUTTON | WS_TABSTOP | WS_GROUP,
              ox + PG_LBL_X, oy + PG_RAD_Y, 120, 18, ID_CAP_C);
    make_ctrl(hwnd, L"BUTTON", L"Q = C \u00D7 V",
              BS_AUTORADIOBUTTON | WS_TABSTOP,
              ox + PG_LBL_X + 130, oy + PG_RAD_Y, 120, 18, ID_CAP_Q);
    make_ctrl(hwnd, L"BUTTON", L"V = Q / C",
              BS_AUTORADIOBUTTON | WS_TABSTOP,
              ox + PG_LBL_X + 260, oy + PG_RAD_Y, 120, 18, ID_CAP_V);

    make_ctrl(hwnd, L"STATIC", L"Kapasitansi C (F) :", SS_LEFT,
              ox + PG_LBL_X, oy + PG_ROW1_Y, PG_LBL_W, 20, ID_LBL_CAP_C);
    g_editCapC = make_ctrl(hwnd, L"EDIT", L"", ES_AUTOHSCROLL,
              ox + PG_EDT_X, oy + PG_ROW1_Y - 3, PG_EDT_W, PG_EDT_H, ID_EDIT_CAP_C);
    make_ctrl(hwnd, L"STATIC", L"Muatan Q (C) :", SS_LEFT,
              ox + PG_LBL_X, oy + PG_ROW1_Y + PG_ROW_DY, PG_LBL_W, 20, ID_LBL_CAP_Q);
    g_editCapQ = make_ctrl(hwnd, L"EDIT", L"", ES_AUTOHSCROLL,
              ox + PG_EDT_X, oy + PG_ROW1_Y + PG_ROW_DY - 3, PG_EDT_W, PG_EDT_H, ID_EDIT_CAP_Q);
    make_ctrl(hwnd, L"STATIC", L"Tegangan (V) :", SS_LEFT,
              ox + PG_LBL_X, oy + PG_ROW1_Y + 2 * PG_ROW_DY, PG_LBL_W, 20, ID_LBL_CAP_V);
    g_editCapV = make_ctrl(hwnd, L"EDIT", L"", ES_AUTOHSCROLL,
              ox + PG_EDT_X, oy + PG_ROW1_Y + 2 * PG_ROW_DY - 3, PG_EDT_W, PG_EDT_H, ID_EDIT_CAP_V);

    make_ctrl(hwnd, L"BUTTON", L"Hitung", 0,
              ox + PG_BTN_X, oy + PG_BTN_Y, PG_BTN_W, PG_BTN_H, ID_BTN_CAP);
    g_lblCapResult = make_ctrl(hwnd, L"STATIC", L"Hasil akan\nmuncul di sini",
              SS_LEFT | SS_SUNKEN,
              ox + PG_RES_X, oy + PG_RES_Y, PG_RES_W, PG_RES_H, ID_LBL_CAP_RESULT);
}

/* --------------------------------------------------------- energy cap --- */
void create_capenergy_page(HWND hwnd, int gx, int gy)
{
    int ox = gx + 2, oy = gy + 2;

    make_ctrl(hwnd, L"BUTTON",
              L"6. Energi Kapasitor  (Ec = \u00BD C\u00B7V\u00B2)",
              BS_GROUPBOX, ox, oy, PG_GRP_W, PG_GRP_H, ID_GB_EC);

    make_ctrl(hwnd, L"BUTTON", L"Ec = \u00BD C\u00B7V\u00B2",
              BS_AUTORADIOBUTTON | WS_TABSTOP | WS_GROUP,
              ox + PG_LBL_X, oy + PG_RAD_Y, 130, 18, ID_EC_CV);
    make_ctrl(hwnd, L"BUTTON", L"Ec = \u00BD Q\u00B7V",
              BS_AUTORADIOBUTTON | WS_TABSTOP,
              ox + PG_LBL_X + 140, oy + PG_RAD_Y, 130, 18, ID_EC_QV);
    make_ctrl(hwnd, L"BUTTON", L"Ec = Q\u00B2 / 2C",
              BS_AUTORADIOBUTTON | WS_TABSTOP,
              ox + PG_LBL_X + 280, oy + PG_RAD_Y, 130, 18, ID_EC_QC);

    make_ctrl(hwnd, L"STATIC", L"Kapasitansi C (F) :", SS_LEFT,
              ox + PG_LBL_X, oy + PG_ROW1_Y, PG_LBL_W, 20, ID_LBL_EC_C);
    g_editECC = make_ctrl(hwnd, L"EDIT", L"", ES_AUTOHSCROLL,
              ox + PG_EDT_X, oy + PG_ROW1_Y - 3, PG_EDT_W, PG_EDT_H, ID_EDIT_EC_C);
    make_ctrl(hwnd, L"STATIC", L"Muatan Q (C) :", SS_LEFT,
              ox + PG_LBL_X, oy + PG_ROW1_Y + PG_ROW_DY, PG_LBL_W, 20, ID_LBL_EC_Q);
    g_editECQ = make_ctrl(hwnd, L"EDIT", L"", ES_AUTOHSCROLL,
              ox + PG_EDT_X, oy + PG_ROW1_Y + PG_ROW_DY - 3, PG_EDT_W, PG_EDT_H, ID_EDIT_EC_Q);
    make_ctrl(hwnd, L"STATIC", L"Tegangan (V) :", SS_LEFT,
              ox + PG_LBL_X, oy + PG_ROW1_Y + 2 * PG_ROW_DY, PG_LBL_W, 20, ID_LBL_EC_V);
    g_editECV = make_ctrl(hwnd, L"EDIT", L"", ES_AUTOHSCROLL,
              ox + PG_EDT_X, oy + PG_ROW1_Y + 2 * PG_ROW_DY - 3, PG_EDT_W, PG_EDT_H, ID_EDIT_EC_V);

    make_ctrl(hwnd, L"BUTTON", L"Hitung", 0,
              ox + PG_BTN_X, oy + PG_BTN_Y, PG_BTN_W, PG_BTN_H, ID_BTN_EC);
    g_lblEcResult = make_ctrl(hwnd, L"STATIC", L"Hasil akan\nmuncul di sini",
              SS_LEFT | SS_SUNKEN,
              ox + PG_RES_X, oy + PG_RES_Y, PG_RES_W, PG_RES_H, ID_LBL_EC_RESULT);
}

/* --------------------------------------------------------- energy ind --- */
void create_indenergy_page(HWND hwnd, int gx, int gy)
{
    int ox = gx + 2, oy = gy + 2;

    make_ctrl(hwnd, L"BUTTON",
              L"7. Energi Induktor  (El = \u00BD L\u00B7I\u00B2)",
              BS_GROUPBOX, ox, oy, PG_GRP_W, PG_GRP_H, ID_GB_EL);

    make_ctrl(hwnd, L"BUTTON", L"El = \u00BD L\u00B7I\u00B2",
              BS_AUTORADIOBUTTON | WS_TABSTOP | WS_GROUP,
              ox + PG_LBL_X, oy + PG_RAD_Y, 130, 18, ID_EL_LI);
    make_ctrl(hwnd, L"BUTTON", L"El = \u00BD \u03A8\u00B7I",
              BS_AUTORADIOBUTTON | WS_TABSTOP,
              ox + PG_LBL_X + 140, oy + PG_RAD_Y, 130, 18, ID_EL_FI);
    make_ctrl(hwnd, L"BUTTON", L"El = \u03A8\u00B2 / 2L",
              BS_AUTORADIOBUTTON | WS_TABSTOP,
              ox + PG_LBL_X + 280, oy + PG_RAD_Y, 130, 18, ID_EL_FL);

    make_ctrl(hwnd, L"STATIC", L"Induktansi L (H) :", SS_LEFT,
              ox + PG_LBL_X, oy + PG_ROW1_Y, PG_LBL_W, 20, ID_LBL_EL_L);
    g_editEL_L = make_ctrl(hwnd, L"EDIT", L"", ES_AUTOHSCROLL,
              ox + PG_EDT_X, oy + PG_ROW1_Y - 3, PG_EDT_W, PG_EDT_H, ID_EDIT_EL_L);
    make_ctrl(hwnd, L"STATIC", L"Arus (A) :", SS_LEFT,
              ox + PG_LBL_X, oy + PG_ROW1_Y + PG_ROW_DY, PG_LBL_W, 20, ID_LBL_EL_I);
    g_editEL_I = make_ctrl(hwnd, L"EDIT", L"", ES_AUTOHSCROLL,
              ox + PG_EDT_X, oy + PG_ROW1_Y + PG_ROW_DY - 3, PG_EDT_W, PG_EDT_H, ID_EDIT_EL_I);
    make_ctrl(hwnd, L"STATIC", L"Fluks \u03A8 (Wb) :", SS_LEFT,
              ox + PG_LBL_X, oy + PG_ROW1_Y + 2 * PG_ROW_DY, PG_LBL_W, 20, ID_LBL_EL_PSI);
    g_editEL_PSI = make_ctrl(hwnd, L"EDIT", L"", ES_AUTOHSCROLL,
              ox + PG_EDT_X, oy + PG_ROW1_Y + 2 * PG_ROW_DY - 3, PG_EDT_W, PG_EDT_H, ID_EDIT_EL_PSI);

    make_ctrl(hwnd, L"BUTTON", L"Hitung", 0,
              ox + PG_BTN_X, oy + PG_BTN_Y, PG_BTN_W, PG_BTN_H, ID_BTN_EL);
    g_lblElResult = make_ctrl(hwnd, L"STATIC", L"Hasil akan\nmuncul di sini",
              SS_LEFT | SS_SUNKEN,
              ox + PG_RES_X, oy + PG_RES_Y, PG_RES_W, PG_RES_H, ID_LBL_EL_RESULT);
}

/* ---------------------------------------------------------------- info - */
void create_info_label(HWND hwnd, int x, int y, int w)
{
    make_ctrl(hwnd, L"STATIC",
              L"V = tegangan/voltase (V)  \u2022  I = arus (A)  "
              L"\u2022  R = hambatan (\u03A9)  \u2022  P = daya (W)  "
              L"\u2022  Q = muatan (C)  \u2022  t = waktu (s)\n"
              L"C = kapasitansi (F)  \u2022  L = induktansi (H)  "
              L"\u2022  \u03A8 = fluks magnet (Wb)  \u2022  E = energi (J)  "
              L"\u2022  Core: Assembly x86-64 (NASM)  \u2022  GUI: C (Win32)",
              SS_LEFT, x, y, w, 36, ID_LBL_INFO);
}
