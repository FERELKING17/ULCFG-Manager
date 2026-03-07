EE_BIN = ulcfg.elf
EE_OBJS = main.o
EE_LIBS = -lpad -lfileXio -lmc -lusbd -lpoweroff -lfat
EE_CFLAGS = -O2 -G0

include $(PS2SDK)/samples/Makefile.pref
include $(PS2SDK)/samples/Makefile.eeglobal

all: $(EE_BIN)

clean:
	rm -f *.elf *.o

include $(PS2SDK)/samples/Makefile.eerules