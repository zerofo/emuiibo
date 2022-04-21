
.PHONY: all emuiibo overlay dist clean

all: emuiibo overlay dist

emuiibo:
	@cd emuiibo && cargo update && cargo nx build --release

overlay:
	@$(MAKE) -C overlay/

dist:
	@rm -rf $(CURDIR)/SdOut
	@mkdir -p $(CURDIR)/SdOut/atmosphere/contents/0100000000000352/flags
	@touch $(CURDIR)/SdOut/atmosphere/contents/0100000000000352/flags/boot2.flag
	@cp $(CURDIR)/emuiibo/target/aarch64-nintendo-switch/release/emuiibo.nsp $(CURDIR)/SdOut/atmosphere/contents/0100000000000352/exefs.nsp
	@cp $(CURDIR)/emuiibo/toolbox.json $(CURDIR)/SdOut/atmosphere/contents/0100000000000352/toolbox.json
	@mkdir -p $(CURDIR)/SdOut/switch/.overlays
	@cp $(CURDIR)/overlay/emuiibo.ovl $(CURDIR)/SdOut/switch/.overlays/emuiibo.ovl
	@mkdir -p $(CURDIR)/SdOut/emuiibo/overlay
	@cp -r $(CURDIR)/overlay/lang $(CURDIR)/SdOut/emuiibo/overlay/

clean:
	@rm -rf $(CURDIR)/SdOut
	@cd emuiibo && cargo clean
	@$(MAKE) clean -C overlay/

translation:
	$(CURDIR)/overlay/tools/translation.py update "*" $(ARGS)
