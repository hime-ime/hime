include ../../../config.mak

ECHO = $(shell whereis -b echo | awk '{print $$2}')

.PHONY: all
all: ed5.cpp ed5.h ed5.pro ed5a.cpp
	@ $(ECHO) -e "\x1b[1;33m** Compiling ed5...\x1b[0m"
	@ $(QT5_QMAKE_PATH) ed5.pro -o ed5.makefile && $(MAKE) -f ed5.makefile

.PHONY: clean
clean:
	-@ for FileName in ed5.makefile ed5a.o ed5.o ed5; \
	do \
		if [ -f "$$FileName" ]; then \
			$(ECHO) -e "\x1b[1;31m** "deleting $$FileName..."\x1b[0m"; \
			rm "$$FileName" ; \
		fi; \
	done
