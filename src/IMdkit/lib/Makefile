include ../../../config.mak

#CFLAGS =  -c -O2 -fno-strength-reduce -Dlinux -D__i386__ -D_POSIX_C_SOURCE=199309L -D_POSIX_SOURCE -D_XOPEN_SOURCE=500L -D_BSD_SOURCE -D_SVID_SOURCE   -DFUNCPROTO=15 -DNARROWPROTO   -DUNIXCONN -DTCPCONN -DXIM_t -DTRANS_SERVER
CFLAGS =  -c $(OPTFLAGS) -fno-strength-reduce -DXIM_t -DTRANS_SERVER
INC = -I../include -I/usr/X11R6/include


SRC = IMConn.c IMMethod.c FrameMgr.c i18nIMProto.c i18nAttr.c i18nMethod.c i18nUtil.c i18nPtHdr.c i18nIc.c i18nClbk.c i18nX.c

OBJ = $(SRC:.c=.o)

all: libXimd.a

libXimd.a: $(OBJ)
	ar cr $@ $(OBJ)
	ranlib $@

.c.o:
	$(CC) $(CFLAGS) $(INC) $<

clean:
	rm -f libXimd.a *.o *.bak
