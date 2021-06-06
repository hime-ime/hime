include config.mak

VERSION_FILE = ChangeLog

DIRS    = src data filter icons man menu scripts

ifeq ($(ECHO),)
ECHO  := $(shell whereis -b echo | awk '{print $$2}')
ifeq ($(ECHO),)
ECHO   = echo
endif
endif

ifeq ($(USE_I18N),Y)
DIRS   += po
endif

.PHONY: all
all:
	@for d in $(DIRS); do $(ECHO) -e "\x1b[1;33m** processing $$d\x1b[0m"; \
	   $(MAKE) -C $$d || exit 1; \
	done

.PHONY: install
install:
	@for d in $(DIRS); do $(ECHO) -e "\x1b[1;32m** installing $$d\x1b[0m"; \
	   $(MAKE) -C $$d install || exit 1; \
	done
	install -d "$(DOC_DIR)"; \
	install -m 644 $(VERSION_FILE) "$(DOC_DIR)"

.PHONY: uninstall
uninstall:
	@for d in $(DIRS); do $(ECHO) -e "\x1b[1;32m** uninstalling $$d\x1b[0m"; \
	   $(MAKE) -C $$d uninstall || exit 1; \
	done

.PHONY: clean
clean:
	@touch src/.depend
	@for d in $(DIRS); do $(ECHO) -e "\x1b[1;31m** cleanup $$d\x1b[0m"; \
	   $(MAKE) -C $$d clean; \
	done

.PHONY: distclean
distclean:
	@$(MAKE) clean
	@rm -f config.mak

config.mak: $(VERSION_FILE) configure
	@$(ECHO) "regenerate $@ ..."
	./configure

.PHONY: clang-format
clang-format:
	clang-format --version
	clang-format -i {src,data}/*.[ch] \
		src/{gtk-im,im-client,modules}/*.[ch] \
		src/qt5-im/*.h \
		src/qt5-im/*.cpp \
		--verbose
