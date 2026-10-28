# PS2-5 Dashboard / ps2dev + gsKit
# make MODE=480p (default, PCSX2 compatible)
# make MODE=720p (gsKit hires multipass, REAL PS2 ONLY)
MODE ?= 480p
ifeq ($(MODE),720p)
  HIRES := 1
else ifeq ($(MODE),480p)
  HIRES := 0
else
  $(error MODE must be 480p or 720p)
endif

# Allow native tests/cleanup without having the MIPS compiler installed.
ifneq ($(MAKECMDGOALS),test)
ifneq ($(MAKECMDGOALS),clean)
ifndef PS2SDK
  $(error PS2SDK is not set. See README.md)
endif
GSKIT ?= $(PS2DEV)/gsKit

# The PS2SDK sample rules use EE_CC and MKDIR but do not initialize them.
# Without these values, make silently interprets '-D_EE' and '-p' as
# ignore-errors recipe prefixes and can report success without an ELF.
EE_CC := mips64r5900el-ps2-elf-gcc
MKDIR := mkdir

ifeq ($(shell command -v $(EE_CC) 2>/dev/null),)
  $(error PS2 cross-compiler not found: $(EE_CC). Check PS2DEV/PATH or use the official ps2dev container)
endif

EE_BIN := dist/PS25-$(MODE).ELF
SRCS := $(wildcard src/*.c)
EE_OBJS := $(patsubst src/%.c,build/$(MODE)/%.o,$(SRCS))
EE_CFLAGS += -std=gnu99 -O2 -Wall -Wextra -Isrc -DPS25_HIRES=$(HIRES)
EE_INCS += -I$(GSKIT)/include -I$(PS2SDK)/ports/include
EE_LDFLAGS += -L$(GSKIT)/lib -L$(PS2SDK)/ports/lib
EE_LIBS += -lgskit -ldmakit -lpad -lm

.PHONY: all
all: $(EE_BIN)

build/$(MODE)/%.o: src/%.c
	@mkdir -p $(dir $@)
	$(EE_CC) $(EE_CFLAGS) $(EE_INCS) -c $< -o $@

include $(PS2SDK)/samples/Makefile.eeglobal
endif
endif

.PHONY: clean test 480p 720p
480p:
	$(MAKE) MODE=480p all
720p:
	$(MAKE) MODE=720p all
test:
	@mkdir -p build/host
	cc -std=c99 -O2 -Wall -Wextra -pedantic -Isrc tests/catalog_test.c src/catalog.c -o build/host/catalog_test
	./build/host/catalog_test
clean:
	rm -rf build dist
