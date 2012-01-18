SPEC         = hime-fedora.spec
VERSION_FILE = ../../ChangeLog


all: $(SPEC)

hime-fedora.spec: hime-fedora.spec.in $(VERSION_FILE)
	@echo "building $@ ..."
	@rm -f $@
	@sed -e "s/__hime_version__/`head -n 1 $(VERSION_FILE)`/" < $< > $@

clean:
	@rm -f $(SPEC)

