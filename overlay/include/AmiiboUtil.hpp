
#pragma once
#include <vector>
#include <string>
#include <json.hpp>

namespace emuovl {

    using JSON = nlohmann::json;

    struct VirtualAmiibo {

        std::string path;
        std::string amiibo_name;

        inline bool IsValid() {
            return !this->path.empty() && !this->amiibo_name.empty();
        }

    };

    VirtualAmiibo ProcessAmiibo(const std::string &path);
    std::vector<VirtualAmiibo> ListAmiibos();

}