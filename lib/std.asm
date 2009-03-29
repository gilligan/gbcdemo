.equ temp_blue 0
.equ temp_green 1
.equ temp_red 2
.equ tmp 3

; 
; generic functions
;

; input: [hl] = src palette data
;        [de] = dst palette data
;         c   = color delta added to R/G/B channel
;         b   = number of colors
; output: 
;        [de] = altered palette
build_faded_palette:

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


color_change_lp:

        push bc  ; save count/delta

        ld a,[hl+]
        ld c,a
        ld a,[hl+]
        ld b,a   ; b=blue|green, c=green|red
        
;
; get color channels
;

        ld a,c
        and %11111
        ld [SCRATCH+temp_red],a

        shr16_bc 5
        ld a,c
        and %11111
        ld [SCRATCH+temp_green],a

        shr16_bc 5
        ld a,c
        ld [SCRATCH+temp_blue],a

        pop bc   ; restore count/delta
        push de  ; save dest
;
; add color delta to channels
;

        ld a,[SCRATCH+temp_red]
        add c
        ld e,a
        and %11100000
        jp z,store_red
        ld e,$1f
store_red:
        ld a,e
        ld [SCRATCH+temp_red],a


        ld a,[SCRATCH+temp_green]
        add c
        ld e,a
        and %11100000
        jp z,store_green
        ld e,$1f
store_green:        
        ld a,e
        ld [SCRATCH+temp_green],a


        ld a,[SCRATCH+temp_blue]
        add c
        ld e,a
        and %11100000
        jp z,store_blue
        ld e,$1f
store_blue:
        ld a,e
        ld [SCRATCH+temp_blue],a


        pop de    ; restore dest 

;
; recompose 15bit RGB word
;
        push hl   ; save src

        ld h,0
        ld l,0

        ld a,[SCRATCH+temp_blue]
        ld l,a
        shl16_hl 5
        ld a,[SCRATCH+temp_green]
        or l
        ld l,a
        shl16_hl 5
        ld a,[SCRATCH+temp_red]
        or l
        ld l,a

        ld a,l
        ld [de],a
        inc de
        ld a,h
        ld [de],a
        inc de

        pop hl    ; restore src
        dec b
        jp nz,color_change_lp

        ret


;
; hl = palette to fade
; c  = constant to add to R/G/B channels  
palette_add_constant:

        ld b,4
add_lp:

        push bc  ; save count/constant

        push hl
        ld a,[hl+]
        ld c,a
        ld a,[hl+]
        ld b,a     ; b=blue|green, c=green|red
        pop hl

        ld a,c
        and %11111
        ld [SCRATCH+temp_red],a

        shr16_bc 5
        ld a,c
        and %11111
        ld [SCRATCH+temp_green],a

        shr16_bc 5
        ld a,c
        ld [SCRATCH+temp_blue],a

        pop bc   ; restore count/constant

        ; check if constant is
        ; positive or negative
        bit 7,b
        jp z,neg_const
        ld a,$1f
        jp store_tmp

neg_const:
        ld a,$00
store_tmp:
        ld [SCRATCH+tmp],a
        
;
; add color delta to channels
;

        ld a,[SCRATCH+temp_red]
        add c
        ld e,a
        and %11100000
        jp z,__store_red
        ld a,[SCRATCH+tmp]
        ld e,a
__store_red:
        ld a,e
        ld [SCRATCH+temp_red],a


        ld a,[SCRATCH+temp_green]
        add c
        ld e,a
        and %11100000
        jp z,__store_green
        ld a,[SCRATCH+tmp]
        ld e,a
__store_green:        
        ld a,e
        ld [SCRATCH+temp_green],a


        ld a,[SCRATCH+temp_blue]
        add c
        ld e,a
        and %11100000
        jp z,__store_blue
        ld a,[SCRATCH+tmp]
        ld e,a
__store_blue:
        ld a,e
        ld [SCRATCH+temp_blue],a

        ld d,0
        ld e,0

        ld a,[SCRATCH+temp_blue]
        ld e,a
        shl16_de 5
        ld a,[SCRATCH+temp_green]
        or e
        ld e,a
        shl16_de 5
        ld a,[SCRATCH+temp_red]
        or e
        ld e,a

        ld a,e
        ld [hl+],a
        ld a,d
        ld [hl+],a
        
        dec b
        jp nz, add_lp




;
; hl = palette to fade
; c  = constant to sub from R/G/B channels
palette_sub_constant:




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

