all: main.gbc

UCON=$(PWD)/../tools/ucon64
AS=$(PWD)/../tools/rgbasm
LD=$(PWD)/../tools/xlink

ASFLAGS=
LDFLAGS=-mmain.map -nmain.sym -tg

SRC = main.asm
OBJ = $(SRC:.asm=.o)

main.link: 
	@echo "[objects]" > main.link
	$(foreach obj,$(OBJ),echo $(obj) >> main.link )
	@echo "[output]" >> main.link
	@echo "main.gbc" >> main.link

main.gbc: $(OBJ) main.link
	$(LD) $(LDFLAGS) main.link
	$(UCON) --chk --nbak $@ >/dev/null

%.o: %.asm
	$(AS) $(ASFLAGS) -o$@ $< 

clean:
	-rm -rf main.gbc *.o *.map *.sym main.link
