include config.mak

ECHO = /bin/echo

VERSION_FILE = VERSION.hime

DIR    = src data filter scripts icons man menu

ifeq ($(USE_I18N),Y)
DIR   += po
endif


all:
	@for d in $(DIR); do $(ECHO) -e "\x1b[1;33m** processing $$d\x1b[0m"; \
	   $(MAKE) -C $$d; \
	done

install:
	@for d in $(DIR); do $(ECHO) -e "\x1b[1;32m** installing $$d\x1b[0m"; \
	   $(MAKE) -C $$d install; \
	done
	@if [ $(prefix) = /usr/local ]; then \
	   install -m 644 icons/hime.png /usr/share/pixmaps; \
	   install -d $(DOC_DIR); \
	   install -m 644 README.html ChangeLog $(DOC_DIR); \
	else \
	   install -d $(DOC_DIR_i); \
	   install -m 644 README.html ChangeLog $(DOC_DIR_i); \
	fi

clean:
	@touch src/.depend
	@for d in $(DIR); do $(ECHO) -e "\x1b[1;31m** cleanup $$d\x1b[0m"; \
	   $(MAKE) -C $$d clean; \
	done

distclean:
	@$(MAKE) clean
	@rm -f config.mak

config.mak: $(VERSION_FILE) configure
	@$(ECHO) "regenerate $@ ..."
	./configure



