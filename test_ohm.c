/*
 * test_ohm.c - Uji unit logika Assembly (tanpa GUI)
 * --------------------------------------------------
 * Build & jalankan:  build.bat test
 */
#include <stdio.h>
#include <math.h>
#include "circuit_asm.h"

static int g_fail = 0;

static void checki(const char *name, long long got, long long want)
{
    int ok = (got == want);
    printf("%-32s got=%-14lld want=%-14lld %s\n", name, got, want,
           ok ? "OK" : "FAIL");
    if (!ok)
        g_fail++;
}

static void check(const char *name, double got, double want)
{
    int ok = (isnan(want) && isnan(got)) ||
             (!isnan(want) && !isnan(got) && fabs(got - want) < 1e-9);
    printf("%-32s got=%-14g want=%-14g %s\n", name, got, want, ok ? "OK" : "FAIL");
    if (!ok)
        g_fail++;
}

int main(void)
{
    double seri[]      = { 100.0, 220.0, 330.0 };
    double dua[]       = { 100.0, 100.0 };
    double invalid[]   = { 100.0, 0.0 };
    double negatif[]   = { 50.0, -10.0 };

    puts("=== Uji Hukum Ohm ===");
    check("V(2 A, 100 ohm) = 200 V",   calc_voltage(2.0, 100.0), 200.0);
    check("V(0.5 A, 330 ohm)",         calc_voltage(0.5, 330.0), 165.0);
    check("I(12 V, 4 ohm) = 3 A",      calc_current(12.0, 4.0), 3.0);
    check("R(10 V, 2 A) = 5 ohm",      calc_resistance(10.0, 2.0), 5.0);
    check("R(5 V, 0 A) -> NaN",        calc_resistance(5.0, 0.0), NAN);
    check("I(5 V, 0 ohm) -> NaN",      calc_current(5.0, 0.0), NAN);
    check("I(-3 V, 0 ohm) -> NaN",     calc_current(-3.0, 0.0), NAN);

    puts("\n=== Uji Rangkaian Seri ===");
    check("100+220+330 = 650",         calc_series_resistance(seri, 3), 650.0);
    check("NULL -> NaN",               calc_series_resistance(NULL, 3), NAN);
    check("count 0 -> NaN",            calc_series_resistance(seri, 0), NAN);
    check("nilai 0 -> NaN",            calc_series_resistance(invalid, 2), NAN);
    check("nilai negatif -> NaN",      calc_series_resistance(negatif, 2), NAN);

    puts("\n=== Uji Rangkaian Paralel ===");
    check("100|100 = 50",              calc_parallel_resistance(dua, 2), 50.0);
    check("100|220|330",               calc_parallel_resistance(seri, 3),
          1.0 / (1.0/100.0 + 1.0/220.0 + 1.0/330.0));
    check("count 0 -> NaN",            calc_parallel_resistance(seri, 0), NAN);
    check("nilai 0 -> NaN",            calc_parallel_resistance(invalid, 2), NAN);
    check("nilai negatif -> NaN",      calc_parallel_resistance(negatif, 2), NAN);

    puts("\n=== Uji Daya Listrik ===");
    check("P(12 V, 2 A) = 24 W",       calc_power(12.0, 2.0), 24.0);
    check("P(230 V, 0.5 A)",           calc_power(230.0, 0.5), 115.0);
    check("P(0 V, 0 A) = 0 W",         calc_power(0.0, 0.0), 0.0);
    check("P = I^2*R (2 A, 100 ohm)",  calc_power_i2r(2.0, 100.0), 400.0);
    check("P = I^2*R (0 A, 50 ohm)",   calc_power_i2r(0.0, 50.0), 0.0);
    check("P = V^2/R (10 V, 4 ohm)",   calc_power_v2r(10.0, 4.0), 25.0);
    check("P = V^2/R (220 V, 100 ohm)", calc_power_v2r(220.0, 100.0), 484.0);
    check("P = V^2/R R=0 -> NaN",      calc_power_v2r(10.0, 0.0), NAN);
    check("P = V^2/R R=-0 -> NaN",     calc_power_v2r(5.0, -0.0), NAN);

    puts("\n=== Uji Muatan Listrik ===");
    check("Q(2 A, 3 s) = 6 C",         calc_charge(2.0, 3.0), 6.0);
    check("Q(0 A, 10 s) = 0 C",        calc_charge(0.0, 10.0), 0.0);
    check("I(10 C, 4 s) = 2.5 A",      calc_current_from_charge(10.0, 4.0), 2.5);
    check("I(5 C, 0 s) -> NaN",        calc_current_from_charge(5.0, 0.0), NAN);
    check("t(12 C, 2 A) = 6 s",        calc_time_from_charge(12.0, 2.0), 6.0);
    check("t(7 C, 0 A) -> NaN",        calc_time_from_charge(7.0, 0.0), NAN);

    puts("\n=== Uji Kapasitansi Kapasitor ===");
    check("C(10 C, 2 V) = 5 F",        calc_capacitance(10.0, 2.0), 5.0);
    check("C(4 C, 0 V) -> NaN",        calc_capacitance(4.0, 0.0), NAN);
    check("C(6 C, -3 V) = -2 F",       calc_capacitance(6.0, -3.0), -2.0);
    check("Q(2 F, 3 V) = 6 C",         calc_charge_from_capacitance(2.0, 3.0), 6.0);
    check("Q(0 F, 10 V) = 0 C",        calc_charge_from_capacitance(0.0, 10.0), 0.0);
    check("V(12 C, 2 F) = 6 V",        calc_voltage_from_capacitance(12.0, 2.0), 6.0);
    check("V(7 C, 0 F) -> NaN",        calc_voltage_from_capacitance(7.0, 0.0), NAN);

    puts("\n=== Uji Energi Kapasitor ===");
    check("Ec(2 F, 3 V) = 9 J",        calc_cap_energy_cv(2.0, 3.0), 9.0);
    check("Ec(0 F, 10 V) = 0 J",       calc_cap_energy_cv(0.0, 10.0), 0.0);
    check("Ec(4 C, 5 V) = 10 J",       calc_cap_energy_qv(4.0, 5.0), 10.0);
    check("Ec(0 C, 220 V) = 0 J",      calc_cap_energy_qv(0.0, 220.0), 0.0);
    check("Ec(6 C, 2 F) = 9 J",        calc_cap_energy_qc(6.0, 2.0), 9.0);
    check("Ec(5 C, 0 F) -> NaN",       calc_cap_energy_qc(5.0, 0.0), NAN);

    puts("\n=== Uji Energi Induktor ===");
    check("El(2 H, 3 A) = 9 J",        calc_ind_energy_li(2.0, 3.0), 9.0);
    check("El(0.5 H, 4 A) = 4 J",      calc_ind_energy_li(0.5, 4.0), 4.0);
    check("El(0 H, 5 A) = 0 J",        calc_ind_energy_li(0.0, 5.0), 0.0);
    check("El(6 Wb, 2 A) = 6 J",       calc_ind_energy_fi(6.0, 2.0), 6.0);
    check("El(4 Wb, 2 H) = 4 J",       calc_ind_energy_fl(4.0, 2.0), 4.0);
    check("El(5 Wb, 0 H) -> NaN",      calc_ind_energy_fl(5.0, 0.0), NAN);

    puts("\n=== Uji Reaktansi AC ===");
    check("XL(50 Hz, 0.1 H)",          calc_xl(50.0, 0.1), 2.0*M_PI*50.0*0.1);
    check("XL(60 Hz, 0.5 H)",          calc_xl(60.0, 0.5), 2.0*M_PI*60.0*0.5);
    check("XL(0 Hz, 2 H) = 0",         calc_xl(0.0, 2.0), 0.0);
    check("XC(50 Hz, 10 uF)",          calc_xc(50.0, 1e-5),
          1.0 / (2.0*M_PI*50.0*1e-5));
    check("XC(100 Hz, 100 nF)",        calc_xc(100.0, 1e-7),
          1.0 / (2.0*M_PI*100.0*1e-7));
    check("XC(f=0) -> NaN",            calc_xc(0.0, 1e-5), NAN);
    check("XC(C=0) -> NaN",            calc_xc(50.0, 0.0), NAN);

    puts("\n=== Uji Impedansi Seri ===");
    check("Z(R=30, X=40) = 50",        calc_z_series(30.0, 40.0), 50.0);
    check("Z(R=0, X=7) = 7",           calc_z_series(0.0, 7.0), 7.0);
    check("Z(R=6, X=-8) = 10",         calc_z_series(6.0, -8.0), 10.0);
    check("Z(R=100, X=0) = 100",       calc_z_series(100.0, 0.0), 100.0);
    /* RLC seri: Z = sqrt(R^2 + (XL - XC)^2) */
    {
        double xl  = calc_xl(50.0, 0.1);
        double xc  = calc_xc(50.0, 1e-5);
        double z   = calc_z_series(30.0, xl - xc);
        double zwant = sqrt(30.0*30.0 + (xl - xc)*(xl - xc));
        check("Z RLC(30, XL-XC)",      z, zwant);
    }

    puts("\n=== Uji Kode Kapasitor (SMD/keramik) ===");
    checki("decode_cap(10, 4) = 100.000 pF", decode_cap_code(10, 4), 100000LL);
    checki("decode_cap(22, 0) = 22 pF",      decode_cap_code(22, 0), 22LL);
    checki("decode_cap(47, 5) = 4.700.000",  decode_cap_code(47, 5), 4700000LL);
    checki("decode_cap(100, 6)",             decode_cap_code(100, 6), 100000000LL);
    checki("decode_cap(exp > 9) -> -1",      decode_cap_code(10, 10), -1LL);
    checki("decode_cap(digits < 0) -> -1",   decode_cap_code(-1, 4), -1LL);

    puts("\n=== Uji Kode Induktor ===");
    checki("decode_ind(10, 1) = 100 nH",     decode_ind_code(10, 1), 100LL);
    checki("decode_ind(47, 0) = 47 nH",      decode_ind_code(47, 0), 47LL);
    checki("decode_ind(22, 1) = 220 nH",     decode_ind_code(22, 1), 220LL);
    checki("decode_ind(exp < 0) -> -1",      decode_ind_code(10, -1), -1LL);

    puts("\n=== Uji Huruf Toleransi (persen x100) ===");
    checki("toleransi 'J' = 500 (5%)",       tolerance_letter_percent('J'), 500LL);
    checki("toleransi 'K' = 1000 (10%)",     tolerance_letter_percent('K'), 1000LL);
    checki("toleransi 'M' = 1500 (15%)",     tolerance_letter_percent('M'), 1500LL);
    checki("toleransi 'N' = 2000 (20%)",     tolerance_letter_percent('N'), 2000LL);
    checki("toleransi 'B' = 50 (0,5%)",      tolerance_letter_percent('B'), 50LL);
    checki("toleransi 'C' = 10 (0,1%)",      tolerance_letter_percent('C'), 10LL);
    checki("toleransi 'D' = 25 (0,25%)",     tolerance_letter_percent('D'), 25LL);
    checki("toleransi 'P' = 10000 (100%)",   tolerance_letter_percent('P'), 10000LL);
    checki("huruf kecil 'j' sama dgn 'J'",   tolerance_letter_percent('j'), 500LL);
    checki("'H' tak dipetakan -> -1",         tolerance_letter_percent('H'), -1LL);
    checki("'X' di luar tabel -> -1",         tolerance_letter_percent('X'), -1LL);
    checki("'A' sebelum B -> -1",             tolerance_letter_percent('A'), -1LL);

    printf("\n%s (%d kegagalan)\n", g_fail ? "GAGAL" : "SEMUA UJI LULUS", g_fail);
    return g_fail ? 1 : 0;
}
