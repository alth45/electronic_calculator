/*
 * circuit_asm.h
 * -------------
 * Deklarasi fungsi inti perhitungan rangkaian elektronika.
 * Implementasi ada di ohm.asm (NASM, x86-64, Windows x64 ABI).
 *
 * Semua nilai memakai tipe double:
 *   V = tegangan / voltase (volt)
 *   I = arus   (ampere)
 *   R = hambatan (ohm)
 *
 * Konvensi error: fungsi mengembalikan NaN (Not a Number) jika input
 * tidak valid, misalnya pembagian nol atau nilai hambatan <= 0.
 * Pemanggil wajib memeriksa dengan isnan() dari <math.h>.
 */
#ifndef CIRCUIT_ASM_H
#define CIRCUIT_ASM_H

/* V = I * R */
double calc_voltage(double current, double resistance);

/* I = V / R   (R == 0 -> NaN) */
double calc_current(double voltage, double resistance);

/* R = V / I   (I == 0 -> NaN) */
double calc_resistance(double voltage, double current);

/* Rs = R1 + R2 + ... + Rn  (count <= 0, NULL, atau ada nilai <= 0 -> NaN) */
double calc_series_resistance(const double *values, int count);

/* Rp = 1 / (1/R1 + 1/R2 + ... + 1/Rn)  (count <= 0, NULL, atau ada nilai <= 0 -> NaN) */
double calc_parallel_resistance(const double *values, int count);

/* P = V * I  (daya, watt) */
double calc_power(double voltage, double current);

/* P = I^2 * R  (daya, watt) */
double calc_power_i2r(double current, double resistance);

/* P = V^2 / R  (daya, watt),  R == 0 -> NaN */
double calc_power_v2r(double voltage, double resistance);

/* Q = I * t  (muatan, coulomb) */
double calc_charge(double current, double time);

/* I = Q / t  (arus, ampere),  t == 0 -> NaN */
double calc_current_from_charge(double charge, double time);

/* t = Q / I  (waktu, sekon),  I == 0 -> NaN */
double calc_time_from_charge(double charge, double current);

/* C = Q / V  (kapasitansi, farad),  V == 0 -> NaN */
double calc_capacitance(double charge, double voltage);

/* Q = C * V  (muatan, coulomb) */
double calc_charge_from_capacitance(double capacitance, double voltage);

/* V = Q / C  (tegangan, volt),  C == 0 -> NaN */
double calc_voltage_from_capacitance(double charge, double capacitance);

/* Ec = 1/2 * C * V^2  (energi kapasitor, joule) */
double calc_cap_energy_cv(double capacitance, double voltage);

/* Ec = 1/2 * Q * V  (energi kapasitor, joule) */
double calc_cap_energy_qv(double charge, double voltage);

/* Ec = Q^2 / (2C)  (energi kapasitor, joule),  C == 0 -> NaN */
double calc_cap_energy_qc(double charge, double capacitance);

/* El = 1/2 * L * I^2  (energi induktor, joule) */
double calc_ind_energy_li(double inductance, double current);

/* El = 1/2 * Psi * I  (energi induktor, joule; Psi = fluks magnet) */
double calc_ind_energy_fi(double flux, double current);

/* El = Psi^2 / (2L)  (energi induktor, joule),  L == 0 -> NaN */
double calc_ind_energy_fl(double flux, double inductance);

#endif /* CIRCUIT_ASM_H */
