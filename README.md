# Kalkulator Rangkaian Elektronika (Assembly + C)

![Bahasa C](https://img.shields.io/badge/Bahasa-C-03599C?style=flat-square&logo=c&logoColor=white)
![Bahasa Assembly](https://img.shields.io/badge/Bahasa-Assembly%20x86--64-555555?style=flat-square)
![Assembler NASM](https://img.shields.io/badge/Assembler-NASM-2C3E50?style=flat-square)
![GUI Win32 API](https://img.shields.io/badge/GUI-Win32%20API-0078D6?style=flat-square&logo=windows&logoColor=white)
![Platform Windows](https://img.shields.io/badge/Platform-Windows-0078D6?style=flat-square&logo=windows&logoColor=white)
![Uji unit 51 lulus](https://img.shields.io/badge/Uji-51%20lulus-brightgreen?style=flat-square)
![Dokumentasi Bahasa Indonesia](https://img.shields.io/badge/Dokumentasi-Bahasa%20Indonesia-FF0000?style=flat-square)

Aplikasi GUI Windows untuk menghitung besaran rangkaian elektronika —
**tegangan, arus, hambatan, gabungan hambatan (seri/paralel), daya listrik, muatan listrik, kapasitansi kapasitor, serta energi dalam kapasitor dan induktor** —
dengan pembagian tugas:

- **GUI**: bahasa **C** memakai Win32 API murni (tanpa framework eksternal), antarmuka **tab control** — tiap kalkulator punya tab sendiri sehingga jendela tetap ringkas
- **Core perhitungan**: bahasa **Assembly x86-64** (NASM) dengan SSE2 (`mulsd`, `divsd`, `ucomisd`)

## Fitur

1. **Hukum Ohm** — pilih magnitudo yang dicari (radio button):
   - Cari Tegangan → $V = I \times R$
   - Cari Arus → $I = \dfrac{V}{R}$
   - Cari Hambatan → $R = \dfrac{V}{I}$
   - Field yang dicari otomatis dinonaktifkan, hasil tampil dengan satuan (V / A / Ω)
2. **Gabungan Hambatan** — masukkan beberapa nilai dipisah koma (contoh: `100, 220, 330`):
   - Seri → $R_s = R_1 + R_2 + \cdots + R_n$ (loop penjumlahan di assembly)
   - Paralel → $R_p = \left(\sum_{i=1}^{n} \frac{1}{R_i}\right)^{-1}$ (loop Σ(1/Rᵢ) di assembly)
3. **Daya Listrik** — pilih rumus (radio button):
   - $P = V \times I$
   - $P = I^2 \times R$
   - $P = \dfrac{V^2}{R}$ ($R = 0$ → error)
   - Field yang tidak dipakai rumus aktif otomatis dinonaktifkan
4. **Muatan Listrik** — pilih magnitudo yang dicari (radio button):
   - Cari Muatan → $Q = I \times t$ (satuan coulomb)
   - Cari Arus → $I = \dfrac{Q}{t}$
   - Cari Waktu → $t = \dfrac{Q}{I}$ (waktu dalam sekon)
   - Field yang dicari otomatis dinonaktifkan
5. **Kapasitansi Kapasitor** — pilih magnitudo yang dicari (radio button):
   - Cari Kapasitansi → $C = \dfrac{Q}{V}$ (satuan farad)
   - Cari Muatan → $Q = C \times V$
   - Cari Tegangan → $V = \dfrac{Q}{C}$
6. **Energi Kapasitor** — pilih rumus (radio button):
   - $E_c = \dfrac{1}{2} C V^2$
   - $E_c = \dfrac{1}{2} Q V$
   - $E_c = \dfrac{Q^2}{2C}$ ($C = 0$ → error)
   - Hasil dalam joule; field yang tidak dipakai rumus aktif otomatis dinonaktifkan
7. **Energi Induktor** — pilih rumus (radio button):
   - $E_l = \dfrac{1}{2} L I^2$
   - $E_l = \dfrac{1}{2} \Psi I$ ($\Psi$ = fluks magnet)
   - $E_l = \dfrac{\Psi^2}{2L}$ ($L = 0$ → error)
   - Hasil dalam joule; field yang tidak dipakai rumus aktif otomatis dinonaktifkan

Validasi input ketat (angka valid, hambatan > 0) dengan pesan error bahasa Indonesia.

## Kumpulan Rumus

**1. Hukum Ohm**

$$V = I \times R \qquad I = \frac{V}{R} \qquad R = \frac{V}{I}$$

**2. Gabungan hambatan**

$$R_s = R_1 + R_2 + \cdots + R_n$$

$$R_p = \left( \frac{1}{R_1} + \frac{1}{R_2} + \cdots + \frac{1}{R_n} \right)^{-1}$$

**3. Daya listrik**

$$P = V \times I = I^2 R = \frac{V^2}{R}$$

**4. Muatan listrik**

$$Q = I \times t \qquad I = \frac{Q}{t} \qquad t = \frac{Q}{I}$$

**5. Kapasitansi kapasitor**

$$C = \frac{Q}{V} \qquad Q = C \times V \qquad V = \frac{Q}{C}$$

**6. Energi dalam kapasitor**

$$E_c = \frac{1}{2} C V^2 = \frac{1}{2} Q V = \frac{Q^2}{2C}$$

**7. Energi dalam induktor**

$$E_l = \frac{1}{2} L I^2 = \frac{1}{2} \Psi I = \frac{\Psi^2}{2L}$$

Keterangan simbol: $V$ = tegangan/voltase (volt), $I$ = arus (ampere), $R$ = hambatan (ohm), $P$ = daya (watt), $Q$ = muatan (coulomb), $t$ = waktu (sekon), $C$ = kapasitansi (farad), $L$ = induktansi (henry), $\Psi$ = fluks magnet (weber), $E_c$ / $E_l$ = energi (joule).

## Persyaratan

| Tool | Fungsi | Contoh versi teruji |
|---|---|---|
| NASM | Assembler `.asm` → `.obj` | 2.16.03 |
| GCC (MinGW-w64) | Kompilasi & link C → `.exe` | 15.1.0 (ucrt-posix-seh) |

Keduanya harus ada di `PATH`. Windows 64-bit (target ABI: Windows x64).

## Cara Build & Menjalankan

```bat
:: Build GUI saja
build.bat

:: Build + jalankan 45 uji unit logika assembly
build.bat test
```

Hasil build:
- `kalkulator_rangkaian.exe` — jalankan langsung dari File Explorer
- `test_ohm.exe` — uji unit CLI (exit code 0 = semua lulus)

Build manual (setara isi `build.bat`):

```bat
nasm -f win64 core\ohm.asm -o ohm.obj
gcc -O2 -Wall -municode -mwindows -Iinclude ^
    src\main.c src\gui.c src\calc_ohm.c src\calc_power.c src\calc_charge.c ^
    src\calc_energy.c ^
    ohm.obj -o kalkulator_rangkaian.exe -lgdi32

:: Uji unit (opsional)
gcc -O2 -Wall -Iinclude test_ohm.c ohm.obj -o test_ohm.exe
```

## Struktur File

```
asm-c/
├── include/
│   ├── circuit_asm.h     # Deklarasi fungsi Assembly untuk C
│   └── app.h             # ID kontrol, konstanta mode, state & prototipe
├── core/
│   └── ohm.asm           # Core perhitungan Assembly (x86-64, SSE2)
├── src/
│   ├── main.c            # Entry point: wWinMain + definisi state global
│   ├── gui.c             # Window procedure, kontrol UI, utilitas GUI
│   ├── calc_ohm.c        # Kalkulator hukum Ohm & gabungan hambatan
│   ├── calc_power.c      # Kalkulator daya listrik
│   ├── calc_charge.c     # Kalkulator muatan & kapasitansi kapasitor
│   ├── calc_energy.c     # Kalkulator energi kapasitor & induktor
├── test_ohm.c            # 51 uji unit (Ohm, seri/paralel, daya, muatan, kapasitor, energi C & L)
├── build.bat             # Script build otomatis
├── .gitignore            # Abaikan artefak build (*.obj, *.exe, dll.)
└── README.md
```

## Arsitektur

```
┌──────────────────────────────────────────────────────┐
│  src/*.c (GUI Win32 / bahasa C)                      │
│  src/main.c        - entry point + state global      │
│  src/gui.c         - window procedure + kontrol UI   │
│  src/calc_*.c      - logika kalkulator per fitur     │
│  include/app.h     - deklarasi bersama antarmodul    │
│  - Parse input pengguna (wcstod)                     │
│  - Validasi & pesan error                            │
│  - Format hasil + satuan                             │
└──────────────┬───────────────────────────────────────┘
               │ panggil fungsi (Windows x64 ABI:
               │ argumen double di XMM0/XMM1, hasil di XMM0)
┌──────────────▼───────────────────────────────────────┐
│  core/ohm.asm (Assembly NASM x86-64)                 │
│  - calc_voltage / calc_current / calc_resistance     │
│  - calc_series_resistance / calc_parallel_resistance │
│  - calc_power / calc_power_i2r / calc_power_v2r      │
│  - calc_charge / calc_current_from_charge            │
│  - calc_time_from_charge                             │
│  - calc_capacitance / calc_charge_from_capacitance   │
│  - calc_voltage_from_capacitance                     │
│  - calc_cap_energy_cv / _qv / _qc                    │
│  - calc_ind_energy_li / _fi / _fl                    │
└──────────────────────────────────────────────────────┘
```

### Konvensi error di assembly

- Semua fungsi memakai tipe `double`.
- Input tidak valid (pembagian nol, `NULL`, count ≤ 0, hambatan ≤ 0 / NaN)
  mengembalikan **quiet NaN** (`0x7FF8000000000000`).
- Pemanggil C wajib memeriksa dengan `isnan()` dari `<math.h>` lalu menampilkan
  pesan error ke pengguna.

## Uji Program

`test_ohm.c` berisi 51 pemeriksaan:

| Kelompok | Jumlah | Contoh kasus |
|---|---|---|
| Hukum Ohm | 7 | `V(2 A, 100 Ω) = 200 V`, pembagian nol → NaN |
| Rangkaian seri | 5 | `100+220+330 = 650 Ω`, nilai ≤ 0 → NaN |
| Rangkaian paralel | 5 | `100\|100 = 50 Ω`, `100\|220\|330 ≈ 56.8966 Ω` |
| Daya listrik | 9 | `P(12 V, 2 A) = 24 W`, `V²/R` dengan R = 0 → NaN |
| Muatan listrik | 6 | `Q(2 A, 3 s) = 6 C`, `I(5 C, 0 s) → NaN` |
| Kapasitansi kapasitor | 7 | `C(10 C, 2 V) = 5 F`, `V(7 C, 0 F) → NaN` |
| Energi kapasitor | 6 | `Ec(2 F, 3 V) = 9 J`, `Ec(5 C, 0 F) → NaN` |
| Energi induktor | 6 | `El(2 H, 3 A) = 9 J`, `El(5 Wb, 0 H) → NaN` |

Jalankan `build.bat test` — keluaran akhir `SEMUA UJI LULUS (0 kegagalan)`.

## Catatan

- "Tegangan" dan "voltase" adalah besaran yang sama (V).
- Satuan ditampilkan: volt (V), ampere (A), ohm (Ω), watt (W), coulomb (C), sekon (s), farad (F), henry (H), weber (Wb), joule (J).
- Perhitungan memakai IEEE 754 double precision — hasil desimal diformat `%.6g`.
