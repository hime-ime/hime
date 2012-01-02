include ../../../config.mak

ECHO = $(shell whereis -b echo | awk '{print $$2}')

.PHONY: all
all: ed3.cpp ed3.h ed3.pro ed3a.cpp
	@ $(ECHO) -e "\x1b[1;33m** Compiling ed3...\x1b[0m"
	@ $(QT3_QMAKE_PATH) ed3.pro -o ed3.makefile && $(MAKE) -f ed3.makefile

.PHONY: clean
clean:
	-@ for FileName in ed3.makefile ed3a.o ed3.o ed3; \
	do \
		if [ -f "$$FileName" ]; then \
			$(ECHO) -e "\x1b[1;31m** "deleting $$FileName..."\x1b[0m"; \
			rm "$$FileName" ; \
		fi; \
	done
