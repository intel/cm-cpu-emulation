/*========================== begin_copyright_notice ============================

Copyright (C) 2021 Intel Corporation

SPDX-License-Identifier: MIT

============================= end_copyright_notice ===========================*/

#pragma once
#include <map>
#include <string>

namespace GfxEmu {
namespace Platform {
#define MAP(name, enumName) {#name, enumName :: name}

enum Id {
    UNDEFINED = -1,
    BDW = 40,
    SKL = 50,
    BXT = 51,
    KBL = 53,
    ICLLP = 71,
    XEHP_SDV = 80,
    TGLLP = 81,
    DG1 = 82,
    PVC = 83,
    DG2 = 84,
    MTL = 87,
    RKL = 90,
    ADLN = 91,
    ADLP = 92,
    ADLS = 93
};

inline std::map<std::string, int64_t>& StaticData_nameToInt() {
    static std::map<std::string, int64_t> nameToInt = {{
        MAP(BDW, GfxEmu::Platform),
        MAP(SKL, GfxEmu::Platform),
        MAP(BXT, GfxEmu::Platform),
        MAP(KBL, GfxEmu::Platform),
        MAP(ICLLP, GfxEmu::Platform),
        MAP(TGLLP, GfxEmu::Platform),
        MAP(XEHP_SDV, GfxEmu::Platform),
        MAP(DG1, GfxEmu::Platform),
        MAP(PVC, GfxEmu::Platform),
        MAP(DG2, GfxEmu::Platform),
        MAP(MTL, GfxEmu::Platform),
        MAP(RKL, GfxEmu::Platform),
        MAP(ADLN, GfxEmu::Platform),
        MAP(ADLP, GfxEmu::Platform),
        MAP(ADLS, GfxEmu::Platform)
    }};
    return nameToInt;
};

namespace Sku {
enum Id {
  DEFAULT = -1,
  UNDEFINED = 0,
  GT1 = 1,
  GT2 = 2,
  GT3 = 3,
  GT4 = 4,
  GT5 = 5,
  GTA = 5,
  GTC = 6,
  GTX = 7,
  GT1_5 = 8
};

inline std::map<std::string, int64_t>& StaticData_nameToInt() {
    static std::map<std::string, int64_t> nameToInt = {{
        MAP(DEFAULT, GfxEmu::Platform::Sku),
        MAP(GT1, GfxEmu::Platform::Sku),
        MAP(GT2, GfxEmu::Platform::Sku),
        MAP(GT3, GfxEmu::Platform::Sku),
        MAP(GT4, GfxEmu::Platform::Sku),
        MAP(GT5, GfxEmu::Platform::Sku),
        MAP(GTA, GfxEmu::Platform::Sku),
        MAP(GTC, GfxEmu::Platform::Sku),
        MAP(GTX, GfxEmu::Platform::Sku),
        MAP(GT1_5, GfxEmu::Platform::Sku)
    }};
    return nameToInt;
};

}; // namespace Sku

#undef MAP

}; // namespace Platform
}; // namespace GfxEmu
