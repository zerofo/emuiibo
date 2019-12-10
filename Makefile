
export EMUIIBO_MAJOR := 0
export EMUIIBO_MINOR := 4
export EMUIIBO_MICRO := 0

.PHONY: all dev clean

base:
	@$(MAKE) -C emuiibo/
	@rm -rf $(CURDIR)/SdOut
	@mkdir -p $(CURDIR)/SdOut/contents/0100000000000352/flags
	@touch $(CURDIR)/SdOut/contents/0100000000000352/flags/boot2.flag
	@cp $(CURDIR)/emuiibo/emuiibo.nsp $(CURDIR)/SdOut/contents/0100000000000352/exefs.nsp

setdev:
	$(eval export EMUIIBO_DEV := true)

nodev:
	$(eval export EMUIIBO_DEV := false)
	@echo
	@echo WARNING! Building in development mode - use this at your own risk...
	@echo

all: nodev base
dev: setdev base

clean:
	@rm -rf $(CURDIR)/SdOut
	@$(MAKE) clean -C emuiibo/