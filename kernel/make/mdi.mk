# Copyright 2017 The Fuchsia Authors
#
# Use of this source code is governed by a MIT-style
# license that can be found in the LICENSE file or at
# https://opensource.org/licenses/MIT

MDI_BIN := $(BUILDDIR)/mdi.bin
MDIGEN := $(BUILDDIR)/tools/mdigen

$(MDI_BIN): $(MDIGEN) $(MDI_SRCS)
	@echo generating $@
	@$(MKDIR)
	$(NOECHO)$(MDIGEN) -o $@ $(MDI_SRCS)

GENERATED += $(MDI_BIN)
EXTRA_BUILDDEPS += $(MDI_BIN)

.PHONY: mdibin
mdibin: $(MDI_BIN)
