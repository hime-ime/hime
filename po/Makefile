include ../config.mak

LANG_FILE = $(shell ls -d *.po | sed 's/\.po//g')

ifeq ($(ECHO),)
ECHO  := $(shell whereis -b echo | awk '{print $$2}')
ifeq ($(ECHO),)
ECHO   = echo
endif
endif

.PHONY: all
all: gmo

gmo:
	@set -x ; for d in $(LANG_FILE); do \
	   $(ECHO) -e "\x1b[1;32m** generating $$d.gmo\x1b[0m"; \
	   rm -f $$d.gmo ; \
	   msgfmt -v -o $$d.gmo $$d.po ; \
	done

hime.pot:
	xgettext --from-code=UTF-8 --keyword=_ --keyword=N_ --no-wrap -o $@ ../*/*.c ../src/*/*.c*

update-po: hime.pot
	@set -x ; for d in $(LANG_FILE); do \
	   $(ECHO) -e "\x1b[1;32m** updating $$d.po\x1b[0m"; \
	   msgmerge --no-wrap -v $$d.po hime.pot > $$d.new.po; \
	   mv $$d.new.po $$d.po ; \
	done

.PHONY: install
install:
	@set -x ; for d in $(LANG_FILE); do \
	   $(ECHO) -e "\x1b[1;32m** installing $$d.mo\x1b[0m"; \
	   install -d "$(datadir)/locale/$$d/LC_MESSAGES" ; \
	   install -m 644 $$d.gmo "$(datadir)/locale/$$d/LC_MESSAGES/hime.mo"; \
	done

.PHONY: uninstall
uninstall:
	@set -x ; for d in $(LANG_FILE); do \
	   $(ECHO) -e "\x1b[1;32m** uninstalling $$d.mo\x1b[0m"; \
	   rm -f "$(datadir)/locale/$$d/LC_MESSAGES/hime.mo"; \
	   rmdir --ignore-fail-on-non-empty "$(datadir)/locale/$$d/LC_MESSAGES" ; \
	done

.PHONY: clean
clean:
	rm -f ./hime.pot *.gmo *.new.po
