# Kalkulator Rangkaian Elektronika (Assembly + C)

Aplikasi GUI Windows untuk menghitung besaran rangkaian elektronika —
**tegangan, arus, hambatan, gabungan hambatan (seri/paralel), daya listrik, muatan listrik, dan kapasitansi kapasitor** —
dengan pembagian tugas:

- **GUI**: bahasa **C** memakai Win32 API murni (tanpa framework eksternal)
- **Core perhitungan**: bahasa **Assembly x86-64** (NASM) dengan SSE2 (`mulsd`, `divsd`, `ucomisd`)

## Fitur

1. **Hukum Ohm** — pilih magnitudo yang dicari (radio button):
   - Cari Tegangan → `V = I × R`
   - Cari Arus → `I = V / R`
   - Cari Hambatan → `R = V / I`
   - Field yang dicari otomatis dinonaktifkan, hasil tampil dengan satuan (V / A / Ω)
2. **Gabungan Hambatan** — masukkan beberapa nilai dipisah koma (contoh: `100, 220, 330`):
   - Seri → `Rs = R1 + R2 + ... + Rn` (loop penjumlahan di assembly)
   - Paralel → `Rp = 1 / (1/R1 + 1/R2 + ... + 1/Rn)` (loop Σ(1/Ri) di assembly)
3. **Daya Listrik** — pilih rumus (radio button):
   - `P = V × I`
   - `P = I² × R`
   - `P = V² / R` (R = 0 → error)
   - Field yang tidak dipakai rumus aktif otomatis dinonaktifkan
4. **Muatan Listrik** — pilih magnitudo yang dicari (radio button):
   - Cari Muatan → `Q = I × t` (satuan coulomb)
   - Cari Arus → `I = Q / t`
   - Cari Waktu → `t = Q / I` (waktu dalam sekon)
   - Field yang dicari otomatis dinonaktifkan
5. **Kapasitansi Kapasitor** — pilih magnitudo yang dicari (radio button):
   - Cari Kapasitansi → `C = Q / V` (satuan farad)
   - Cari Muatan → `Q = C × V`
   - Cari Tegangan → `V = Q / C`

Validasi input ketat (angka valid, hambatan > 0) dengan pesan error bahasa Indonesia.

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

:: Build + jalankan 26 uji unit logika assembly
build.bat test
```

Hasil build:
- `kalkulator_rangkaian.exe` — jalankan langsung dari File Explorer
- `test_ohm.exe` — uji unit CLI (exit code 0 = semua lulus)

Build manual (setara isi `build.bat`):

```bat
nasm -f win64 ohm.asm -o ohm.obj
gcc -O2 -Wall -municode -mwindows main.c ohm.obj -o kalkulator_rangkaian.exe -lgdi32
```

## Struktur File

```
asm-c/
├── ohm.asm               # Core perhitungan Assembly (x86-64, SSE2)
├── circuit_asm.h         # Deklarasi fungsi assembly untuk C
├── main.c                # GUI Win32 (Unicode) — 3 bagian kalkulator
├── test_ohm.c            # 39 uji unit (Ohm, seri/paralel, daya, muatan, kapasitor)
├── build.bat             # Script build otomatis
├── .gitignore            # Abaikan artefak build (*.obj, *.exe, dll.)
└── README.md
```

## Arsitektur

```
┌──────────────────────────────────────────────────────┐
│  main.c (GUI Win32 / bahasa C)                       │
│  - Parse input pengguna (wcstod)                     │
│  - Validasi & pesan error                            │
│  - Format hasil + satuan                             │
└──────────────┬───────────────────────────────────────┘
               │ panggil fungsi (Windows x64 ABI:
               │ argumen double di XMM0/XMM1, hasil di XMM0)
┌──────────────▼───────────────────────────────────────┐
│  ohm.asm (Assembly NASM x86-64)                      │
│  - calc_voltage / calc_current / calc_resistance     │
│  - calc_series_resistance / calc_parallel_resistance │
│  - calc_power / calc_power_i2r / calc_power_v2r      │
│  - calc_charge / calc_current_from_charge            │
│  - calc_time_from_charge                             │
│  - calc_capacitance / calc_charge_from_capacitance   │
│  - calc_voltage_from_capacitance                     │
└──────────────────────────────────────────────────────┘
```

### Konvensi error di assembly

- Semua fungsi memakai tipe `double`.
- Input tidak valid (pembagian nol, `NULL`, count ≤ 0, hambatan ≤ 0 / NaN)
  mengembalikan **quiet NaN** (`0x7FF8000000000000`).
- Pemanggil C wajib memeriksa dengan `isnan()` dari `<math.h>` lalu menampilkan
  pesan error ke pengguna.

## Uji Program

`test_ohm.c` berisi 39 pemeriksaan:

| Kelompok | Jumlah | Contoh kasus |
|---|---|---|
| Hukum Ohm | 7 | `V(2 A, 100 Ω) = 200 V`, pembagian nol → NaN |
| Rangkaian seri | 5 | `100+220+330 = 650 Ω`, nilai ≤ 0 → NaN |
| Rangkaian paralel | 5 | `100\|100 = 50 Ω`, `100\|220\|330 ≈ 56.8966 Ω` |
| Daya listrik | 9 | `P(12 V, 2 A) = 24 W`, `V²/R` dengan R = 0 → NaN |
| Muatan listrik | 6 | `Q(2 A, 3 s) = 6 C`, `I(5 C, 0 s) → NaN` |
| Kapasitansi kapasitor | 7 | `C(10 C, 2 V) = 5 F`, `V(7 C, 0 F) → NaN` |

Jalankan `build.bat test` — keluaran akhir `SEMUA UJI LULUS (0 kegagalan)`.

## Catatan

- "Tegangan" dan "voltase" adalah besaran yang sama (V).
- Satuan ditampilkan: volt (V), ampere (A), ohm (Ω), watt (W), coulomb (C), sekon (s), farad (F).
- Perhitungan memakai IEEE 754 double precision — hasil desimal diformat `%.6g`.
