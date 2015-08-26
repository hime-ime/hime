include ../config.mak

SCRIPTS=hime-user-setup utf8-edit gtab.append_prepare tsin-gtab-import
BIN_SCRIPTS=hime-env

all:	$(SCRIPTS)

install:
	install -d "$(HIME_SCRIPT_DIR_i)"
	install -m 755 $(SCRIPTS) "$(HIME_SCRIPT_DIR_i)"
	install -d "$(bindir)"
	install -m 755 $(BIN_SCRIPTS) "$(bindir)"

clean:
	@echo "clean up"
	@rm -f *~ tags core.*

