include ../config.mak

.SUFFIXES:	.kbmsrc .kbm .cin .gtab
CFLAGS= -DUNIX=1 $(WALL) $(OPTFLAGS) $(GTKINC) -I..
export NO_GTK_INIT=
export HIME_NO_RELOAD=

DATA=pho.tab2 tsin32.idx NewCJ3.gtab \
     cj.gtab ar30.gtab ar30-big.gtab dayi3.gtab pinyin.gtab jyutping.gtab cj5.gtab \
     et.kbm et26.kbm zo.kbm pinyin.kbm dvorak.kbm mitac.kbm colemak.kbm \
     ibm.kbm tsin32 hsu.kbm pho-huge.tab2 simplex.gtab symbols.gtab russian.gtab \
     greek.gtab cj-punc.gtab latin-letters.gtab tsin-1.src pin-juyin.xlt \
     t2s.dat s2t.dat cj543.gtab s-tsin32 s-tsin32.idx s-pho.tab2 s-pho-huge.tab2 \
     pinyin-no-tone.kbm simplex-punc.gtab hangul.gtab

DATAKEEP=symbol-table gtab.list phrase.table phrase-ctrl.table

all:	$(DATA)

tsin32:	tsin.src ../hime-tsa2d32
	export LD_LIBRARY_PATH=../im-client;../hime-tsa2d32 $<
tsin32.idx:	tsin32


s-tsin32:	s-tsin.src ../hime-tsa2d32
	export LD_LIBRARY_PATH=../im-client;../hime-tsa2d32 $< $@
s-tsin32.idx:	s-tsin32


pho.tab2:	 pho.tab2.src ../hime-phoa2d
	export LD_LIBRARY_PATH=../im-client;../hime-phoa2d $<

s-pho.tab2:	 s-pho.tab2.src ../hime-phoa2d
	export LD_LIBRARY_PATH=../im-client;../hime-phoa2d $<

pho-huge.tab2:	 pho-huge.tab2.src ../hime-phoa2d
	export LD_LIBRARY_PATH=../im-client;../hime-phoa2d $<

s-pho-huge.tab2:	 s-pho-huge.tab2.src ../hime-phoa2d
	export LD_LIBRARY_PATH=../im-client;../hime-phoa2d $<

t2s.dat:	t2s-file t2s-file.table
	./t2s-file
s2t.dat:	t2s-file

pin-juyin.xlt:	../pin-juyin pin-juyin.src
	../pin-juyin

extr1:	extr1.c
	@echo "building $@ ..."
	@$(CC) $(CFLAGS) $< ../util.o ../locale.o -o $@ $(LDFLAGS)

t2s-file:	t2s-file.c
	@echo "building $@ ..."
	@$(CC) $(CFLAGS) $< ../util.o ../locale.o -o $@ $(LDFLAGS)

tsin-1.src:	extr1
	./extr1 > $@

.cin.gtab:      ../cin2gtab
	../cin2gtab $<

.kbmsrc.kbm:	../kbmcv
	../kbmcv $<

install:
	install -d $(HIME_TABLE_DIR_i)
	install -m 644 $(DATA) $(DATAKEEP) $(HIME_TABLE_DIR_i)
#	cd $(HIME_TABLE_DIR_i); bzip2 -f *.gtab

clean:
	@echo "clean up"
	@rm -f *~ tags core.* $(DATA) *.gtab extr1 t2s-file tsin-1.src tsin-gtabidx* tsin-ch-idx
