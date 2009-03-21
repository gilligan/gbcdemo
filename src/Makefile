all: main.gbc

RGBFIX=rgbfix
AS=rgbasm
LD=xlink

ASFLAGS=
LDFLAGS=-mmain.map -nmain.sym -tg -z00

SRC = main.asm
OBJ = $(SRC:.asm=.obj)

all: main.gbc

main.link: 
	@echo "[objects]" > main.link
	$(foreach obj,$(OBJ),echo $(obj) >> main.link )
	@echo "[output]" >> main.link
	@echo "main.gbc" >> main.link

main.gbc: $(OBJ) main.link
	$(LD) $(LDFLAGS) main.link
	$(RGBFIX) -p -v main.gbc 

%.obj: %.asm
	$(AS) $(ASFLAGS) -o$@ $< 

%.asm: %.S
	$(CPP) -o $@ $<


clean:
	-rm -rf main.gbc *.o *.map *.sym main.link
