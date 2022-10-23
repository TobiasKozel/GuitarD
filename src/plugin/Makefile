#!/usr/bin/make -f
# Makefile for DISTRHO Plugins #
# ---------------------------- #
# Created by falkTX
#

# --------------------------------------------------------------
# Project name, used for binaries

NAME = GuitarD

# --------------------------------------------------------------
# Files to build

FILES_DSP = \
	DistrhoDspDummy.cpp
	# DistrhoDsp.cpp

FILES_UI  = \
	DistrohUi.cpp

# --------------------------------------------------------------
# Do some magic

include ../../dpf/Makefile.plugins.mk

LINK_FLAGS      += $(SHARED_MEMORY_LIBS)

TARGETS += jack
# TARGETS += lv2_sep
# TARGETS += vst2
# TARGETS += vst3

all: $(TARGETS)


# --------------------------------------------------------------
