.include "../../include/header.inc"
.include "../../include/regs.inc"

.bank 0 slot 0
.org $150

reset:
        di
        ld SP,$fff4     ; init stack pointer

        ld a,1
        ld [KEY1],a
        stop            ; enable double speed

        xor a
        ld  [SCY],a
        ld  [SCX],a
        ld  [LCDC],a


.INCLUDE "../../include/std.asm"

irq_vbi:
irq_lcd:
irq_tim:
irq_ser:
irq_htl:
        reti
