# PS2-5 Dashboard / ps2dev + gsKit
# make MODE=480p (default, PCSX2 compatible)
# make MODE=720p (standard 16-bit double-buffered 720p, GS eDRAM)
# make MODE=720p-hires (legacy 3-pass mode; A/B diagnostics only)
MODE ?= 480p
HIRES := 0
DIAG := 0
DEMO := 0
DIRECT720 := 0
MAGH_HD := 0
MAGH_INTERLACED := 0
SINGLE720 := 0
PROBE := 0
HIRES_PASSES := 3
DIAG_VIDEO := 0
DIAG_PAD := 0
ifeq ($(MODE),720p-magh)
  MAGH_HD := 1
else ifeq ($(MODE),1080i-magh)
  MAGH_HD := 1
  MAGH_INTERLACED := 1
else ifeq ($(MODE),720p)
  HIRES := 0
  DIRECT720 := 1
else ifeq ($(MODE),720p-single)
  HIRES := 0
  DIRECT720 := 1
  SINGLE720 := 1
else ifeq ($(MODE),probe720p)
  HIRES := 0
  DIRECT720 := 1
  DEMO := 1
  PROBE := 1
else ifeq ($(MODE),720p-hires2)
  HIRES := 1
  HIRES_PASSES := 2
else ifeq ($(MODE),probe720p-hires2)
  HIRES := 1
  HIRES_PASSES := 2
  DEMO := 1
  PROBE := 1
else ifeq ($(MODE),720p-hires)
  HIRES := 1
else ifeq ($(MODE),480p)
  HIRES := 0
else ifeq ($(MODE),demo480p)
  HIRES := 0
  DEMO := 1
else ifeq ($(MODE),diag480i)
  HIRES := 0
  DIAG := 1
  DIAG_VIDEO := 0
else ifeq ($(MODE),diag576i)
  HIRES := 0
  DIAG := 1
  DIAG_VIDEO := 1
else ifeq ($(MODE),diag480p)
  HIRES := 0
  DIAG := 1
  DIAG_VIDEO := 2
else ifeq ($(MODE),diag720p-magh)
  DIAG := 1
  DIAG_VIDEO := 12
  DIAG_PAD := 1
else ifeq ($(MODE),diag1080i-magh)
  DIAG := 1
  DIAG_VIDEO := 13
  DIAG_PAD := 1
else ifeq ($(MODE),diag720p-magh-ct16)
  DIAG := 1
  DIAG_VIDEO := 14
  DIAG_PAD := 1
else ifeq ($(MODE),diag720p-scaled)
  HIRES := 0
  DIAG := 1
  DIAG_VIDEO := 3
else ifeq ($(MODE),diag720p-single)
  HIRES := 0
  DIAG := 1
  DIAG_VIDEO := 4
else ifeq ($(MODE),diag720p-double)
  HIRES := 0
  DIAG := 1
  DIAG_VIDEO := 5
else ifeq ($(MODE),diag720p-double-pad)
  HIRES := 0
  DIAG := 1
  DIAG_VIDEO := 5
  DIAG_PAD := 1
else ifeq ($(MODE),diag720p-rgb32)
  HIRES := 0
  DIAG := 1
  DIAG_VIDEO := 6
else ifeq ($(MODE),diag720p-698)
  HIRES := 0
  DIAG := 1
  DIAG_VIDEO := 8
  DIAG_PAD := 1
else ifeq ($(MODE),diag720p-704)
  HIRES := 0
  DIAG := 1
  DIAG_VIDEO := 9
  DIAG_PAD := 1
else ifeq ($(MODE),diag720p-640)
  HIRES := 0
  DIAG := 1
  DIAG_VIDEO := 10
  DIAG_PAD := 1
else ifeq ($(MODE),diag720p-480)
  HIRES := 0
  DIAG := 1
  DIAG_VIDEO := 11
  DIAG_PAD := 1
else ifeq ($(MODE),diag720p-16s)
  HIRES := 0
  DIAG := 1
  DIAG_VIDEO := 7
else
  $(error Unknown MODE. Use 480p, 720p-magh, 1080i-magh, diag720p-magh, diag1080i-magh or diagnostics)
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
ifeq ($(DIAG),1)
  SRCS := src/diag.c
else
  SRCS := $(filter-out src/diag.c,$(wildcard src/*.c))
endif
EE_OBJS := $(patsubst src/%.c,build/$(MODE)/%.o,$(SRCS))
EE_CFLAGS += -std=gnu99 -O2 -Wall -Wextra -Isrc -DPS25_HIRES=$(HIRES) -DPS25_HIRES_PASSES=$(HIRES_PASSES) -DPS25_DIRECT720=$(DIRECT720) -DPS25_MAGH_HD=$(MAGH_HD) -DPS25_MAGH_INTERLACED=$(MAGH_INTERLACED) -DPS25_SINGLE720=$(SINGLE720) -DPS25_PROBE=$(PROBE) -DPS25_DEMO=$(DEMO) -DPS25_DIAG_VIDEO=$(DIAG_VIDEO) -DPS25_DIAG_PAD=$(DIAG_PAD)
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

.PHONY: clean test 480p 720p probe720p 720p-single 720p-hires 720p-hires2 probe720p-hires2 720p-magh 1080i-magh diagnostics demo
480p:
	$(MAKE) MODE=480p all
720p:
	$(MAKE) MODE=720p all
720p-magh:
	$(MAKE) MODE=720p-magh all
1080i-magh:
	$(MAKE) MODE=1080i-magh all

probe720p:
	$(MAKE) MODE=probe720p all
720p-single:
	$(MAKE) MODE=720p-single all
720p-hires:
	$(MAKE) MODE=720p-hires all
720p-hires2:
	$(MAKE) MODE=720p-hires2 all
probe720p-hires2:
	$(MAKE) MODE=probe720p-hires2 all
diagnostics:
	$(MAKE) MODE=diag480i all
	$(MAKE) MODE=diag576i all
	$(MAKE) MODE=diag480p all
	$(MAKE) MODE=diag720p-magh all
	$(MAKE) MODE=diag720p-magh-ct16 all
	$(MAKE) MODE=diag1080i-magh all
	$(MAKE) MODE=diag720p-scaled all
	$(MAKE) MODE=diag720p-single all
	$(MAKE) MODE=diag720p-double all
	$(MAKE) MODE=diag720p-double-pad all
	$(MAKE) MODE=diag720p-698 all
	$(MAKE) MODE=diag720p-704 all
	$(MAKE) MODE=diag720p-640 all
	$(MAKE) MODE=diag720p-480 all
	$(MAKE) MODE=diag720p-rgb32 all
	$(MAKE) MODE=diag720p-16s all
demo:
	$(MAKE) MODE=demo480p all
test:
	@mkdir -p build/host
	cc -std=c99 -O2 -Wall -Wextra -pedantic -Isrc tests/catalog_test.c src/catalog.c -o build/host/catalog_test
	./build/host/catalog_test
	cc -std=c99 -O2 -Wall -Wextra -pedantic -Isrc tests/text_layout_test.c src/text_layout.c -o build/host/text_layout_test
	./build/host/text_layout_test
clean:
	rm -rf build dist
