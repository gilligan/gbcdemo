RGBFIX=rgbfix
AS=rgbasm
LD=xlink

ASFLAGS=-i../../include/
LDFLAGS=-mmain.map -nmain.sym -tg -z00

OBJ = $(SRC:.asm=.obj)

all: $(TARGET).gbc

$(TARGET).link: 
	@echo "[objects]" > main.link
	$(foreach obj,$(OBJ),echo $(obj) >> main.link )
	@echo "[output]" >> main.link
	@echo "main.gbc" >> main.link

$(TARGET).gbc: $(OBJ) $(TARGET).link
	$(LD) $(LDFLAGS) main.link
	$(RGBFIX) -p -v main.gbc 

%.obj: %.asm
	$(AS) $(ASFLAGS) -o$@ $< 

%.asm: %.S
	$(CPP) -o $@ $<

clean:
	-rm -rf $(TARGET).gbc *.o *.obj *.map *.sym main.link
