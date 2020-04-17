
#pragma once
#include <emu_Types.hpp>
#include <emu_Results.hpp>

namespace ipc::mii {

    // Thanks to Thog and Ryujinx for reversing mii stuff :)

    enum class Age : u32 {
        Young,
        Normal,
        Old,
        All,
    };

    enum class Gender : u32 {
        Male,
        Female,
        All,
    };

    enum class FaceColor : u32 {
        Black,
        White,
        Asian,
        All,
    };

    enum class SourceFlag : u32 {
        Database = BIT(0),
        Default = BIT(1),
        All = BIT(0) | BIT(1),
    };

    enum class SpecialMiiKeyCode : u32 {
        Normal = 0,
        Special = 0xA523B78F,
    };

    Result Initialize();
    void Finalize();

    // For full randomness, age, gender and face color values must be ::All!
    Result BuildRandom(CharInfo *out, Age age, Gender gender, FaceColor face_dolor);
    Result GetCount(u32 *out_count);
    Result GetCharInfo(u32 idx, CharInfo *out_info);
}