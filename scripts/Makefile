include ../config.mak

SCRIPTS=hime-user-setup utf8-edit gtab.append_prepare html-browser tsin-gtab-import tsin-gtab-edit

all:	$(SCRIPTS)

install:
	install -d $(HIME_SCRIPT_DIR_i)
	install -m 755 $(SCRIPTS) $(HIME_SCRIPT_DIR_i)

clean:
	@echo "clean up"
	@rm -f *~ tags core.*

