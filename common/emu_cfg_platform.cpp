/*========================== begin_copyright_notice ============================

Copyright (C) 2021 Intel Corporation

SPDX-License-Identifier: MIT

============================= end_copyright_notice ===========================*/

#include <algorithm>

#include "emu_cfg_platform.h"
#include "emu_log.h"

namespace GfxEmu {
namespace Cfg {

bool PlatformConfig::isValidSku(GfxEmu::Platform::Sku::Id id) const {
    return std::any_of(
               skuVariants.begin(),
               skuVariants.end (),
               [=] (auto a) { return a == id; });
}

GfxEmu::Platform::Sku::Id PlatformConfig::getValidSkuOrDefault(GfxEmu::Platform::Sku::Id id) const {
    return isValidSku(id) ? id : defaultSku;
}

size_t PlatformConfig::getMaxThreads(GfxEmu::Platform::Sku::Id id) const {
    return maxThreads.at(id);
}

size_t PlatformConfig::getThreadsPerEu(GfxEmu::Platform::Sku::Id id) const {
    return threadsPerEu.at(id);
}

size_t PlatformConfig::getEuPerSubslice(GfxEmu::Platform::Sku::Id id) const {
    return euPerSubslice.at(id);
}

size_t PlatformConfig::getMaxCcsIndex(GfxEmu::Platform::Sku::Id id) const {
    return maxCcsIndex.at(id);
}

GfxEmu::Platform::Sku::Id PlatformConfig::getDefaultSku() const {
    return defaultSku;
}

const PlatformConfig& getPlatformConfig(GfxEmu::Platform::Id id) {
    static const PlatformConfigMap platformConfigMap = {
    {GfxEmu::Platform::SKL, {
        /*.name =*/ "SKL",
        /*.archid =*/ 900,
        /*.hwSimd =*/ 8,
        /*.flags =*/ PlatformFlags::Fp64 | PlatformFlags::IeeeDivSqrt,
        /*.skuVariants =*/ {GT1,GT2,GT3,GT4},
        /*.defaultSku =*/ GT2,
        /*.maxThreads =*/    {{GT1, 98},{GT2,161},{GT3,329},{GT4,497}},
        /*.threadsPerEu =*/  {{GT1,  7},{GT2,  7},{GT3,  7},{GT4, 7}},
        /*.euPerSubslice =*/ {{GT1,  8},{GT2,  8},{GT3,  8},{GT4, 8}},
    }},
    {GfxEmu::Platform::BXT, {
        /*.name =*/ "BXT",
        /*.archid =*/ 920,
        /*.hwSimd =*/ 8,
        /*.flags =*/ PlatformFlags::None,
        /*.skuVariants =*/ {GTA,GTC,GTX},
        /*.defaultSku =*/ GTX,
        /*.maxThreads =*/    {{GTA, 108},{GTC,72},{GTX,144}},
        /*.threadsPerEu =*/  {{GTA,  6},{GTC,  6},{GTX,  6}},
        /*.euPerSubslice =*/ {{GTA,  6},{GTC,  6},{GTX,  8}},
    }},
    {GfxEmu::Platform::KBL, {
        /*.name =*/ "KBL",
        /*.archid =*/ 950,
        /*.hwSimd =*/ 8,
        /*.flags =*/ PlatformFlags::Fp64 | PlatformFlags::IeeeDivSqrt,
        /*.skuVariants =*/ {GT1,GT2,GT3/*,GT4*/},
        /*.defaultSku =*/ GT2,
        /*.maxThreads =*/    {{GT1, -1},{GT2,161},{GT3,329}},
        /*.threadsPerEu =*/  {{GT1, -1},{GT2,  7},{GT3,  7}},
        /*.euPerSubslice =*/ {{GT1, -1},{GT2,  8},{GT3,  8}},
    }},
    {GfxEmu::Platform::ICLLP, {
        /*.name =*/ "ICLLP",
        /*.archid =*/ 1150,
        /*.hwSimd =*/ 8,
        /*.flags =*/ PlatformFlags::None,
        /*.skuVariants =*/ {GT1,GT2},
        /*.defaultSku =*/ GT2,
        /*.maxThreads =*/    {{GT1, 224},{GT2,448}},
        /*.threadsPerEu =*/  {{GT1, 7},  {GT2,  7}},
        /*.euPerSubslice =*/ {{GT1, 8},  {GT2,  8}},
    }},
    {GfxEmu::Platform::TGLLP, {
        /*.name =*/ "TGLLP",
        /*.archid =*/ 1200,
        /*.hwSimd =*/ 8,
        /*.flags =*/ PlatformFlags::Dp4a,
        /*.skuVariants =*/ {GT1,GT2},
        /*.defaultSku =*/ GT2,
        /*.maxThreads =*/    {{GT1, 224},{GT2,672}},
        /*.threadsPerEu =*/  {{GT1, 7},  {GT2,  7}},
        /*.euPerSubslice =*/ {{GT1, 16}, {GT2,  16}},
    }},
    {GfxEmu::Platform::XEHP_SDV, {
        /*.name =*/ "XEHP_SDV",
        /*.archid =*/ 1270,
        /*.hwSimd =*/ 8,
        /*.flags =*/ PlatformFlags::Fp64 | PlatformFlags::IeeeDivSqrt | PlatformFlags::Dp4a,
        /*.skuVariants =*/ {GT1,GT2,GT3},
        /*.defaultSku =*/ GT2,
        /*.maxThreads =*/    {{GT1, 1024},{GT2,2048},{GT3,4096}},
        /*.threadsPerEu =*/  {{GT1, 8},   {GT2,  8}, {GT3,  8}},
        /*.euPerSubslice =*/ {{GT1, 16},  {GT2,  16},{GT3,  16}},
        /*.maxCcsIndex =*/   {{GT1, 1},   {GT2,2},   {GT3,4}},
    }},
    };

    try {
        return platformConfigMap.at(id);
    } catch(...) {
        GFX_EMU_FAIL_WITH_MESSAGE(fCfg, "platform %u is not supported.\n", id);
    }
}

}; // namespace Cfg
}; // namespace GfxEmu
