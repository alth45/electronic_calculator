/*
 * main.c - Entry point GUI Kalkulator Rangkaian Elektronika
 * ---------------------------------------------------------
 * Berisi definisi state global dan fungsi wWinMain (registrasi window
 * class, pembuatan window, message loop).
 *
 * Pembagian modul:
 *   include/app.h        - deklarasi bersama (ID, mode, state, prototipe)
 *   include/circuit_asm.h- deklarasi fungsi Assembly core
 *   src/gui.c            - window procedure + kontrol UI + utilitas
 *   src/calc_ohm.c       - kalkulator hukum Ohm & gabungan hambatan
 *   src/calc_power.c     - kalkulator daya listrik
 *   src/calc_charge.c    - kalkulator muatan & kapasitansi kapasitor
 *   core/ohm.asm         - logika perhitungan Assembly x86-64
 *
 * Build: jalankan build.bat (butuh NASM + GCC MinGW-w64).
 */
#include "app.h"

/* -------------------------------------------------------------- state ---- */
int   g_mode = MODE_V;
HWND  g_editV, g_editI, g_editR, g_lblOhmResult;
HWND  g_editList, g_comboTopo, g_lblNetResult;
int   g_pwr_mode = PWR_VI;
HWND  g_editPV, g_editPI, g_editPR, g_lblPwrResult;
int   g_chg_mode = CHG_Q;
HWND  g_editCHGQ, g_editCHGI, g_editCHGT, g_lblChgResult;
int   g_cap_mode = CAP_C;
HWND  g_editCapC, g_editCapQ, g_editCapV, g_lblCapResult;
int   g_e_mode = EC_CV;
HWND  g_editECC, g_editECQ, g_editECV, g_lblEcResult;

/* -------------------------------------------------------------- entry ---- */
int WINAPI wWinMain(HINSTANCE inst, HINSTANCE prev, PWSTR cmd, int show)
{
    const wchar_t *CLASS_NAME = L"CircuitCalcWnd";
    WNDCLASSW wc;
    HWND      hwnd;
    RECT      rc = { 0, 0, 988, 548 };
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
