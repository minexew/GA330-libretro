TOOLCHAIN=arm-none-eabi-
AS=$(TOOLCHAIN)as
CC=$(TOOLCHAIN)gcc
LD=$(TOOLCHAIN)ld
OBJCOPY=$(TOOLCHAIN)objcopy
OBJDUMP=$(TOOLCHAIN)objdump
SIZE=$(TOOLCHAIN)size

BUILD=build

ARCHFLAGS=-mcpu=arm926ej-s
CFLAGS=-ffreestanding -Wall -ffunction-sections -fdata-sections -Os
LDFLAGS=-nostartfiles -Wl,--section-start=.text=0x20000000

OUTPUT=minisys.bin
ELF=minisys.elf
OBJ_NAMES=init.o syscalls.o thunks.o
OBJS=$(addprefix $(BUILD)/,$(OBJ_NAMES))

$(shell mkdir -p $(BUILD))

all: $(OUTPUT)

clean:
	rm -f $(OUTPUT) $(ELF) $(BUILD)/*.o

$(OUTPUT): $(ELF)
	$(OBJCOPY) -O binary $< $@

$(ELF): $(OBJS)
	$(CC) $(ARCHFLAGS) $(LDFLAGS) -o $@ $(OBJS)
	$(OBJDUMP) -Drtx $@ >$(BUILD)/minisys.objdump

$(BUILD)/%.o: %.c
	$(CC) $(ARCHFLAGS) $(CFLAGS) -c -o $@ $<

$(BUILD)/%.o: %.s
	$(AS) $(ARCHFLAGS) -c -o $@ $<
