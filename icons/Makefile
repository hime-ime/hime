include ../config.mak

ECHO   = /bin/echo

DIR    = taylorchu

all:

install:
	install -d $(datadir)/pixmaps
	install -m 644 hime.png $(datadir)/pixmaps
	install -d $(HIME_ICON_DIR_i)
	install -m 644 30x30/*.png $(HIME_ICON_DIR_i)
	@for d in $(DIR); do \
	   set -x ; \
	   $(ECHO) -e "\x1b[1;32m** installing icons/$$d\x1b[0m"; \
	   install -d $(HIME_ICON_DIR_i)/$$d ; \
	   install -m 644 $$d/*.png $(HIME_ICON_DIR_i)/$$d ; \
	done

clean:

