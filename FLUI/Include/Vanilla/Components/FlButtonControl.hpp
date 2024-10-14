#pragma once

#include "Structures.hpp"

class FlButtonControl final : public FlControl
{
//        byte data[888]{}; // 0x378 in sub_55D2B0
public:
        byte data[0xCC]{};
        char iconFile[100]{};
        byte data2[0x248]{};
private:
        FlButtonControl() = default;
};
