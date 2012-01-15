include ../../../config.mak

ECHO = $(shell whereis -b echo | awk '{print $$2}')

.PHONY: all
all: ed4.cpp ed4.h ed4.pro ed4a.cpp
	@ $(ECHO) -e "\x1b[1;33m** Compiling ed4...\x1b[0m"
	@ $(QT4_QMAKE_PATH) ed4.pro -o ed4.makefile && $(MAKE) -f ed4.makefile

.PHONY: clean
clean:
	-@ for FileName in ed4.makefile ed4a.o ed4.o ed4; \
	do \
		if [ -f "$$FileName" ]; then \
			$(ECHO) -e "\x1b[1;31m** "deleting $$FileName..."\x1b[0m"; \
			rm "$$FileName" ; \
		fi; \
	done
