####################################################################################################
#
# flipflip's ESP8266 Tschenggins Lämpli
#
# Copyright (c) 2018 Philippe Kehl <flipflip at oinkzwurgl dot org>
#
####################################################################################################

SKETCH  := TschengginsLaempli.ino
ARDUINO := arduino
PORT    := /dev/ttyUSB0
SRCS    := $(SKETCH) $(sort $(wildcard src/*.h) $(wildcard src/*.c) $(wildcard src/*.cpp))
DEPS    := Makefile tools/debug.pl tools/gen_config.h
PERL    := perl
TOUCH   := touch
RM      := rm
MV      := mv
SED     := sed

.PHONY: defaulttarget
defaulttarget:
	@echo "Make what?! Try 'make help'..."

####################################################################################################
# canned receipe for verify, build, clean, etc.

target_names   :=
targets_verify :=
targets_upload :=
targets_clean  :=
targets_trace  :=

define makeRules

target_names += $(strip $(1))

targets_verify += $(strip $(1))-verify

.PHONY: $(strip $(1))-verify
$(strip $(1))-verify: build-$(strip $(1))/.verify

build-$(strip $(1))/.verify: $(SRCS)
	$(PERL) tools/gen_config_h.pl $(1)
	$(ARDUINO) --verify --preserve-temp-files --verbose --board $(2) $$< --pref build.path=$$(dir $$@) # 2>&1 | sed -r 's|/.*?/build-$(strip $(1))/sketch/||g'
	$(TOUCH) $$@

targets_upload += $(strip $(1))-upload

.PHONY: $(strip $(1))-upload
$(strip $(1))-upload: $(SRCS)
	$(PERL) tools/gen_config_h.pl $(1)
	$(ARDUINO) --upload --preserve-temp-files --verbose --board $(2) $$< --pref build.path=build-$(strip $(1)) --port $(PORT) # 2>&1 | sed -r 's|/.*?/build-$(strip $(1))/sketch/||g'

targets_clean += $(strip $(1))-clean

.PHONY: $(strip $(1))-clean
$(strip $(1))-clean:
	$(RM) -rf build-$(strip $(1))

targets_trace += $(strip $(1))-trace

.PHONY: $(strip $(1))-trace
$(strip $(1))-trace: build-$(strip $(1))/$(SKETCH).elf
	$(PERL) tools/EspStackTraceDecoder.pl build-$(strip $(1))/$(SKETCH).elf -

endef

####################################################################################################
# generate target receipes

BOARD_ESP8266_D1MINI := esp8266:esp8266:d1_mini:xtal=80,vt=flash,exception=enabled,ssl=basic,eesz=4M2M,ip=lm2f,dbg=Disabled,lvl=None____,wipe=none,baud=3000000
$(eval $(call makeRules, esp8266-d1mini, $(BOARD_ESP8266_D1MINI)))

BOARD_ESP32_GITTA := esp32:esp32:esp32:PSRAM=disabled,PartitionScheme=minimal,CPUFreq=160,FlashMode=dio,FlashFreq=40,FlashSize=2M,UploadSpeed=921600,DebugLevel=debug
$(eval $(call makeRules, esp32-gitta, $(BOARD_ESP32_GITTA)))

####################################################################################################

.PHONY: help
help:
	@echo
	@echo "Usage:"
	@echo
	@echo "    make <target> [PORT=...] [ARDUINO=...]"
	@echo
	@echo "Where <target> can be:"
	@echo
	@echo "    <name>-verify    build (verify) sketch"
	@echo "    <name>-upload    build and upload sketch"
	@echo "    <name>-trace     decode stack trace"
	@echo "    <name>-clean     clean build directory"
	@echo "    monitor          show serial output"
	@echo "    clean            clean all build directories"
	@echo "    verify           build (verify) sketch for all <name>s"
	@echo
	@echo "The following <name>s are available:"
	@echo
	@echo "    $(target_names)"
	@echo
	@echo "Optional parameters:"
	@echo
	@echo "    PORT      serial port (default: $(PORT))"
	@echo "    ARDUINO   path to arduino binary (default: $(ARDUINO))"
	@echo
	@echo "Example to build, upload and start serial monitor:"
	@echo
	@echo "   make $(firstword $(target_names))-upload monitor"
	@echo

.PHONY: debugmf
debugmf:
	@echo "target_names: $(target_names)"
	@echo "targets_verify: $(targets_verify)"
	@echo "targets_upload: $(targets_upload)"
	@echo "targets_trace: $(targets_trace)"
	@echo "targets_clean: $(targets_clean)"

.PHONY: verify
verify: $(targets_verify)

.PHONY: clean
clean: $(targets_clean)
	rm -f src/config.h

.PHONY: monitor
monitor:
	perl tools/debug.pl $(PORT):115200

####################################################################################################
# eof