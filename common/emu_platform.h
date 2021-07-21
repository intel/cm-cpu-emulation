/*===================== begin_copyright_notice ==================================

 Copyright (c) 2021, Intel Corporation


 Permission is hereby granted, free of charge, to any person obtaining a
 copy of this software and associated documentation files (the "Software"),
 to deal in the Software without restriction, including without limitation
 the rights to use, copy, modify, merge, publish, distribute, sublicense,
 and/or sell copies of the Software, and to permit persons to whom the
 Software is furnished to do so, subject to the following conditions:

 The above copyright notice and this permission notice shall be included
 in all copies or substantial portions of the Software.

 THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS
 OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL
 THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR
 OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE,
 ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR
 OTHER DEALINGS IN THE SOFTWARE.
======================= end_copyright_notice ==================================*/

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
    TGLLP = 81

};

inline auto StaticData_nameToInt = []()->auto& {
    static  std::map<std::string, int64_t> nameToInt = {{
        MAP(BDW, GfxEmu::Platform),
        MAP(SKL, GfxEmu::Platform),
        MAP(BXT, GfxEmu::Platform),
        MAP(KBL, GfxEmu::Platform),
        MAP(ICLLP, GfxEmu::Platform),
        MAP(TGLLP, GfxEmu::Platform),
        MAP(XEHP_SDV, GfxEmu::Platform),
    }};
    return nameToInt;
};

namespace Sku {
enum Id {
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

inline auto StaticData_nameToInt = []()->auto& {
    static std::map<std::string, int64_t> nameToInt = {{
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

