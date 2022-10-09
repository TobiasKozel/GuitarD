#!/usr/bin/make -f
include dpf/Makefile.base.mk

all: dgl plugins 

ifneq ($(CROSS_COMPILING),true)
CAN_GENERATE_TTL = true
else ifneq ($(EXE_WRAPPER),)
CAN_GENERATE_TTL = true
endif

dgl:
	$(MAKE) -C dpf/dgl

plugins: dgl
	$(MAKE) all -C src/plugin

ifeq ($(CAN_GENERATE_TTL),true)
gen: plugins dpf/utils/lv2_ttl_generator
	@$(CURDIR)dpf/utils/generate-ttl.sh

utils/lv2_ttl_generator:
	$(MAKE) -C dpf/utils/lv2-ttl-generator
else
gen:
endif

clean:
	$(MAKE) clean -C dpf/dgl
	$(MAKE) clean -C src/plugin
	rm -rf bin build

.PHONY: plugins
