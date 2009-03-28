.include "../../include/memory.inc"

.include "../../include/header.inc"
.include "../../include/regs.inc"
.include "../../include/macros.inc"


; ram setup

; $c000-$c0ff : scratchmem
; $c100-      : global ram
;
.equ SCRATCH $c000

.enum $C100 EXPORT
        demo_part db
        vbi_ptr   dw
        lcd_ptr   dw
        tim_ptr   dw

        pal1_tmp  dw
        pal2_tmp  dw
        pal3_tmp  dw
        pal4_tmp  dw
        pal5_tmp  dw
        pal6_tmp  dw
        pal7_tmp  dw
        pal8_tmp  dw

.ende

.bank 0 slot 0
.org $150

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

        ld a,1
        ld [demo_part],a

mainloop:

        ld a,[demo_part]
        ld ($2000),a
        jp $4000
        
        jp mainloop

irq_vbi:

        ld a,[vbi_ptr]
        ld h,a
        ld a,[vbi_ptr+1]
        ld l,a
        jp hl

irq_lcd:
        ld a,[lcd_ptr]
        ld h,a
        ld a,[lcd_ptr+1]
        ld l,a
        jp hl

irq_tim:
        ld a,[tim_ptr]
        ld h,a
        ld a,[tim_ptr+1]
        ld l,a
        jp hl

irq_ser:
        reti 
irq_htl:
        reti


.include "../../lib/std.asm"

