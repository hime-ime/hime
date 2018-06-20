include ../config.mak

FILTER_LIST=$(shell ls|grep -v Makefile)

.PHONY: all
all:

.PHONY: install
install:
	install -d "$(filterdir)"
	install -m 755 $(FILTER_LIST) "$(filterdir)"

.PHONY: uninstall
uninstall:
	cd $(filterdir) && rm -f $(FILTER_LIST)
	rmdir --ignore-fail-on-non-empty $(filterdir)

.PHONY: clean
clean:
	@echo "clean up"
	@rm -f *~

