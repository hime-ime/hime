include ../config.mak

.PHONY: all
all:

.PHONY: install
install:
	install -d "$(man1dir)"
	install -m 644 *.1 "$(man1dir)"

.PHONY: uninstall
uninstall:
	cd $(man1dir) && rm -f *.1
	rmdir --ignore-fail-on-non-empty $(man1dir)

.PHONY: clean
clean:
	@echo "clean up"
	@rm -f *~

