CARTTYPE  EQU 2
ROMSIZE   EQU 0
RAMSIZE   EQU 3
MANUCODE  EQU 1
VERSION   EQU 1

SECTION "Main Mem", BSS[$C000]

SECTION "SpriteOAM", BSS[$C100]

SECTION "Header",home[$100]

  nop
  jp reset 

  DB $CE,$ED,$66,$66,$CC,$0D,$00,$0B,$03,$73,$00,$83,$00,$0C,$00,$0D
  DB $00,$08,$11,$1F,$88,$89,$00,$0E,$DC,$CC,$6E,$E6,$DD,$DD,$D9,$99
  DB $BB,$BB,$67,$63,$6E,$0E,$EC,$CC,$DD,$DC,$99,$9F,$BB,$B9,$33,$3E
  ;Title of the game
  db 0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,128 ; Filled in by RGBFIX
  ;Not used
  db 0,0,0; last should be 3, won't work in emu unles it's 0
  ;Cartridge type:
  db CARTTYPE
  ;Rom Size:
  db ROMSIZE
  ;Ram Size:
  db RAMSIZE
  ;Manufacturer code:
  db 00, $33
  ;Version Number
  db VERSION
  ;Complement check
  db $0a
  ;Checksum
  dw 0

reset:
        di
        ld SP,$fff4     ; init stack pointer

        ld a,1
        ldio [$4d],a
        stop            ; enable double speed

        xor a
        ldio [$42],a
        ldio [$43],a
        ldio [$40],a

        include "std.asm"

main:
        jr main

SECTION "foobar", CODE[$4000], BANK[1]

        nop
        nop
        nop
