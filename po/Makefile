include ../config.mak

LANG_FILE = $(shell ls -d *.po | sed 's/\.po//g')

ifeq ($(ECHO),)
ECHO  := $(shell whereis -b echo | awk '{print $$2}')
ifeq ($(ECHO),)
ECHO   = echo
endif
endif

all: gmo

gmo:
	@set -x ; for d in $(LANG_FILE); do \
	   $(ECHO) -e "\x1b[1;32m** generating $$d.gmo\x1b[0m"; \
	   rm -f $$d.gmo ; \
	   msgfmt -v -o $$d.gmo $$d.po ; \
	done

hime.pot:
	xgettext --from-code=UTF-8 --keyword=_ --keyword=N_ -o $@ ../*/*.c ../src/*/*.c*

update-po: hime.pot
	@set -x ; for d in $(LANG_FILE); do \
	   $(ECHO) -e "\x1b[1;32m** updating $$d.po\x1b[0m"; \
	   msgmerge -v $$d.po hime.pot > $$d.new.po ; \
	   mv $$d.new.po $$d.po ; \
	done

install:
	@set -x ; for d in $(LANG_FILE); do \
	   $(ECHO) -e "\x1b[1;32m** installing $$d.mo\x1b[0m"; \
	   install -d "$(datadir)/locale/$$d/LC_MESSAGES" ; \
	   install -m 644 $$d.gmo "$(datadir)/locale/$$d/LC_MESSAGES/hime.mo"; \
	done

clean:
	rm -f ./hime.pot *.gmo *.new.po
