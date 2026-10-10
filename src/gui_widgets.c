/*
 * gui_widgets.c - Utilitas kontrol GUI & parsing input
 * ----------------------------------------------------
 * Widget helper pembuat kontrol, pembaca angka dari edit box,
 * dan parser daftar hambatan dipisah koma.
 */
#include "gui.h"
#include <wchar.h>

/* Widget helper: buat kontrol child + set font GUI default. */
HWND make_ctrl(HWND parent, const wchar_t *cls, const wchar_t *txt,
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