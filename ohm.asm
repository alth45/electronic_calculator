; =============================================================================
; ohm.asm  -  Core logika perhitungan rangkaian elektronika
; -----------------------------------------------------------------------------
; Assembler : NASM (x86-64)
; Target    : Windows x64 calling convention
;             - Argumen double pertama/kedua : XMM0 / XMM1
;             - Argumen pointer (RCX) dan int (EDX) untuk array
;             - Return value double : XMM0
;             - SSE2 selalu tersedia di x86-64 (untuk mulsd/divsd)
;
; Semua fungsi mengembalikan quiet NaN (0x7FF8000000000000) sebagai
; kode error jika input tidak valid (pembagian nol, NULL, count <= 0,
; atau ada hambatan <= 0 / NaN di dalam array).
; =============================================================================

default rel

section .rodata
    dq_zero:    dq 0.0
    dq_one:     dq 1.0
    dq_nan:     dq 0x7FF8000000000000      ; quiet NaN = kode error

section .text

global calc_voltage
global calc_current
global calc_resistance
global calc_series_resistance
global calc_parallel_resistance
global calc_power
global calc_power_i2r
global calc_power_v2r
global calc_charge
global calc_current_from_charge
global calc_time_from_charge


; -----------------------------------------------------------------------------
; double calc_voltage(double current /*XMM0*/, double resistance /*XMM1*/)
; V = I * R
; -----------------------------------------------------------------------------
calc_voltage:
    mulsd   xmm0, xmm1                     ; xmm0 = I * R
    ret

; -----------------------------------------------------------------------------
; double calc_current(double voltage /*XMM0*/, double resistance /*XMM1*/)
; I = V / R,  error (NaN) jika R == 0
; -----------------------------------------------------------------------------
calc_current:
    movq    rax, xmm1                      ; ambil bit pattern R
    mov     rcx, 0x7FFFFFFFFFFFFFFF
    and     rax, rcx                       ; mask exponent+mantissa
    jz      .error                         ; |R| == 0 (termasuk -0.0) -> error
    divsd   xmm0, xmm1                     ; xmm0 = V / R
    ret
.error:
    movsd   xmm0, [dq_nan]
    ret

; -----------------------------------------------------------------------------
; double calc_resistance(double voltage /*XMM0*/, double current /*XMM1*/)
; R = V / I,  error (NaN) jika I == 0
; -----------------------------------------------------------------------------
calc_resistance:
    movq    rax, xmm1                      ; ambil bit pattern I
    mov     rcx, 0x7FFFFFFFFFFFFFFF
    and     rax, rcx
    jz      .error                         ; |I| == 0 -> error
    divsd   xmm0, xmm1                     ; xmm0 = V / I
    ret
.error:
    movsd   xmm0, [dq_nan]
    ret

; -----------------------------------------------------------------------------
; double calc_series_resistance(const double *values /*RCX*/, int count /*EDX*/)
; Rs = R1 + R2 + ... + Rn  (loop penjumlahan di assembly)
; -----------------------------------------------------------------------------
calc_series_resistance:
    test    rcx, rcx
    jz      .error                         ; pointer NULL
    cmp     edx, 0
    jle     .error                         ; count <= 0

    movsxd  r8, edx                        ; r8 = count (64-bit)
    xor     r9, r9                         ; r9 = i = 0
    xorpd   xmm0, xmm0                     ; total = 0.0

.loop:
    cmp     r9, r8
    jge     .done

    movsd   xmm1, [rcx + r9*8]             ; xmm1 = values[i]

    ucomisd xmm1, xmm1                     ; cek NaN: PF=1 jika unordered
    jpe     .error
    ucomisd xmm1, [dq_zero]                ; harus > 0.0
    jbe     .error

    addsd   xmm0, xmm1                     ; total += values[i]
    inc     r9
    jmp     .loop

.done:
    ret                                     ; hasil di xmm0
.error:
    movsd   xmm0, [dq_nan]
    ret

; -----------------------------------------------------------------------------
; double calc_parallel_resistance(const double *values /*RCX*/, int count /*EDX*/)
; Rp = 1 / (1/R1 + 1/R2 + ... + 1/Rn)
; -----------------------------------------------------------------------------
calc_parallel_resistance:
    test    rcx, rcx
    jz      .error                         ; pointer NULL
    cmp     edx, 0
    jle     .error                         ; count <= 0

    movsxd  r8, edx                        ; r8 = count (64-bit)
    xor     r9, r9                         ; r9 = i = 0
    xorpd   xmm0, xmm0                     ; akumulator sum(1/Ri) = 0.0

.loop:
    cmp     r9, r8
    jge     .finalize

    movsd   xmm1, [rcx + r9*8]             ; xmm1 = values[i]

    ucomisd xmm1, xmm1                     ; cek NaN: PF=1 jika unordered
    jpe     .error
    ucomisd xmm1, [dq_zero]                ; harus > 0.0
    jbe     .error

    movsd   xmm2, [dq_one]
    divsd   xmm2, xmm1                     ; xmm2 = 1 / values[i]
    addsd   xmm0, xmm2                     ; sum += 1/Ri
    inc     r9
    jmp     .loop

.finalize:
    ; Karena setiap suku 1/Ri > 0, maka sum > 0 -> pembagian aman.
    movsd   xmm1, [dq_one]
    divsd   xmm1, xmm0                     ; 1 / sum
    movapd  xmm0, xmm1
    ret
.error:
    movsd   xmm0, [dq_nan]
    ret

; =============================================================================
; Daya listrik (watt)
; =============================================================================

; -----------------------------------------------------------------------------
; double calc_power(double voltage /*XMM0*/, double current /*XMM1*/)
; P = V * I
; -----------------------------------------------------------------------------
calc_power:
    mulsd   xmm0, xmm1                     ; P = V * I
    ret

; -----------------------------------------------------------------------------
; double calc_power_i2r(double current /*XMM0*/, double resistance /*XMM1*/)
; P = I^2 * R
; -----------------------------------------------------------------------------
calc_power_i2r:
    mulsd   xmm0, xmm0                     ; I * I
    mulsd   xmm0, xmm1                     ; I^2 * R
    ret

; -----------------------------------------------------------------------------
; double calc_power_v2r(double voltage /*XMM0*/, double resistance /*XMM1*/)
; P = V^2 / R,  error (NaN) jika R == 0
; -----------------------------------------------------------------------------
calc_power_v2r:
    movq    rax, xmm1                      ; ambil bit pattern R
    mov     rcx, 0x7FFFFFFFFFFFFFFF
    and     rax, rcx                       ; mask exponent+mantissa
    jz      .error                         ; |R| == 0 -> error
    mulsd   xmm0, xmm0                     ; V * V
    divsd   xmm0, xmm1                     ; V^2 / R
    ret
.error:
    movsd   xmm0, [dq_nan]
    ret

; =============================================================================
; Muatan listrik (coulomb)
; =============================================================================

; -----------------------------------------------------------------------------
; double calc_charge(double current /*XMM0*/, double time /*XMM1*/)
; Q = I * t
; -----------------------------------------------------------------------------
calc_charge:
    mulsd   xmm0, xmm1                     ; Q = I * t
    ret

; -----------------------------------------------------------------------------
; double calc_current_from_charge(double charge /*XMM0*/, double time /*XMM1*/)
; I = Q / t,  error (NaN) jika t == 0
; -----------------------------------------------------------------------------
calc_current_from_charge:
    movq    rax, xmm1                      ; ambil bit pattern t
    mov     rcx, 0x7FFFFFFFFFFFFFFF
    and     rax, rcx                       ; mask exponent+mantissa
    jz      .error                         ; |t| == 0 -> error
    divsd   xmm0, xmm1                     ; I = Q / t
    ret
.error:
    movsd   xmm0, [dq_nan]
    ret

; -----------------------------------------------------------------------------
; double calc_time_from_charge(double charge /*XMM0*/, double current /*XMM1*/)
; t = Q / I,  error (NaN) jika I == 0
; -----------------------------------------------------------------------------
calc_time_from_charge:
    movq    rax, xmm1                      ; ambil bit pattern I
    mov     rcx, 0x7FFFFFFFFFFFFFFF
    and     rax, rcx                       ; mask exponent+mantissa
    jz      .error                         ; |I| == 0 -> error
    divsd   xmm0, xmm1                     ; t = Q / I
    ret
.error:
    movsd   xmm0, [dq_nan]
    ret
