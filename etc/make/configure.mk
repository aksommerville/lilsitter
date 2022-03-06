# configure.mk

#-----------------------------------------------------------
# Who is building?

ifndef MA_HOST
  UNAMES:=$(shell uname -s)
  ifeq ($(UNAMES),Linux)
    # Three variations on Linux, distinguishable by host name. (i guess that's not ideal)
    UNAMEN:=$(shell uname -n)
    ifeq ($(UNAMEN),raspberrypi)
      MA_HOST:=raspi
    else ifeq ($(UNAMEN),vcs)
      MA_HOST:=linuxdrm
      MA_BUILD_TINY:=0
    else ifeq ($(UNAMEN),gigglebyte)
      MA_HOST:=linuxdrm
      MA_BUILD_TINY:=0
    else
      MA_HOST:=linuxdefault
    endif
  else ifeq ($(UNAMES),Darwin)
    MA_HOST:=macos
  else ifneq (,$(strip $(filter MINGW%,$(UNAMES))))
    MA_HOST:=mswin
  else
    $(warning Unable to determine MA_HOST. Building generic.)
    MA_HOST:=generic
  endif
endif

#---------------------------------------------------------
# What do we want to build?

ifndef MA_BUILD_TINY
  MA_BUILD_TINY:=1
else ifeq ($(MA_BUILD_TINY),0)
  MA_BUILD_TINY:=
endif

ifndef MA_BUILD_NATIVE
  ifneq (,$(strip $(filter $(MA_HOST),$(notdir $(wildcard src/platform/*)))))
    MA_BUILD_NATIVE:=1
  else
    MA_BUILD_NATIVE:=
  endif
endif

#--------------------------------------------------------
# Other shared facts.

CFILES_COMMON:=$(shell find src/common -name '*.c')
CFILES_MAIN:=$(shell find src/main -name '*.c')
HFILES_MAIN:=$(shell find src/main -name '*.h')
RUNCMD=$<
