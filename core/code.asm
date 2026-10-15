; =============================================================================
; code.asm  -  Core penerjemah kode angka komponen SMD / keramik
; -----------------------------------------------------------------------------
; Assembler : NASM (x86-64)
; Target    : System V AMD64 (Linux/ELF) -- dipakai oleh test_ohm.c
;             - Argumen integer pertama/kedua : EDI / ESI
;             - Return value integer          : RAX
; Aritmetika murni integer (tanpa SSE), dipakai oleh src/calc_code.c untuk
; menerjemahkan kode yang tercetak pada badan komponen:
;   - Kode kapasitor 3 digit : "104" -> 10 * 10^4 pF = 100 nF
;   - Kode induktor  3 digit : "101" -> 10 * 10^1 nH = 100 nH
;   - Huruf toleransi        : "J"   -> 500 (persen x100 => 5 %)
; Konvensi error: mengembalikan -1 jika input tidak valid.
; =============================================================================

default rel

section .rodata
<<<<<<< HEAD
align 8
tw10: dq  1, 10, 100, 1000, 10000, 100000, 1000000, 10000000, 100000000, 1000000000         ; 10^0 .. 10^9
=======
    align 8

tw10:   dq  1, 10, 100, 1000, 10000, 100000, 1000000, 10000000
        dq  100000000, 1000000000         ; 10^0 .. 10^9
>>>>>>> f552dacaf93bff496b9c75609de2b44620d7566f

; Persen x100 untuk huruf toleransi EIA B..P (indeks = huruf - 'B').
; Semua slot dari 'B' (indeks 0) s.d. 'P' (indeks 14) ditulis eksplisit dan
; berurutan supaya tidak ada pergeseran indeks.
; 0xFFFF = sentinel "tidak dikenal" (H, L, O tidak dipetakan oleh standar).
tol_x100: dw  50        ; B -> 0,5 %
          dw  10        ; C -> 0,1 %
          dw  25        ; D -> 0,25 %
          dw  50        ; E -> 0,5 %
          dw  100       ; F -> 1 %
          dw  200       ; G -> 2 %
          dw  0xFFFF    ; H -> tidak dipetakan
          dw  0xFFFF    ; I -> tidak dipetakan
          dw  500       ; J -> 5 %
          dw  1000      ; K -> 10 %
          dw  0xFFFF    ; L -> tidak dipetakan
          dw  1500      ; M -> 15 %
          dw  2000      ; N -> 20 %
          dw  0xFFFF    ; O -> tidak dipetakan
          dw  10000     ; P -> 100 %
          dw  0xFFFF    ; Q (pengaman batas tabel)

section .text

global decode_cap_code
global decode_ind_code
global tolerance_letter_percent

; -----------------------------------------------------------------------------
; long long decode_cap_code(int digits /*EDI*/, int exp /*ESI*/)
; Nilai dalam pikofarad = digits * 10^exp.
; Mengembalikan -1 jika parameter tidak valid (digits < 0 atau exp di luar
; 0..9). Contoh kode "104": digits=10, exp=4 -> 100000 pF = 100 nF.
; -----------------------------------------------------------------------------
decode_cap_code:
    test    edi, edi                      ; digits negatif?
    js      .error
    cmp     esi, 0
    jl      .error                        ; exp < 0  -> tidak valid
    cmp     esi, 9
    jg      .error                        ; exp > 9  -> tidak valid
    movsxd  rax, edi                      ; rax = digits (sign-extend)
    movsxd  rsi, esi
    lea     rdx, [rel tw10]               ; tabel pangkat 10
    mul     qword [rdx + rsi*8]           ; rax = digits * 10^exp
    ret
.error:
    mov     rax, -1
    ret

; -----------------------------------------------------------------------------
; long long decode_ind_code(int digits /*EDI*/, int exp /*ESI*/)
; Sama seperti decode_cap_code, tetapi satuan hasil adalah nanohenry (nH).
; Contoh kode "101": digits=10, exp=1 -> 100 nH = 0,1 uH.
; -----------------------------------------------------------------------------
decode_ind_code:
    jmp     decode_cap_code               ; aturan aritmetika identik

; -----------------------------------------------------------------------------
; long long tolerance_letter_percent(int letter /*EDI*/)
; Huruf toleransi EIA -> persen x100 sebagai integer.
;   B=50(0,5%) C=10(0,1%) D=25(0,25%) E=50 F=100(1%) G=200(2%)
;   J=500(5%)  K=1000(10%) M=1500(15%) N=2000(20%) P=10000(100%)
; Huruf kecil diterima (di-upper lebih dulu). Huruf tak dikenal -> -1.
;
; BUG LAMA: tabel tol_x100 didefinisikan dengan baris `dw 50, 10, 25, 50,
; 100, 200` untuk B..G, tetapi sentinel H/L/O ditulis sebagai satu `dw` per
; baris sehingga slot 'I' (yang harusnya ada di antara H dan J) hilang.
; Akibatnya indeks J..P bergeser -1 dan huruf-huruf tersebut membaca nilai
; yang salah / sentinel (-1). Perbaikan: setiap slot B..P ditulis eksplisit
; dan berurutan (termasuk I -> 0xFFFF), ditambah normalisasi huruf kecil ke
; lowercase via `or eax, 0x20` lalu `sub eax, 'b'` dengan batas indeks 0..14.
; -----------------------------------------------------------------------------
tolerance_letter_percent:
    mov     eax, edi
    and     eax, 0xFF                     ; buang byte di atas ASCII
    or      eax, 0x20                     ; 'B' -> 'b' (case-insensitive)
    sub     eax, 'b'                      ; indeks ke tabel (0 untuk 'b')
    cmp     eax, 14                       ; di luar rentang B..P?
    ja      .error                        ; (sub membuat < 'b' jadi besar unsigned)
    lea     rdx, [rel tol_x100]
    movzx   eax, word [rdx + rax*2]       ; ambil nilai persen x100
    cmp     eax, 0xFFFF                   ; sentinel tidak dikenal?
    je      .error
    ret
.error:
    mov     rax, -1
    ret
