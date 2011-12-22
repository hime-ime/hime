include ../config.mak

all:

install:
	if [ $(prefix) = /usr/local ]; then \
	   install hime-setup.desktop /usr/share/applications; \
	   which update-menus > /dev/null 2>&1 && update-menus || exit 0; \
	else \
	   install -d $(datadir)/applications; \
	   install -m 644 hime-setup.desktop $(datadir)/applications; \
	fi

clean:
	@echo "clean up"
	@rm -f $(MENUS) *~

