
.PHONY: all emuiibo overlay dist clean

# We need to provide a custom target triple since the official tier 3 one doesn't provide crypto support
TARGET_TRIPLE := aarch64-nintendo-switch-freestanding-crypto

all: emuiibo overlay dist

emuiibo:
	@cd emuiibo && cargo update && cargo nx build --release --target $(TARGET_TRIPLE).json

overlay:
	@$(MAKE) -C overlay/

dist:
	@rm -rf $(CURDIR)/SdOut
	@mkdir -p $(CURDIR)/SdOut/atmosphere/contents/0100000000000352/flags
	@touch $(CURDIR)/SdOut/atmosphere/contents/0100000000000352/flags/boot2.flag
	@cp $(CURDIR)/emuiibo/target/$(TARGET_TRIPLE)/release/emuiibo.nsp $(CURDIR)/SdOut/atmosphere/contents/0100000000000352/exefs.nsp
	@cp $(CURDIR)/emuiibo/toolbox.json $(CURDIR)/SdOut/atmosphere/contents/0100000000000352/toolbox.json
	@mkdir -p $(CURDIR)/SdOut/switch/.overlays
	@cp $(CURDIR)/overlay/emuiibo.ovl $(CURDIR)/SdOut/switch/.overlays/emuiibo.ovl
	@mkdir -p $(CURDIR)/SdOut/emuiibo/overlay
	@cp -r $(CURDIR)/overlay/lang $(CURDIR)/SdOut/emuiibo/overlay/

clean:
	@rm -rf $(CURDIR)/SdOut
	@cd emuiibo && cargo clean
	@$(MAKE) clean -C overlay/