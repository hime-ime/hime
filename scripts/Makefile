include ../config.mak

SCRIPTS=hime-user-setup utf8-edit gtab.append_prepare tsin-gtab-import
BIN_SCRIPTS=hime-env

.PHONY: all
all:	$(SCRIPTS)

.PHONY: install
install:
	install -d "$(HIME_SCRIPT_DIR)"
	install -m 755 $(SCRIPTS) "$(HIME_SCRIPT_DIR)"
	install -d "$(bindir)"
	install -m 755 $(BIN_SCRIPTS) "$(bindir)"

.PHONY: uninstall
uninstall:
	cd $(HIME_SCRIPT_DIR) && rm -f $(SCRIPTS)
	cd $(bindir) && rm -f $(BIN_SCRIPTS)
	rmdir --ignore-fail-on-non-empty $(bindir)
	rmdir --ignore-fail-on-non-empty $(HIME_SCRIPT_DIR)

.PHONY: clean
clean:
	@echo "clean up"
	@rm -f *~ tags core.*

