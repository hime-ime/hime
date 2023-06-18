HIME_SCRIPTS = hime-user-setup utf8-edit gtab.append_prepare tsin-gtab-import
HIMEdir = $(HIME_SCRIPT_DIR)
BIN_SCRIPTS = hime-env
BINdir = $(bindir)

.PHONY: all
all:	$(SCRIPTS)

.PHONY: install
install:
	install -d "$(HIMEdir)"
	install -m 755 $(HIME_SCRIPTS) "$(HIMEdir)"
	install -d "$(BINdir)"
	install -m 755 $(BIN_SCRIPTS) "$(BINdir)"

.PHONY: uninstall
uninstall:
	cd $(HIMEdir) && rm -f $(HIME_SCRIPTS)
	cd $(BINdir) && rm -f $(BIN_SCRIPTS)
	rmdir --ignore-fail-on-non-empty $(BINdir)
	rmdir --ignore-fail-on-non-empty $(HIMEdir)

.PHONY: clean
clean:
	@echo "clean up"
	@rm -f *~ tags core.*

