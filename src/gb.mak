AS=wla-gb
LD=wlalink

ASFLAGS=
LDFLAGS=-vrs

OBJ = $(SRC:.asm=.obj)

all: $(TARGET).gbc

$(TARGET).link: 
	@echo "[objects]" > main.link
	@echo $(OBJ) | sed -e 's/\ /\n/' >> main.link

$(TARGET).gbc: $(OBJ) $(TARGET).link
	$(LD) $(LDFLAGS) main.link $(TARGET).gbc

%.obj: %.asm
	$(AS) $(ASFLAGS) -o $< $@

%.asm: %.S
	$(CPP) -E -P -o $@ $<

clean:
	-rm -rf $(TARGET).{gbc,map,sym,link} *.o *.obj 
