SPEC         = hime-fedora.spec
HIME_VERSION = ../../VERSION.hime


all: $(SPEC)

hime-fedora.spec: hime-fedora.spec.in $(HIME_VERSION)
	@echo "building $@ ..."
	@rm -f $@
	@sed -e "s/__hime_version__/`cat $(HIME_VERSION)`/" < $< > $@

clean:
	@rm -f $(SPEC)

