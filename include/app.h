/*
 * app.h - Deklarasi bersama antarmodul GUI
 * -----------------------------------------
 * Berisi ID kontrol, konstanta mode, deklarasi state global,
 * serta prototipe fungsi yang dipakai lintas file di src/.
 */
#ifndef APP_H
#define APP_H

#ifndef UNICODE
#define UNICODE
#endif
#ifndef _UNICODE
#define _UNICODE
#endif

#include <windows.h>
#include <commctrl.h>

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
    ID_LBL_V_OHM,
    ID_LBL_I_OHM,
    ID_LBL_R_OHM,
    ID_EDIT_RES_LIST,
    ID_COMBO_TOPO,
    ID_BTN_NETWORK,
    ID_LBL_NET_RESULT,
    ID_LBL_LIST,
    ID_LBL_TOPO,
    ID_PWR_VI,
    ID_PWR_I2R,
    ID_PWR_V2R,
    ID_EDIT_PV,
    ID_EDIT_PI,
    ID_EDIT_PR,
    ID_BTN_POWER,
    ID_LBL_PWR_RESULT,
    ID_LBL_PV,
    ID_LBL_PI,
    ID_LBL_PR,
    ID_CHG_Q,
    ID_CHG_I,
    ID_CHG_T,
    ID_EDIT_CHG_Q,
    ID_EDIT_CHG_I,
    ID_EDIT_CHG_T,
    ID_BTN_CHARGE,
    ID_LBL_CHG_RESULT,
    ID_LBL_CHG_Q,
    ID_LBL_CHG_I,
    ID_LBL_CHG_T,
    ID_CAP_C,
    ID_CAP_Q,
    ID_CAP_V,
    ID_EDIT_CAP_C,
    ID_EDIT_CAP_Q,
    ID_EDIT_CAP_V,
    ID_BTN_CAP,
    ID_LBL_CAP_RESULT,
    ID_LBL_CAP_C,
    ID_LBL_CAP_Q,
    ID_LBL_CAP_V,
    ID_EC_CV,
    ID_EC_QV,
    ID_EC_QC,
    ID_EDIT_EC_C,
    ID_EDIT_EC_Q,
    ID_EDIT_EC_V,
    ID_BTN_EC,
    ID_LBL_EC_RESULT,
    ID_LBL_EC_C,
    ID_LBL_EC_Q,
    ID_LBL_EC_V,
    ID_EL_LI,
    ID_EL_FI,
    ID_EL_FL,
    ID_EDIT_EL_L,
    ID_EDIT_EL_I,
    ID_EDIT_EL_PSI,
    ID_BTN_EL,
    ID_LBL_EL_RESULT,
    ID_LBL_EL_L,
    ID_LBL_EL_I,
    ID_LBL_EL_PSI,
    ID_GB_OHM,
    ID_GB_NET,
    ID_GB_PWR,
    ID_GB_CHG,
    ID_GB_CAP,
    ID_GB_EC,
    ID_GB_EL,
    ID_TAB,
    ID_LBL_INFO
};

/* --------------------------------------------------------- mode/konstanta - */
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

#define CAP_C 0                  /* cari kapasitansi: C = Q / V */
#define CAP_Q 1                  /* cari muatan:       Q = C × V */
#define CAP_V 2                  /* cari tegangan:     V = Q / C */

#define EC_CV 0                  /* energi kapasitor: Ec = ½ C V² */
#define EC_QV 1                  /*                   Ec = ½ Q V  */
#define EC_QC 2                  /*                   Ec = Q² / 2C */

#define EL_LI 0                  /* energi induktor: El = ½ L I² */
#define EL_FI 1                  /*                 El = ½ Ψ I  */
#define EL_FL 2                  /*                 El = Ψ² / 2L */

#define TAB_COUNT 7              /* jumlah tab kalkulator */

/* ------------------------------------------------ state global (main.c) --- */
extern int   g_mode;
extern HWND  g_editV, g_editI, g_editR, g_lblOhmResult;
extern HWND  g_editList, g_comboTopo, g_lblNetResult;
extern int   g_pwr_mode;
extern HWND  g_editPV, g_editPI, g_editPR, g_lblPwrResult;
extern int   g_chg_mode;
extern HWND  g_editCHGQ, g_editCHGI, g_editCHGT, g_lblChgResult;
extern int   g_cap_mode;
extern HWND  g_editCapC, g_editCapQ, g_editCapV, g_lblCapResult;
extern int   g_e_mode;
extern HWND  g_editECC, g_editECQ, g_editECV, g_lblEcResult;
extern int   g_el_mode;
extern HWND  g_editEL_L, g_editEL_I, g_editEL_PSI, g_lblElResult;
extern HWND  g_tab;

/* ------------------------------------------------------------- gui.c ----- */
BOOL     read_double(HWND edit, double *out);
int      parse_resistor_list(const wchar_t *text, double *out, int max_count);
LRESULT CALLBACK wnd_proc(HWND hwnd, UINT msg, WPARAM wp, LPARAM lp);

/* --------------------------------------------------------- calc_ohm.c ---- */
void do_ohm_calc(HWND hwnd);
void do_network_calc(HWND hwnd);

/* ------------------------------------------------------- calc_power.c ---- */
void do_power_calc(HWND hwnd);

/* ------------------------------------------------------ calc_charge.c ---- */
void do_charge_calc(HWND hwnd);
void do_cap_calc(HWND hwnd);

/* ---------------------------------------------------- calc_energy.c ---- */
void do_energy_calc(HWND hwnd);
void do_ind_energy_calc(HWND hwnd);

#endif /* APP_H */
