/*========================== begin_copyright_notice ============================

Copyright (C) 2017 Intel Corporation

SPDX-License-Identifier: MIT

============================= end_copyright_notice ===========================*/

#include "emu_dbgsymb.h"

#ifdef GFX_EMU_DEBUG_SYMBOLS_ACCESS_ENABLED
namespace GfxEmu {
namespace DbgSymb {

DbgSymbIface& obj() {
#ifndef _WIN32
    thread_local
#endif
    static DbgSymbIface instance_;
    return instance_;
} ;

}; // namespace DbgSymb
}; // namespace GfxEmu
#endif // GFX_EMU_DEBUG_SYMBOLS_ACCESS_ENABLED
