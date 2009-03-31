.include "../../include/memory.inc"
.include "../../include/header.inc"
.include "../../include/regs.inc"
.include "../../include/macros.inc"

.bank 0 slot 0
.org $150

.equ SCRATCH $c000
.equ scroll $C100
.equ col_delta $c101
.equ delay $c102
.equ pal $c103


reset:
        di
        ld SP,$fff4     ; init stack pointer

        ld a,1
        ld [KEY1],a
        stop            ; enable double speed

        nop
        nop
        nop
        nop

        xor a
        ld  [SCY],a
        ld  [SCX],a
        ld  [LCDC],a


        ld hl,city_chr
        ld de,$8000
        ld bc,city_chr_end-city_chr
        call dma_copy

        ld hl,city_mapl
        ld de,$9800
        ld bc,city_mapl_end-city_mapl
        call dma_copy

        ;load_pal city_pal,8
        ld hl,city_pal
        ld de,pal
        ld bc,8
        call copy_mem

        ld a,4
        ld [delay],a

        ld a,(1<<7)|(1<<4)       ; LCD Controller = On
        ld [LCDC],a




wait:
        ld a,[STAT]
        and %11
        dec a
        jp nz,wait

        ld a,[delay]
        dec a
        jp nz, store_delay

        ld hl,pal
        ld c,-1
        call palette_add_constant

        ld a,4

store_delay:
        
        ld [delay],a

;        ld hl,city_pal
;        ld de,pal
;        ld a,[col_delta]
;        inc a
;        and %11111
;        ld [col_delta],a
;        ld c,a
;        ld b,4
;        call build_faded_palette

        ld a,$80
        ld [BCPS],a
        load_pal pal,8


wait2:
        ld a,[STAT]
        and %11
        dec a
        jp z,wait2

        ld a,[scroll]
        inc a
        ld [scroll],a

        jp wait

.include "../../lib/std.asm"

irq_vbi:
        reti
irq_lcd:
        reti
irq_tim:
        reti
irq_ser:
        reti
irq_htl:
        reti

.include "city.pal.s"
.include "city.chr.s"
.include "city.map.s"

