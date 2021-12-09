/*========================== begin_copyright_notice ============================

Copyright (C) 2021 Intel Corporation

SPDX-License-Identifier: MIT

============================= end_copyright_notice ===========================*/

#pragma once

#include <map>
#include <vector>

#include "emu_platform.h"
#include "emu_api_export.h"

namespace GfxEmu {
namespace Cfg {

using namespace GfxEmu::Platform::Sku;

enum PlatformFlags {
  None = 0,
  Fp64 = 1,
  IeeeDivSqrt = 2,
  Dp4a = 4,
};

struct PlatformConfig {
    using PerSkuMap = std::map<typename GfxEmu::Platform::Sku::Id,size_t>;

    std::string name;

    unsigned archid;
    unsigned hwSimd;
    unsigned flags;

    std::vector<typename GfxEmu::Platform::Sku::Id> skuVariants;
    GfxEmu::Platform::Sku::Id defaultSku;

    PerSkuMap maxThreads;
    PerSkuMap threadsPerEu;
    PerSkuMap euPerSubslice;
    PerSkuMap maxCcsIndex;

    GFX_EMU_API size_t getMaxThreads(GfxEmu::Platform::Sku::Id) const;
    GFX_EMU_API size_t getThreadsPerEu(GfxEmu::Platform::Sku::Id) const;
    GFX_EMU_API size_t getEuPerSubslice(GfxEmu::Platform::Sku::Id) const;
    GFX_EMU_API size_t getMaxCcsIndex(GfxEmu::Platform::Sku::Id) const;
    GFX_EMU_API GfxEmu::Platform::Sku::Id getDefaultSku() const;
    GFX_EMU_API bool isValidSku(GfxEmu::Platform::Sku::Id id) const;
    GFX_EMU_API GfxEmu::Platform::Sku::Id getValidSkuOrDefault(GfxEmu::Platform::Sku::Id id) const;
};

using PlatformConfigMap = std::map<GfxEmu::Platform::Id,PlatformConfig>;
GFX_EMU_API const PlatformConfig& getPlatformConfig(GfxEmu::Platform::Id id);

}; // namespace Cfg
}; // namespace GfxEmu
