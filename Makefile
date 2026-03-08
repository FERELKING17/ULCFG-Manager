EE_CC = mips64r5900el-ps2-elf-gcc
EE_BIN = ulcfg.elf
EE_OBJS = main.o
EE_LIBS = -lpad -lfileXio -lmc -lusbd -lpoweroff -lfat
EE_CFLAGS = -O2 -G0 -I$(PS2SDK)/ee/include -I$(PS2SDK)/common/include
EE_LDFLAGS = -L$(PS2SDK)/ee/lib -L$(PS2SDK)/common/lib

all: $(EE_BIN)

clean:
	rm -f *.elf *.o

$(EE_BIN): $(EE_OBJS)
	$(EE_CC) $(EE_CFLAGS) $(EE_OBJS) -o $(EE_BIN) $(EE_LDFLAGS) $(EE_LIBS)

%.o: %.c
	$(EE_CC) $(EE_CFLAGS) -c $< -o $@