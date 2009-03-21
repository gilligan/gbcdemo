; 
; standard functions
;

;
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

