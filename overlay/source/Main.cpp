
#define TESLA_INIT_IMPL
#include <emuiibo.hpp>
#include <libtesla_ext.hpp>
#include <dirent.h>

namespace {

    bool g_emuiibo_init_ok = false;
    bool g_in_second_menu = false;
    bool g_active_amiibo_valid = false;
    bool g_current_app_intercepted = false;
    char g_emuiibo_amiibo_dir[FS_MAX_PATH];
    emu::Version g_emuiibo_version;

    char g_active_amiibo_path[FS_MAX_PATH];
    emu::VirtualAmiiboData g_active_amiibo_data;

    inline bool IsActiveAmiiboValid() {
        return strlen(g_active_amiibo_path) > 0;
    }

    inline void UpdateActiveAmiibo() {
        emu::GetActiveVirtualAmiibo(&g_active_amiibo_data, g_active_amiibo_path, FS_MAX_PATH);
    }

    // Returns true if the value changed
    inline bool UpdateCurrentApplicationIntercepted() {
        bool ret = false;
        emu::IsCurrentApplicationIdIntercepted(&ret);
        if(ret != g_current_app_intercepted) {
            g_current_app_intercepted = ret;
            return true;
        }
        return false;
    }

    inline std::string MakeActiveAmiiboText() {
        if(g_active_amiibo_valid) {
            return std::string("Active virtual amiibo: ") + g_active_amiibo_data.name;
        }
        return "No active virtual amiibo";
    }

    inline std::string MakeTitleText() {
        if(!g_emuiibo_init_ok) {
            return "emuiibo";
        }
        return "emuiibo v" + std::to_string(g_emuiibo_version.major) + "." + std::to_string(g_emuiibo_version.minor) + "." + std::to_string(g_emuiibo_version.micro) + " (" + (g_emuiibo_version.dev_build ? "dev" : "release") + ")";
    }

    inline std::string MakeStatusText() {
        if(!g_emuiibo_init_ok) {
            return "emuiibo was not accessed.";
        }
        std::string msg = "Emulation: ";
        auto e_status = emu::GetEmulationStatus();
        switch(e_status) {
            case emu::EmulationStatus::On: {
                msg += "on\n";
                auto v_status = emu::GetActiveVirtualAmiiboStatus();
                switch(v_status) {
                    case emu::VirtualAmiiboStatus::Invalid: {
                        msg += "No active virtual amiibo.";
                        break;
                    }
                    case emu::VirtualAmiiboStatus::Connected: {
                        msg += "Virtual amiibo: ";
                        msg += g_active_amiibo_data.name;
                        msg += " (connected - select to disconnect)";
                        break;
                    }
                    case emu::VirtualAmiiboStatus::Disconnected: {
                        msg += "Virtual amiibo: ";
                        msg += g_active_amiibo_data.name;
                        msg += " (disconnected - select to connect)";
                        break;
                    }
                }
                msg += "\n";
                if(g_current_app_intercepted) {
                    msg += "Current game is being intercepted by emuiibo.";
                }
                else {
                    msg += "Current game is not being intercepted.";
                }
                break;
            }
            case emu::EmulationStatus::Off: {
                msg += "off";
                break;
            }
        }
        return msg;
    }

}

class AmiiboList : public tsl::Gui {

    private:
        tsl::elm::CustomOverlayFrame *root_frame;
        tsl::elm::BigCategoryHeader *selected_header;
        tsl::elm::CategoryHeader *count_header;
        tsl::elm::List *list;
        tsl::elm::List *header_list;
        std::string amiibo_path;

    public:
        AmiiboList(const std::string &path) : root_frame(new tsl::elm::CustomOverlayFrame(MakeTitleText(), MakeStatusText())), amiibo_path(path) {}

        bool OnItemClick(u64 keys, const std::string &path) {
            if(keys & KEY_A) {
                char amiibo_path[FS_MAX_PATH] = {0};
                strcpy(amiibo_path, path.c_str());
                if(IsActiveAmiiboValid()) {
                    if(strcmp(g_active_amiibo_path, amiibo_path) == 0) {
                        // User selected the active amiibo, so let's change connection then
                        auto status = emu::GetActiveVirtualAmiiboStatus();
                        switch(status) {
                            case emu::VirtualAmiiboStatus::Connected: {
                                emu::SetActiveVirtualAmiiboStatus(emu::VirtualAmiiboStatus::Disconnected);
                                root_frame->setSubtitle(MakeStatusText());
                                break;
                            }
                            case emu::VirtualAmiiboStatus::Disconnected: {
                                emu::SetActiveVirtualAmiiboStatus(emu::VirtualAmiiboStatus::Connected);
                                root_frame->setSubtitle(MakeStatusText());
                                break;
                            }
                            default:
                                break;
                        }
                        return true;
                    }
                }
                // Set active amiibo and update our active amiibo value
                emu::SetActiveVirtualAmiibo(amiibo_path, FS_MAX_PATH);
                UpdateActiveAmiibo();
                selected_header->setText(MakeActiveAmiiboText());
                root_frame->setSubtitle(MakeStatusText());
                return true;   
            }
            return false;
        }

        virtual tsl::elm::Element *createUI() override {
            list = new tsl::elm::List();
            header_list = new tsl::elm::List();

            u32 count = 0;
            tsl::hlp::doWithSDCardHandle([&](){
                auto dir = opendir(this->amiibo_path.c_str());
                if(dir) {
                    while(true) {
                        auto entry = readdir(dir);
                        if(entry == nullptr) {
                            break;
                        }
                        char path[FS_MAX_PATH] = {0};
                        sprintf(path, "%s/%s", this->amiibo_path.c_str(), entry->d_name);
                        // Find virtual amiibo
                        emu::VirtualAmiiboData data = {};
                        if(R_SUCCEEDED(emu::TryParseVirtualAmiibo(path, FS_MAX_PATH, &data))) {
                            auto item = new tsl::elm::SmallListItem(data.name);
                            item->setClickListener(std::bind(&AmiiboList::OnItemClick, this, std::placeholders::_1, std::string(path)));
                            list->addItem(item);
                            count++;
                        }
                    }
                    closedir(dir);
                }
            });

            selected_header = new tsl::elm::BigCategoryHeader(MakeActiveAmiiboText(), true);
            count_header = new tsl::elm::CategoryHeader("Available virtual amiibos (" + std::to_string(count) + ")", true);

            header_list->addItem(selected_header);
            header_list->addItem(count_header);

            root_frame->setHeader(header_list);
            root_frame->setContent(list);
            return root_frame;
        }

        virtual void update() override {
            if(UpdateCurrentApplicationIntercepted()) {
                root_frame->setSubtitle(MakeStatusText());
            }
        }

};

class CategoryList : public tsl::Gui {

    private:
        tsl::elm::CustomOverlayFrame *root_frame;
        tsl::elm::BigCategoryHeader *selected_header;
        tsl::elm::CategoryHeader *count_header;
        tsl::elm::List *list;
        tsl::elm::List *header_list;

    public:
        CategoryList() : root_frame(new tsl::elm::CustomOverlayFrame(MakeTitleText(), MakeStatusText())) {}

        static bool OnItemClick(u64 keys, const std::string &path) {
            if(keys & KEY_A) {
                tsl::changeTo<AmiiboList>(path);
                return true;
            }
            return false;
        }

        virtual tsl::elm::Element *createUI() override {
            list = new tsl::elm::List();
            header_list = new tsl::elm::List();

            // Root
            auto root_item = new tsl::elm::SmallListItem("<root>");
            root_item->setClickListener(std::bind(&CategoryList::OnItemClick, std::placeholders::_1, g_emuiibo_amiibo_dir));
            list->addItem(root_item);

            u32 count = 1; // Root
            tsl::hlp::doWithSDCardHandle([&](){
                auto dir = opendir(g_emuiibo_amiibo_dir);
                if(dir) {
                    while(true) {
                        auto entry = readdir(dir);
                        if(entry == nullptr) {
                            break;
                        }
                        char path[FS_MAX_PATH] = {0};
                        sprintf(path, "%s/%s", g_emuiibo_amiibo_dir, entry->d_name);
                        // If it's a valid amiibo, skip
                        emu::VirtualAmiiboData tmp_data;
                        if(R_SUCCEEDED(emu::TryParseVirtualAmiibo(path, FS_MAX_PATH, &tmp_data))) {
                            continue;
                        }
                        if(entry->d_type & DT_DIR) {
                            auto item = new tsl::elm::SmallListItem(entry->d_name);
                            item->setClickListener(std::bind(&CategoryList::OnItemClick, std::placeholders::_1, std::string(path)));
                            list->addItem(item);
                            count++;
                        }
                    }
                    closedir(dir);
                }
            });

            selected_header = new tsl::elm::BigCategoryHeader(MakeActiveAmiiboText(), true);
            count_header = new tsl::elm::CategoryHeader("Available categories (" + std::to_string(count) + ")", true);

            header_list->addItem(selected_header);
            header_list->addItem(count_header);

            root_frame->setHeader(header_list);
            root_frame->setContent(list);
            return root_frame;
        }

        virtual void update() override {
            if(UpdateCurrentApplicationIntercepted()) {
                root_frame->setSubtitle(MakeStatusText());
            }
        }

};

class MainGui : public tsl::Gui {

    private:
        tsl::elm::BigCategoryHeader *amiibo_header;
        tsl::elm::OverlayFrame *root_frame;

    public:
        MainGui() : amiibo_header(new tsl::elm::BigCategoryHeader(MakeActiveAmiiboText(), true)), root_frame(new tsl::elm::OverlayFrame(MakeTitleText(), MakeStatusText())) {}

        virtual tsl::elm::Element *createUI() override {
            auto list = new tsl::elm::List();
            
            if(g_emuiibo_init_ok) {
                auto status = emu::GetEmulationStatus();

                auto *toggle_item = new tsl::elm::NamedStepTrackBar("\u22EF", { "Off", "On" });
                auto *select_item = new tsl::elm::SmallListItem("View amiibo");

                u8 toggle_progress;
                switch(status) {
                    case emu::EmulationStatus::On:
                        toggle_progress = 1;
                        break;
                    case emu::EmulationStatus::Off:
                        toggle_progress = 0;
                        break;
                }
                toggle_item->setProgress(toggle_progress);

                toggle_item->setValueChangedListener([&](u8 progress) {
                    switch(progress) {
                        case 1: {
                            emu::SetEmulationStatus(emu::EmulationStatus::On);
                            root_frame->setSubtitle(MakeStatusText());
                            break;
                        }
                        case 0: {
                            emu::SetEmulationStatus(emu::EmulationStatus::Off);
                            root_frame->setSubtitle(MakeStatusText());
                            break;
                        }
                    }    
                });
                
                select_item->setClickListener([](u64 keys) { 
                    if(keys & KEY_A) {
                        tsl::changeTo<CategoryList>();
                        g_in_second_menu = true;
                        return true;
                    }
                    return false;
                });

                list->addItem(new tsl::elm::BigCategoryHeader("Manage emulation (on / off)", true));
                list->addItem(toggle_item);
                list->addItem(amiibo_header);
                list->addItem(select_item);
            }
            else {
                list->addItem(new tsl::elm::BigCategoryHeader("...", true));
            }

            root_frame->setContent(list);
            return root_frame;
        }

        virtual void update() override {
            if(g_in_second_menu) {
                root_frame->setSubtitle(MakeStatusText());
                g_in_second_menu = false;
            }
            if(UpdateCurrentApplicationIntercepted()) {
                root_frame->setSubtitle(MakeStatusText());
            }
        }

};

class Overlay : public tsl::Overlay {

    public:
        virtual void initServices() override {
            tsl::hlp::doWithSmSession([&] {
                if(emu::IsAvailable()) {
                    g_emuiibo_init_ok = R_SUCCEEDED(emu::Initialize());
                    if(g_emuiibo_init_ok) {
                        g_emuiibo_version = emu::GetVersion();
                        emu::GetVirtualAmiiboDirectory(g_emuiibo_amiibo_dir, FS_MAX_PATH);
                    }
                }
            });
            if(g_emuiibo_init_ok) {
                UpdateActiveAmiibo();
            }
        }
        
        virtual void exitServices() override {
            emu::Exit();
        }
        
        virtual std::unique_ptr<tsl::Gui> loadInitialGui() override {
            return initially<MainGui>();
        }

};

int main(int argc, char **argv) {
    return tsl::loop<Overlay>(argc, argv);
}