#include <tr/tr_Translation.hpp>
#include <tesla.hpp>
#include <unordered_map>
#include <fstream>
#include <tr/json.hpp>

namespace tr {

    namespace {

        constexpr auto DefaultNotFoundString = "???";

        std::unordered_map<std::string, std::string> g_LanguageStrings;

        bool GetSystemLanguage(std::string &out_lang) {
            if(R_SUCCEEDED(setInitialize())) {
                u64 lang_code = 0;
                if(R_SUCCEEDED(setGetSystemLanguage(&lang_code))) {
                    out_lang.assign(reinterpret_cast<const char*>(&lang_code));
                    return true;
                }
                setExit();
            }

            return false;
        }

        inline std::string MakeLanguageFilePath(const std::string &lang) {
            return "sdmc:/emuiibo/overlay/lang/" + lang + ".json";
        }

        bool LoadLanguage(const std::string &lang) {
            g_LanguageStrings.clear();
            auto ok = false;
            tsl::hlp::doWithSDCardHandle([&] {
                try {
                    const auto lang_path = MakeLanguageFilePath(lang);
                    std::ifstream ifs(lang_path);
                    const auto lang_json = nlohmann::json::parse(ifs);

                    if(lang_json.count("strings")) {
                        for(const auto &str: lang_json["strings"]) {
                            if(str.count("key") && str.count("value")) {
                                const auto &key = str["key"].get<std::string>();
                                const auto &value = str["value"].get<std::string>();
                                g_LanguageStrings[key] = value;
                            }
                        }
                    }
                    ok = true;
                }
                catch(std::exception&) {
                    ok = false;
                }
            });
            return ok;
        }

    }

    bool Load() {
        std::string cur_lang;
        if(GetSystemLanguage(cur_lang)) {
            if(LoadLanguage(cur_lang)) {
                return true;
            }
        }

        return false;
    }

    std::string Translate(const std::string &key) {
        if(g_LanguageStrings.count(key)) {
            return g_LanguageStrings.at(key);
        }
        else {
            return DefaultNotFoundString;
        }
    }

}