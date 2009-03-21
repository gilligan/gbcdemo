all: main.gbc

RGBFIX=rgbfix
AS=rgbasm
LD=xlink

ASFLAGS=
LDFLAGS=-mmain.map -nmain.sym -tg -z00

SRC = main.asm
OBJ = $(SRC:.asm=.o)

main.link: 
	@echo "[objects]" > main.link
	$(foreach obj,$(OBJ),echo $(obj) >> main.link )
	@echo "[output]" >> main.link
	@echo "main.gbc" >> main.link

main.gbc: $(OBJ) main.link
	$(LD) $(LDFLAGS) main.link
	$(RGBFIX) -p -v main.gbc 

%.o: %.asm
	$(AS) $(ASFLAGS) -o$@ $< 

clean:
	-rm -rf main.gbc *.o *.map *.sym main.link
