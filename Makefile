
export EMUIIBO_MAJOR := 0
export EMUIIBO_MINOR := 6
export EMUIIBO_MICRO := 4

.PHONY: all clean

all:
	@cd emuiibo && cargo update && cargo nx --release
	@$(MAKE) -C overlay/
	@rm -rf $(CURDIR)/SdOut
	@mkdir -p $(CURDIR)/SdOut/atmosphere/contents/0100000000000352/flags
	@touch $(CURDIR)/SdOut/atmosphere/contents/0100000000000352/flags/boot2.flag
	@cp $(CURDIR)/emuiibo/target/aarch64-none-elf/release/emuiibo.nsp $(CURDIR)/SdOut/atmosphere/contents/0100000000000352/exefs.nsp
	@cp $(CURDIR)/emuiibo/toolbox.json $(CURDIR)/SdOut/atmosphere/contents/0100000000000352/toolbox.json
	@mkdir -p $(CURDIR)/SdOut/switch/.overlays
	@cp $(CURDIR)/overlay/emuiibo.ovl $(CURDIR)/SdOut/switch/.overlays/emuiibo.ovl
	@mkdir -p $(CURDIR)/SdOut/emuiibo/overlay
	@cp -r $(CURDIR)/lang $(CURDIR)/SdOut/emuiibo/overlay/

clean:
	@rm -rf $(CURDIR)/SdOut
	@cd emuiibo && cargo clean
	@$(MAKE) clean -C overlay/

translation:
	$(CURDIR)/overlay/tools/translation.py update "*" $(ARGS)
