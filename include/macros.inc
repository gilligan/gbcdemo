;
; wait for vblank to start
;
.macro wait_vblank
@wait:
        ld a,[STAT]
        and 1
        jp z,@wait
.endm

;
; stupid macro to do (bc>>n)
;
.macro shr16_bc
        .rept \1
                srl b
                rr c
        .endr
.endm

;
; stupid macro to do (bc<<n)
;
.macro shl16_bc
        .rept \1
        sla c
        rl b
        .endr
.endm

;
; load_pal [source] [byte-count]
; NOTE: set BCPS prior to this
.macro load_pal ;
        ld hl,\1
        ld c,\2
@load_pal:
        ld a,[hl+]
        ld [BCPD],a
        dec c
        jp nz,@load_pal
.endm


;
; carry out single DMA transfer
; [len] bytes from [src] to [dst] 
;
.macro do_dma_copy ; src,dst,len
        ld a,>\1
        ld [HDMA1],a
        ld a,<\1
        ld [HDMA2],a
        ld a,>\2
        ld [HDMA3],a
        ld a,<\2
        ld [HDMA4],a
        ld a,\3>>4
        ld [HDMA5],a
.endm

;
; wait until [bit] in [reg] is set
;
.macro wait_bit_set reg, bit
@wait_bit:
        bit reg, bit
        jp z,@wait_bit
.endm

;
; wait until [bit] in [reg] is clear
;
.macro wait_bit_clear reg, bit
@wait_bit:
        bit reg, bit
        jp nz, @wait_bit
.endm
