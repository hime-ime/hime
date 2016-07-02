include ../config.mak

FILTER_LIST=$(shell ls|grep -v Makefile)

all:

install:
	install -d "$(filterdir)"
	install -m 755 $(FILTER_LIST) "$(filterdir)"

clean:
	@echo "clean up"
	@rm -f *~

