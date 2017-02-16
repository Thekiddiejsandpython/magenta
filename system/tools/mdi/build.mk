# Copyright 2017 The Fuchsia Authors
#
# Use of this source code is governed by a MIT-style
# license that can be found in the LICENSE file or at
# https://opensource.org/licenses/MIT

LOCAL_DIR := $(GET_LOCAL_DIR)

MDIGEN := $(BUILDDIR)/tools/mdigen
MDIGEN_CFLAGS := -Isystem/public

TOOLS := $(MDIGEN)

SRC := \
    $(LOCAL_DIR)/mdigen.cpp \
    $(LOCAL_DIR)/parser.cpp \
    $(LOCAL_DIR)/tokens.cpp \

$(MDIGEN): $(SRC)
	@echo compiling $^
	@$(MKDIR)
	$(NOECHO)$(HOST_CXX) $(HOST_COMPILEFLAGS) $(HOST_CPPFLAGS) $(MDIGEN_CFLAGS) -o $@ $^

GENERATED += $(TOOLS)
EXTRA_BUILDDEPS += $(TOOLS)

# phony rule to build just the tools
.PHONY: tools
tools: $(TOOLS)
