include ../config.mak

DIR    = black dark gray pink

ifeq ($(ECHO),)
ECHO  := $(shell whereis -b echo|sed 's/^echo: *\([^ ]\+\).*/\1/')
ifeq ($(ECHO),)
ECHO   = echo
endif
endif

all:

install:
	install -d $(datadir)/pixmaps
	install -m 644 hime.png $(datadir)/pixmaps
	install -d $(HIME_DEFAULT_ICON_DIR_i)
	install -m 644 30x30/*.png $(HIME_DEFAULT_ICON_DIR_i)
	@set -x ; for d in $(DIR); do \
	   $(ECHO) -e "\x1b[1;32m** installing icons/$$d\x1b[0m"; \
	   install -d $(HIME_DEFAULT_ICON_DIR_i)/$$d ; \
	   install -m 644 $$d/*.png $(HIME_DEFAULT_ICON_DIR_i)/$$d ; \
	done

clean:

