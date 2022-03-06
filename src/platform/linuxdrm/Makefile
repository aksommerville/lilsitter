# Makefile for "linuxdrm" platform.
# Suitable for modern Linux systems, if there's no X server, eg Atari VCS.
# I'm using PulseAudio initially for no particular reason. Consider copying ALSA from raspi if there's any problems.

_CCDEF:=-DMA_APP_NAME=\"$(PROJECT_NAME)\" -DMA_NON_TINY=1
_CCINC:=-Isrc/platform/linuxdrm -Isrc/common -I/usr/include/libdrm
_CCERR:=-Werror -Wimplicit

ifeq ($(shell uname -n),gigglebyte)
  _CCDEF+=-DMA_ALSA_DEVICE_NAME='"hw:0,3"'
else ifeq ($(shell uname -n),vcs)
  _CCDEF+=-DMA_ALSA_DEVICE_NAME='"hw:0,3"'
else
  _CCDEF+=-DMA_ALSA_DEVICE_NAME='"default"'
endif

linuxdrm_CC:=gcc -c -MMD -O2 $(_CCINC) $(_CCERR) $(_CCDEF)
linuxdrm_LD:=gcc
linuxdrm_LDPOST:=-lasound -lpthread -ldrm -lgbm

linuxdrm_MIDDIR:=mid/platform/linuxdrm
linuxdrm_OUTDIR:=out/linuxdrm

linuxdrm_EXE:=$(linuxdrm_OUTDIR)/$(PROJECT_NAME)

linuxdrm_CFILES:=$(shell find src/platform/linuxdrm -name '*.c') $(CFILES_COMMON) $(CFILES_MAIN)
linuxdrm_OFILES:=$(patsubst src/%.c,$(linuxdrm_MIDDIR)/%.o,$(linuxdrm_CFILES))
linuxdrm_OFILES+=$(patsubst mid/data/%.c,$(linuxdrm_MIDDIR)/data/%.o,$(DATA_MIDFILES_EMBED))
-include $(linuxdrm_OFILES:.o=.d)

$(linuxdrm_MIDDIR)/%.o:src/%.c;$(PRECMD) $(linuxdrm_CC) -o $@ $<
$(linuxdrm_MIDDIR)/data/%.o:mid/data/%.c;$(PRECMD) $(linuxdrm_CC) -o $@ $<

$(linuxdrm_EXE):$(linuxdrm_OFILES);$(PRECMD) $(linuxdrm_LD) -o $@ $^ $(linuxdrm_LDPOST)
RUNCMD=$< --files=out/data
