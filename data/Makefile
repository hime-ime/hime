include ../config.mak

SRC_DIR       = ../src
IM_CLIENT_DIR = ../src/im-client

.SUFFIXES:	.kbmsrc .kbm .cin .gtab
CFLAGS= -DUNIX=1 $(WALL) $(OPTFLAGS) $(GTKINC) -I$(SRC_DIR)
export NO_GTK_INIT=
export HIME_NO_RELOAD=

DATA=pho.tab2 tsin32.idx \
     cj.gtab ar30.gtab ar30-big.gtab dayi3.gtab pinyin.gtab jyutping.gtab cj5.gtab \
     et.kbm et26.kbm zo.kbm pinyin.kbm dvorak.kbm mitac.kbm colemak.kbm \
     ibm.kbm tsin32 hsu.kbm pho-huge.tab2 simplex.gtab symbols.gtab russian.gtab \
     greek.gtab cj-punc.gtab latin-letters.gtab tsin-1.src pin-juyin.xlt \
     t2s.dat s2t.dat cj543.gtab s-tsin32 s-tsin32.idx s-pho.tab2 s-pho-huge.tab2 \
     pinyin-no-tone.kbm simplex-punc.gtab hangul.gtab

DATAKEEP = symbol-table gtab.list phrase.table phrase-ctrl.table

PHO_A2D    = ../src/hime-phoa2d
TS_A2D     = ../src/hime-tsa2d32
CIN2GTAB   = ../src/cin2gtab
KBMCV      = ../src/kbmcv
PIN_JUYIN  = ../src/pin-juyin


all:	$(DATA)

tsin32:	tsin.src  $(TS_A2D)
	export LD_LIBRARY_PATH=$(IM_CLIENT_DIR); $(TS_A2D) $<
tsin32.idx:	tsin32


s-tsin32:	s-tsin.src  $(TS_A2D)
	export LD_LIBRARY_PATH=$(IM_CLIENT_DIR); $(TS_A2D) $< $@
s-tsin32.idx:	s-tsin32


pho.tab2:	 pho.tab2.src $(PHO_A2D)
	export LD_LIBRARY_PATH=$(IM_CLIENT_DIR); $(PHO_A2D) $<

s-pho.tab2:	 s-pho.tab2.src $(PHO_A2D)
	export LD_LIBRARY_PATH=$(IM_CLIENT_DIR); $(PHO_A2D) $<

pho-huge.tab2:	 pho-huge.tab2.src $(PHO_A2D)
	export LD_LIBRARY_PATH=$(IM_CLIENT_DIR); $(PHO_A2D) $<

s-pho-huge.tab2:	 s-pho-huge.tab2.src $(PHO_A2D)
	export LD_LIBRARY_PATH=$(IM_CLIENT_DIR); $(PHO_A2D) $<

t2s.dat:	t2s-file t2s-file.table
	./t2s-file
s2t.dat:	t2s-file

pin-juyin.xlt: $(PIN_JUYIN) pin-juyin.src
	$(PIN_JUYIN)

extr1:	extr1.c
	@echo "building $@ ..."
	@$(CC) $(CFLAGS) $< $(SRC_DIR)/util.o $(SRC_DIR)/locale.o -o $@ $(LDFLAGS)

t2s-file:	t2s-file.c
	@echo "building $@ ..."
	@$(CC) $(CFLAGS) $< $(SRC_DIR)/util.o $(SRC_DIR)/locale.o -o $@ $(LDFLAGS)

tsin-1.src:	extr1
	./extr1 > $@

.cin.gtab: $(CIN2GTAB)
	$(CIN2GTAB) $<

.kbmsrc.kbm: $(KBMCV)
	$(KBMCV) $<

install:
	install -d $(HIME_TABLE_DIR_i)
	install -m 644 $(DATA) $(DATAKEEP) $(HIME_TABLE_DIR_i)
#	cd $(HIME_TABLE_DIR_i); bzip2 -f *.gtab

clean:
	@echo "clean up"
	@rm -f *~ tags core.* $(DATA) *.gtab extr1 t2s-file tsin-1.src tsin-gtabidx* tsin-ch-idx
