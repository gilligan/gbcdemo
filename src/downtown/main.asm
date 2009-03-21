.rombanksize $4000
.rombanks 4

.memorymap
SLOTSIZE $4000
DEFAULTSLOT 1
SLOT 0 $0000
SLOT 1 $4000
.endme

.BANK 0
.ORG $104

.DB $CE $ED $66 $66 $CC $0D $00 $0B $03 $73 $00 $83 $00 $0C $00 $0D
.DB $00 $08 $11 $1F $88 $89 $00 $0E $DC $CC $6E $E6 $DD $DD $D9 $99
.DB $BB $BB $67 $63 $6E $0E $EC $CC $DD $DC $99 $9F $BB $B9 $33 $3E

.BANK 0 SLOT 0

.ORG $40				;vbi.
	JP	irq_vbi
.ORG $48				;lcd stat.
	JP	irq_lcd
.ORG $50				;timer.
	JP	irq_tim
.ORG $58				;serial.
	JP	irq_ser
.ORG $60				;high to low.
	JP	irq_htl
.ORG $100

        nop
        jp reset





.bank 0 slot 0
.org $150


reset:
        di
        ld SP,$fff4     ; init stack pointer

        ld a,1
        ldh [$4d],a
        stop            ; enable double speed

        xor a
        ldh  [$42],a
        ldh  [$43],a
        ldh  [$40],a

.INCLUDE "../../include/std.asm"

irq_vbi:
irq_lcd:
irq_tim:
irq_ser:
irq_htl:
        reti
