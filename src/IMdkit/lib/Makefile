include ../../../config.mak

CFLAGS +=  -c $(OPTFLAGS) -fno-strength-reduce -DXIM_t -DTRANS_SERVER

ifeq ($(FREEBSD), 0)
  INC = -I../include -I/usr/X11R6/include
else
  INC = -I../include -I/usr/local/include/
endif

SRC = IMConn.c IMMethod.c FrameMgr.c i18nIMProto.c i18nAttr.c i18nMethod.c i18nUtil.c i18nPtHdr.c i18nIc.c i18nClbk.c i18nX.c

OBJ = $(SRC:.c=.o)

all: libXimd.a

libXimd.a: $(OBJ)
	@echo "archiving $@ ..."
	@ar cr $@ $(OBJ)
	@ranlib $@

.c.o:
	@echo "  $< -> $@"
	$(CC) $(CPPFLAGS) $(CFLAGS) $(INC) -c $<

clean:
	@echo "clean up"
	@rm -f libXimd.a *.o *.bak
