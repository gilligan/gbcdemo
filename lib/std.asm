; 
; generic functions
;

; input: [hl] = src palette data
;        [de] = dst palette data
;         c   = fade step (0..31)
;         b   = number of colors
fade_white_to_pal:

/*
   for (int i=0; i<4; i++){

        r = get_red(hl[i]);
        g = get_green(hl[i]):
        b = get_blue(hl[i]);

        r += c; if (r>31) r=31;
        g += c; if (g>31) g=31;
        b += c; if (b>31) b=31;

        de[i] = compose_pal(r,g,b)
   }
   
*/

.equ temp_blue 0
.equ temp_green 1
.equ temp_red 2

color_change_lp:

        ld a,[hl]
        srl a
        srl a     ; a = blue
        add c     ; add color delta
        bit 5,a
        jp z,blue_foo
        ld a,$0f
blue_foo:
        ld [SCRATCH+temp_blue],a

        ld a,[hl]
        sla a
        sla a
        sla a
        ld b,a
        ld a,[hl+]
        srl a
        srl a
        srl a
        srl a
        srl a
        or b
        and %11111 ; a = green
        add c      ; add color delta
        bit 5,a
        jp z,green_foo
        ld a,$0f
green_foo:
        ld [SCRATCH+temp_green],a

        ld a,[hl+]
        and %11111 ; a = red
        add c
        bit 5,a
        jp z,red_foo
red_foo:
        ld a,$0f
        ld [SCRATCH+temp_red],a

        ; 
        ; compose R/G/B and store values back
        ; 
        ld a,[SCRATCH+temp_blue]
        sla a
        sla a
        ld b,a
        ld a,[SCRATCH+temp_green]
        srl a
        srl a
        srl a
        and %11
        ld [de],a ; store upper byte (blue/green)
        inc de

        ld a,[SCRATCH+temp_green]
        and %111
        sla a
        sla a
        sla a
        sla a
        sla a
        ld b,a
        ld a,[SCRATCH+temp_red]
        or b
        ld [de],a
        inc de

        dec b
        jp nz,color_change_lp
 
        ret

sprite_load_dma:
	push	af
	ld	a, $C0
	ldh	($46), A			; $46 == r_dma
	ld	a, $28

-	dec	a
	jr	nz, -
	pop	af
	ret

;
; copies the sprite handler code
; to $FF80 (has to be executed from there)
;
copy_sprite_handler:
        ld hl,$ff80
        ld de,sprite_load_dma
        .rept 12
        ld a,[hl+]
        ld [de],a
        inc a
        .endr
        ret


; wait for scanline
;
; in       : b=scanline
; out      : --
; destroys : a,b
;
wait_for_scanline:
        dec b
lp:
        ldh a,[$44]
        cp b
        jp nz,lp
hblank:
        ldh a,[$41]
        and %11
        jp nz,hblank
        ret

;
; copy bc bytes of memory from [hl] to [de]
;
; in       : hl=src,de=dest,bc=count
; out      : --
; destroys : a,hl,de,bc
;
copy_mem:
        inc b
        inc c
        jr @skip

@loop:
        ld a,[hl+]
        ld [de],a
        inc de
@skip:  dec c
        jr nz,@loop
        dec b
        jr nz,@loop
        ret

;
; reads joypad status
;
; in       : --
; out      : A=button pressed
; destroys : a,b
;
read_joypad:

	ld a,$20      ; <- bit 5 = $20
	ldh [$00],a  ; <- select P14 by setting it low
	ldh a,[$00]
	ldh a,[$00]
	ldh a,[$00]
	ldh a,[$00]  ; <- wait a few cycles
	cpl           ; <- complement A
	and $0F       ; <- get only first 4 bits
	swap a        ; <- swap it
	ld b,a        ; <- store A in B
	ld a,$10
	ldh [$00],a  ; <- select P15 by setting it low
	ldh a,[$00]
	ldh a,[$00]
	ldh a,[$00]
	ldh a,[$00]
	ldh a,[$00]
	ldh a,[$00]  ; <- Wait a few MORE cycles
	cpl           ; <- complement (invert)
	and $0F       ; <- get first 4 bits
	or b          ; <- put A and B together

	ld b,a        ; <- store A in B
	ldh a,[$8B]  ; <- read old joy data from ram
	xor  b         ; <- toggle w/current button bit
	and  b         ; <- get current button bit back
	ldh [$8C],a  ; <- save in new Joydata storage
	ld a,b        ; <- put original value in A
	ldh [$8B],a  ; <- store it as old joy data
	ld a,$30      ; <- deselect P14 and P15
	ldh [$00],a  ; <- RESET Joypad
	ret           ; <- Return from Subroutine




;
; copy data via DMA
; 
; in       : hl = src, de = dst, bc = number of bytes (mod16==0) / max == 4096
; out      : --
; destroys : uhm.. everything :)
;
dma_copy:
        shr16_bc 4 ; bc = count/16

        ld a,c
        sub 128
        jp nc,chain_dma

        ld a,h
        ld [HDMA1],a
        ld a,l
        ld [HDMA2],a
        ld a,d
        ld [HDMA3],a
        ld a,e
        ld [HDMA4],a
        ld a,c
        dec a 
        and $7f
        ld [HDMA5],a
        ret

chain_dma:

        ld a,c
        and $80
        jp nz,above_max
        ld a,c
above_max:
        ld b,a         ; b = cnt = (count>128)?128:count;

        ld a,h
        ld [HDMA1],a
        ld a,l
        ld [HDMA2],a
        ld a,d
        ld [HDMA3],a
        ld a,e
        ld [HDMA4],a
        ld a,b
        dec a 
        and $7f
        ld [HDMA5],a

        ld a,c
        sub b            ; count -= cnt;
        ret z            ; if (count == 0 ) return;
        ld c,a

        push bc

        ld a,b
        ld c,a
        ld b,0
        shl16_bc 4       ; bc = cnt*16;

        ld a,l
        add c
        ld l,a
        ld a,h
        adc b
        ld h,a           ; hl += cnt*16

        ld a,e
        add c
        ld e,a
        ld a,d
        adc b
        ld d,a           ; de += cnt*16

        pop bc
        jp chain_dma

