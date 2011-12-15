SPEC         = hime-fedora.spec
VERSION_FILE = ../../VERSION.hime


all: $(SPEC)

hime-fedora.spec: hime-fedora.spec.in $(VERSION_FILE)
	@echo "building $@ ..."
	@rm -f $@
	@sed -e "s/__hime_version__/`cat $(VERSION_FILE)`/" < $< > $@

clean:
	@rm -f $(SPEC)

