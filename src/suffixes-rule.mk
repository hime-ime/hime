.SUFFIXES:	.c .o .E .pico .cpp

.cpp.o:
	@echo "  $< -> $@"
	$(CCX) $(CPPFLAGS) $(CFLAGS) -c $<
.c.o:
	@echo "  $< -> $@"
	$(CC) $(CPPFLAGS) $(CFLAGS) -c $<
.c.pico:
	@echo "  $< -> $@"
	$(CC) $(CPPFLAGS) $(CFLAGS) -c -fpic -o $@ $<
.cpp.pico:
	@echo "  $< -> $@"
	$(CCX) $(CPPFLAGS) $(CFLAGS) -c -fpic -o $@ $<
.c.E:
	@echo "  $< -> $@"
	$(CC) $(CPPFLAGS) $(CFLAGS) -E -o $@ $<
.cpp.E:
	@echo "  $< -> $@"
	$(CCX) $(CPPFLAGS) $(CFLAGS) -E -o $@ $<


CFLAGS+= $(OPTFLAGS) $(GTKINC) \
		-DGDK_DISABLE_SINGLE_INCLUDES -DGDK_DISABLE_DEPRECATED \
		-DGTK_DISABLE_SINGLE_INCLUDES -DGTK_DISABLE_DEPRECATED \
		-DG_DISABLE_SINGLE_INCLUDES \
		-DGSEAL_ENABLE \
		-DHIME_BIN_DIR=\"$(HIME_BIN_DIR)\" \
		-DHIME_DEFAULT_ICON_DIR=\"$(HIME_DEFAULT_ICON_DIR)\" \
		-DHIME_OGG_DIR=\"$(HIME_OGG_DIR)\" \
		-DHIME_SCRIPT_DIR=\"$(HIME_SCRIPT_DIR)\" \
		-DHIME_TABLE_DIR=\"$(HIME_TABLE_DIR)\" \
		-DHIME_VERSION=\"$(HIME_VERSION)\" \
		-DDOC_DIR=\"$(DOC_DIR)\" \
		-DSYS_ICON_DIR=\"$(SYS_ICON_DIR)\" \
		-DFREEBSD=$(FREEBSD) \
		-I./IMdkit/include -I./im-client
