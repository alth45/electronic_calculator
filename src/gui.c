/*
 * gui.c - Window procedure, event, dan manajemen tab
 * ---------------------------------------------------
 * Membangun tab control + memanggil pembuat halaman (gui_pages.c),
 * menangani event tombol/radio (WM_COMMAND), perpindahan tab
 * (WM_NOTIFY), serta sinkronisasi enable/disable field per mode.
 *
 * Pembagian modul GUI:
 *   include/gui.h     - deklarasi bersama modul GUI
 *   src/gui_widgets.c - utilitas kontrol & parsing input
 *   src/gui_pages.c   - pembuatan halaman (tab) tiap kalkulator
 *   src/gui.c         - file ini (window procedure & event)
 */
#include "gui.h"
#include <wchar.h>

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

/* Sinkronkan edit reaktansi/impedansi AC sesuai rumus aktif. */
static void sync_ac_mode(void)
{
    EnableWindow(g_editAC_F, TRUE);                                   /* selalu dipakai */
    EnableWindow(g_editAC_L, g_ac_mode == AC_XL || g_ac_mode == AC_ZRL ||
                             g_ac_mode == AC_ZRLC);
    EnableWindow(g_editAC_C, g_ac_mode == AC_XC || g_ac_mode == AC_ZRC ||
                             g_ac_mode == AC_ZRLC);
    EnableWindow(g_editAC_R, g_ac_mode == AC_ZRL || g_ac_mode == AC_ZRC ||
                             g_ac_mode == AC_ZRLC);
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
    if (id >= ID_AC_XL         && id <= ID_LBL_AC_R)   return 7;
    if (id >= ID_CODE_CAP      && id <= ID_LBL_CODE_RESULT) return 8;
    switch (id) {                                     /* group box tiap tab */
    case ID_GB_OHM: return 0;
    case ID_GB_NET: return 1;
    case ID_GB_PWR: return 2;
    case ID_GB_CHG: return 3;
    case ID_GB_CAP: return 4;
    case ID_GB_EC:  return 5;
    case ID_GB_EL:  return 6;
    case ID_GB_AC:  return 7;
    case ID_GB_CODE: return 8;
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
            L"Kapasitor", L"Energi C", L"Energi L", L"Impedansi AC",
            L"Kode SMD"
        };
        TCITEMW ti;
        RECT    rc;
        int     i, gx, gy;

        /* Tab control lega: 9 tab muat tanpa scroll (±87 px/tab).
         * Tinggi 264 px -> area tampilan tab ±219 px, cukup untuk
         * halaman tertinggi (grup 200 px) + padding, sehingga tidak ada
         * kontrol tab yang menimpa bagian bawah jendela / label info. */
        g_tab = make_ctrl(hwnd, WC_TABCONTROL, L"", WS_TABSTOP,
                          12, 12, 696, 264, ID_TAB);
        for (i = 0; i < TAB_COUNT; i++) {
            ZeroMemory(&ti, sizeof ti);
            ti.mask    = TCIF_TEXT;
            ti.pszText = (LPWSTR)TAB_NAMES[i];
            SendMessageW(g_tab, TCM_INSERTITEMW, (WPARAM)i, (LPARAM)&ti);
        }

        /* Area tampilan tab (client area di bawah baris tab header) */
        rc.left = 12; rc.top = 12; rc.right = 708; rc.bottom = 276;
        SendMessageW(g_tab, TCM_ADJUSTRECT, TRUE, (LPARAM)&rc);
        gx = rc.left;
        gy = rc.top;

        /* Bangun seluruh halaman pada posisi sama; show_tab memilih aktif */
        create_ohm_page(hwnd, gx, gy);
        create_network_page(hwnd, gx, gy);
        create_power_page(hwnd, gx, gy);
        create_charge_page(hwnd, gx, gy);
        create_cap_page(hwnd, gx, gy);
        create_capenergy_page(hwnd, gx, gy);
        create_indenergy_page(hwnd, gx, gy);
        create_ac_page(hwnd, gx, gy);
        create_code_page(hwnd, gx, gy);
        create_info_label(hwnd, 16, 282, 688);

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

        SetWindowTextW(g_editAC_F, L"50");
        SetWindowTextW(g_editAC_L, L"0.1");
        SetWindowTextW(g_editAC_C, L"0.00001");   /* 10 uF */
        SetWindowTextW(g_editAC_R, L"30");
        CheckRadioButton(hwnd, ID_AC_XL, ID_AC_ZRLC, ID_AC_XL);
        g_ac_mode = AC_XL;
        sync_ac_mode();

        SetWindowTextW(g_editCodeKod, L"104");
        SetWindowTextW(g_editCodeTol, L"J");
        CheckRadioButton(hwnd, ID_CODE_CAP, ID_CODE_IND, ID_CODE_CAP);
        g_code_mode = CODE_CAP;

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
        if (id >= ID_AC_XL && id <= ID_AC_ZRLC) {
            g_ac_mode = id - ID_AC_XL;
            sync_ac_mode();
            return 0;
        }
        if (id == ID_BTN_AC) {
            do_ac_calc(hwnd);
            return 0;
        }
        if (id == ID_CODE_CAP || id == ID_CODE_IND) {
            g_code_mode = id - ID_CODE_CAP;
            return 0;
        }
        if (id == ID_BTN_CODE) {
            do_code_calc(hwnd);
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