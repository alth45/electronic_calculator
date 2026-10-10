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
    dq_half:    dq 0.5                      ; konstanta 1/2 untuk energi
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
global calc_capacitance
global calc_charge_from_capacitance
global calc_voltage_from_capacitance
global calc_cap_energy_cv
global calc_cap_energy_qv
global calc_cap_energy_qc
global calc_ind_energy_li
global calc_ind_energy_fi
global calc_ind_energy_fl


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

; =============================================================================
; Kapasitansi kapasitor (farad)
; =============================================================================

; -----------------------------------------------------------------------------
; double calc_capacitance(double charge /*XMM0*/, double voltage /*XMM1*/)
; C = Q / V,  error (NaN) jika V == 0
; -----------------------------------------------------------------------------
calc_capacitance:
    movq    rax, xmm1                      ; ambil bit pattern V
    mov     rcx, 0x7FFFFFFFFFFFFFFF
    and     rax, rcx                       ; mask exponent+mantissa
    jz      .error                         ; |V| == 0 -> error
    divsd   xmm0, xmm1                     ; C = Q / V
    ret
.error:
    movsd   xmm0, [dq_nan]
    ret

; -----------------------------------------------------------------------------
; double calc_charge_from_capacitance(double capacitance /*XMM0*/,
;                                     double voltage /*XMM1*/)
; Q = C * V
; -----------------------------------------------------------------------------
calc_charge_from_capacitance:
    mulsd   xmm0, xmm1                     ; Q = C * V
    ret

; -----------------------------------------------------------------------------
; double calc_voltage_from_capacitance(double charge /*XMM0*/,
;                                      double capacitance /*XMM1*/)
; V = Q / C,  error (NaN) jika C == 0
; -----------------------------------------------------------------------------
calc_voltage_from_capacitance:
    movq    rax, xmm1                      ; ambil bit pattern C
    mov     rcx, 0x7FFFFFFFFFFFFFFF
    and     rax, rcx                       ; mask exponent+mantissa
    jz      .error                         ; |C| == 0 -> error
    divsd   xmm0, xmm1                     ; V = Q / C
    ret
.error:
    movsd   xmm0, [dq_nan]
    ret

; =============================================================================
; Energi dalam kapasitor (joule)
; =============================================================================

; -----------------------------------------------------------------------------
; double calc_cap_energy_cv(double capacitance /*XMM0*/, double voltage /*XMM1*/)
; Ec = 1/2 * C * V^2
; -----------------------------------------------------------------------------
calc_cap_energy_cv:
    mulsd   xmm1, xmm1                     ; V * V
    mulsd   xmm0, xmm1                     ; C * V^2
    mulsd   xmm0, [dq_half]                ; * 1/2
    ret

; -----------------------------------------------------------------------------
; double calc_cap_energy_qv(double charge /*XMM0*/, double voltage /*XMM1*/)
; Ec = 1/2 * Q * V
; -----------------------------------------------------------------------------
calc_cap_energy_qv:
    mulsd   xmm0, xmm1                     ; Q * V
    mulsd   xmm0, [dq_half]                ; * 1/2
    ret

; -----------------------------------------------------------------------------
; double calc_cap_energy_qc(double charge /*XMM0*/, double capacitance /*XMM1*/)
; Ec = Q^2 / (2 * C),  error (NaN) jika C == 0
; -----------------------------------------------------------------------------
calc_cap_energy_qc:
    movq    rax, xmm1                      ; ambil bit pattern C
    mov     rcx, 0x7FFFFFFFFFFFFFFF
    and     rax, rcx                       ; mask exponent+mantissa
    jz      .error                         ; |C| == 0 -> error
    mulsd   xmm0, xmm0                     ; Q * Q
    divsd   xmm0, xmm1                     ; Q^2 / C
    mulsd   xmm0, [dq_half]                ; * 1/2  -> Q^2 / (2C)
    ret
.error:
    movsd   xmm0, [dq_nan]
    ret

; =============================================================================
; Energi dalam induktor (joule)
; =============================================================================

; -----------------------------------------------------------------------------
; double calc_ind_energy_li(double inductance /*XMM0*/, double current /*XMM1*/)
; El = 1/2 * L * I^2
; -----------------------------------------------------------------------------
calc_ind_energy_li:
    mulsd   xmm1, xmm1                     ; I * I
    mulsd   xmm0, xmm1                     ; L * I^2
    mulsd   xmm0, [dq_half]                ; * 1/2
    ret

; -----------------------------------------------------------------------------
; double calc_ind_energy_fi(double flux /*XMM0*/, double current /*XMM1*/)
; El = 1/2 * Psi * I   (Psi = fluks magnet / tautan fluks)
; -----------------------------------------------------------------------------
calc_ind_energy_fi:
    mulsd   xmm0, xmm1                     ; Psi * I
    mulsd   xmm0, [dq_half]                ; * 1/2
    ret

; -----------------------------------------------------------------------------
; double calc_ind_energy_fl(double flux /*XMM0*/, double inductance /*XMM1*/)
; El = Psi^2 / (2 * L),  error (NaN) jika L == 0
; -----------------------------------------------------------------------------
calc_ind_energy_fl:
    movq    rax, xmm1                      ; ambil bit pattern L
    mov     rcx, 0x7FFFFFFFFFFFFFFF
    and     rax, rcx                       ; mask exponent+mantissa
    jz      .error                         ; |L| == 0 -> error
    mulsd   xmm0, xmm0                     ; Psi * Psi
    divsd   xmm0, xmm1                     ; Psi^2 / L
    mulsd   xmm0, [dq_half]                ; * 1/2  -> Psi^2 / (2L)
    ret
.error:
    movsd   xmm0, [dq_nan]
    ret
