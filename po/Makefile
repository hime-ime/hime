include ../config.mak

.SUFFIXES: .po .gmo

.po.gmo:
	file=`echo $* | sed 's,.*/,,'`.gmo \
		&& rm -f $$file && msgfmt -o $$file $<

all: en_US.gmo zh_CN.gmo

hime.pot:
	xgettext --from-code=UTF-8 --keyword=_ --keyword=N_ -o $@ ../*/*.c ../src/*/*.c*

update-po: hime.pot
	msgmerge en_US.po hime.pot > en_US.new.po
	msgmerge zh_CN.po hime.pot > zh_CN.new.po
	mv en_US.new.po en_US.po
	mv zh_CN.new.po zh_CN.po

install:
	install -d $(datadir)/locale/en_US/LC_MESSAGES
	install -d $(datadir)/locale/zh_CN/LC_MESSAGES
	install -m 644 en_US.gmo $(datadir)/locale/en_US/LC_MESSAGES/hime.mo
	install -m 644 zh_CN.gmo $(datadir)/locale/zh_CN/LC_MESSAGES/hime.mo

clean:
	rm -f ./hime.pot *.gmo *.new.po
