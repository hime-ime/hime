include ../config.mak

DIR    = black dark gray pink

ifeq ($(ECHO),)
ECHO  := $(shell whereis -b echo|sed 's/^echo: *\([^ ]\+\).*/\1/')
ifeq ($(ECHO),)
ECHO   = echo
endif
endif

.PHONY: all
all:

.PHONY: install
install:
	install -d "$(datadir)/pixmaps"
	install -m 644 hime.png "$(datadir)/pixmaps"
	install -d "$(HIME_DEFAULT_ICON_DIR)"
	install -m 644 30x30/*.png "$(HIME_DEFAULT_ICON_DIR)"
	@set -x ; for d in $(DIR); do \
	   $(ECHO) -e "\x1b[1;32m** installing icons/$$d\x1b[0m"; \
	   install -d "$(HIME_DEFAULT_ICON_DIR)/$$d" ; \
	   install -m 644 $$d/*.png "$(HIME_DEFAULT_ICON_DIR)/$$d" ; \
	done

.PHONY: uninstall
uninstall:
	cd $(datadir)/pixmaps && rm -f hime.png
	cd $(HIME_DEFAULT_ICON_DIR) && rm -f *.png
	@set -x ; for d in $(DIR); do \
	   $(ECHO) -e "\x1b[1;32m** uninstalling icons/$$d\x1b[0m"; \
	   cd $(HIME_DEFAULT_ICON_DIR)/$$d && rm -f *.png && cd -; \
	   rmdir --ignore-fail-on-non-empty $(HIME_DEFAULT_ICON_DIR)/$$d; \
	done
	rmdir --ignore-fail-on-non-empty $(HIME_DEFAULT_ICON_DIR)
	rmdir --ignore-fail-on-non-empty $(datadir)/pixmaps

.PHONY: clean
clean:

