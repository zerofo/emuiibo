
#pragma once
#include <ipc/mii/mii_Service.hpp>

namespace ipc::mii {

    void DumpSystemMiis();
    
    inline CharInfo GenerateRandomMii() {
        CharInfo charinfo = {};
        BuildRandom(&charinfo, Age::All, Gender::All, Race::All);
        return charinfo;
    }

}