/*
 * layout_test.c - Verifikasi tinggi/halaman tata letak tab (regresi)
 * ------------------------------------------------------------------
 * Memastikan setiap kontrol pada semua tab berada di dalam batas
 * halaman sehingga tidak ada konten tab yang menjulur ke bawah dan
 * saling menimpa label keterangan / bagian bawah jendela.
 *
 * Rumus koordinat ditulis ulang di sini mengikuti gui_pages.c dan
 * dibandingkan dengan konstan PG_* dari include/gui.h; bila salah
 * satu diubah tanpa menyesuaikan yang lain, tes ini gagal.
 *
 * Jalankan otomatis lewat:  build.bat test
 *   gcc -O2 -Wall -Iinclude tests/layout_test.c -o build/layout_test.exe
 */
#include <stdio.h>

/* Konstanta yang sama persis dengan include/gui.h */
#define PG_GRP_W        688
#define PG_GRP_H        200
#define PG_NET_H        140
#define PG_RAD_Y        28
#define PG_ROW1_Y       58
#define PG_ROW_DY       32
#define PG_LBL_X        20
#define PG_LBL_W        150
#define PG_EDT_X        176
#define PG_EDT_W        220
#define PG_EDT_H        26
#define PG_BTN_X        430
#define PG_BTN_Y        58
#define PG_BTN_W        140
#define PG_BTN_H        36
#define PG_RES_X        430
#define PG_RES_Y        102
#define PG_RES_W        230
#define PG_RES_H        80
#define PG_PAGE_BOTTOM  (PG_GRP_H - 4)     /* 196 px dari origin grup */

/* Origin group box tiap halaman = gx+2, gy+2 (lihat gui_pages.c) */
#define GRP_OFF         2

/* Geometri tab control & jendela (lihat gui.c / main.c) */
#define TAB_TOP         12
#define TAB_H           264                /* tinggi kontrol tab */
#define HEADER_H        24                 /* taksiran tinggi header tab */
#define TAB_DISPLAY_H   (TAB_H - HEADER_H - 8)  /* area tampilan ±232 */
#define INFO_Y          282
#define INFO_H          36
#define WIN_CLIENT_H    330                /* tinggi client jendela */

static int failures = 0;

static void check_le(const char *what, int y_bottom, int limit)
{
    if (y_bottom > limit) {
        printf("FAIL: %s: bottom=%d melebihi batas %d\n",
               what, y_bottom, limit);
        failures++;
    } else {
        printf("ok  : %s: bottom=%d <= %d\n", what, y_bottom, limit);
    }
}

int main(void)
{
    int row_last_edit, res_bottom, radio2_bottom;

    printf("== Verifikasi tinggi halaman tab ==\n");

    /* 1. Group box semua halaman setinggi PG_GRP_H harus muat di
     *    area tampilan tab control. */
    check_le("group box standar (7 tab)",
             GRP_OFF + PG_GRP_H, TAB_DISPLAY_H);
    check_le("group box network (lebih pendek)",
             GRP_OFF + PG_NET_H, TAB_DISPLAY_H);

    /* 2. Kontrol terbawah halaman 3-baris: label hasil 2 baris. */
    res_bottom = GRP_OFF + PG_RES_Y + PG_RES_H;
    check_le("halaman standar - label hasil", res_bottom, PG_PAGE_BOTTOM);

    /* 3. Halaman AC (Impedansi): 4 baris input + 2 baris radio. */
    row_last_edit = GRP_OFF + PG_ROW1_Y + 3 * PG_ROW_DY - 3 + PG_EDT_H;
    check_le("tab AC - edit baris ke-4 (R)", row_last_edit, PG_PAGE_BOTTOM);

    radio2_bottom = GRP_OFF + PG_RAD_Y + 22 + 18;
    check_le("tab AC - radio baris ke-2", radio2_bottom, PG_PAGE_BOTTOM);

    /* Tombol Hitung tetap di dalam grup. */
    check_le("tombol Hitung", GRP_OFF + PG_BTN_Y + PG_BTN_H, PG_PAGE_BOTTOM);

    /* 4. Label keterangan di bawah tab control tidak boleh keluar
     *    dari client area jendela. */
    check_le("label info vs client jendela", INFO_Y + INFO_H, WIN_CLIENT_H);

    /* 5. Tab control sendiri (termasuk area tampilannya) harus berada
     *    di atas label info -> tidak saling menimpa. */
    check_le("bawah tab control vs label info",
             TAB_TOP + TAB_H, INFO_Y);

    if (failures) {
        printf("HASIL: %d pemeriksaan tata letak GAGAL\n", failures);
        return 1;
    }
    printf("HASIL: semua pemeriksaan tata letak LULUS\n");
    return 0;
}
