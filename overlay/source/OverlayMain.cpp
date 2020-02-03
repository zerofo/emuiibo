#include <tesla.hpp>
#include <AmiiboUtil.hpp>
#include <nfpemu.h>
#include <sstream>

namespace emuovl {

    tsl::element::Frame *root;
    bool emu_init_ok = false;

    #define EMUOVL_DRAWER_HEADER_TEXT(txt, ...) auto header = new tsl::element::CustomDrawer(0, 0, 100, FB_WIDTH, [__VA_ARGS__](u16 x, u16 y, tsl::Screen *screen) { \
                std::string msg = "emuiibo (not available) - overlay"; \
                if(emu_init_ok) { \
                    NfpEmuVersion ver = {}; \
                    nfpemuGetVersion(&ver); \
                    msg = "emuiibo v" + std::to_string(ver.major) + "." + std::to_string(ver.minor) + "." + std::to_string(ver.micro) + " - overlay"; \
                } \
                screen->drawString(msg.c_str(), false, 15, 50, 20, tsl::a(0xFFFF)); \
                screen->drawString(txt, false, 20, 75, 15, tsl::a(0xFFFF)); \
            }); \
            root->addElement(header);

    static void updateUI();

    static void reloadUI() {

        root->clear();

        if(emu_init_ok) {

            NfpEmuEmulationStatus st = EmuEmulationStatus_Off;
            nfpemuGetStatus(&st);

            auto list = new tsl::element::List();

            if(st == EmuEmulationStatus_Off) {

                EMUOVL_DRAWER_HEADER_TEXT("emuiibo is currently off.")

                auto on_item = new tsl::element::ListItem("Toggle on emulation");
                on_item->setClickListener([](s64 keys) {
                    if(keys & KEY_A) {
                        nfpemuSetEmulationOnForever();
                        updateUI();
                        return true;
                    }
                    return false;
                });
                list->addItem(on_item);

                auto on_once_item = new tsl::element::ListItem("Toggle on emulation (once)");
                on_once_item->setClickListener([](s64 keys) {
                    if(keys & KEY_A) {
                        nfpemuSetEmulationOnOnce();
                        updateUI();
                        return true;
                    }
                    return false;
                });
                list->addItem(on_once_item);

            }
            else {

                char cur_amiibo_p[FS_MAX_PATH];
                nfpemuGetCurrentAmiibo(cur_amiibo_p);
                auto cur_amiibo = ProcessAmiibo(cur_amiibo_p);

                bool is_amiibo_selected = cur_amiibo.IsValid();

                std::string amiibo_msg = "There is no amiibo selected.";
                if(cur_amiibo.IsValid()) {
                    amiibo_msg = "Selected amiibo: '" + cur_amiibo.amiibo_name + "'";
                }

                EMUOVL_DRAWER_HEADER_TEXT(amiibo_msg.c_str(), amiibo_msg, is_amiibo_selected, cur_amiibo)

                auto amiibos = ListAmiibos();

                auto off_item = new tsl::element::ListItem("Toggle off emulation");
                off_item->setClickListener([](s64 keys) {
                    if(keys & KEY_A) {
                        nfpemuSetEmulationOff();
                        updateUI();
                        return true;   
                    }
                    return false;
                });
                list->addItem(off_item);

                for(auto amiibo: amiibos) {
                    std::string name = amiibo.amiibo_name;
                    bool is_selected = false;
                    if(cur_amiibo.IsValid()) {
                        if(name == cur_amiibo.amiibo_name) {
                            is_selected = true;
                            name = " * " + amiibo.amiibo_name;
                        }
                    }
                    auto item = new tsl::element::ListItem(amiibo.amiibo_name);
                    if(!is_selected) {
                        item->setClickListener([amiibo](s64 keys) {
                            if(keys & KEY_A) {
                                nfpemuSetCustomAmiibo(amiibo.path.c_str());
                                updateUI();
                                return true;   
                            }
                            return false;
                        });
                    }

                    list->addItem(item);
                }

            }

            root->addElement(list);

        }
        else {

            EMUOVL_DRAWER_HEADER_TEXT("Unable to access emuiibo...")

        }

        tsl::Gui::requestFocus(root, FocusDirection::UP);

    }

    static void updateUI() {
        reloadUI();
        root->layout();
    }

    class EmuiiboGui : public tsl::Gui {

        public:

            EmuiiboGui() {}
            ~EmuiiboGui() {}

            // Called on UI creation
            virtual tsl::Element *createUI() {
                root = new tsl::element::Frame();
                
                reloadUI();

                return root;
            }

            // Called once per frame
            virtual void update() {
            }

    };

    class Overlay : public tsl::Overlay {

        public:

            Overlay() {}
            ~Overlay() {}

            tsl::Gui *onSetup() {
                if(R_SUCCEEDED(smInitialize())) {
                    if(nfpemuIsAccessible()) {
                        emu_init_ok = R_SUCCEEDED(nfpemuInitialize());
                    }
                }

                return new EmuiiboGui();
            }

            void onOverlayShow(tsl::Gui *gui) override { 
                if (root != nullptr) {
                    reloadUI();
                    root->layout();
                }
                
                tsl::Gui::playIntroAnimation();
            }
            
            void onDestroy() {
                if(nfpemuIsAccessible()) {
                    nfpemuExit();
                }
                smExit();
            }
        
    };

}

tsl::Overlay *overlayLoad() {
    return new emuovl::Overlay();
}