/*========================== begin_copyright_notice ============================

Copyright (C) 2021 Intel Corporation

SPDX-License-Identifier: MIT

============================= end_copyright_notice ===========================*/

#pragma once

#include <cstddef>

#include "emu_log.h"
#include "emu_dbgsymb_types.h"

namespace GfxEmu {

struct KernelArg {
public:
    std::string name;
    std::string typeName;
    bool isFloat {false};
    bool isClass {false};
    bool isPointer {false};
    enum class SurfaceKind {
        DATA_PORT_SURF
    };

private:
    std::shared_ptr<void> m_bufPtr {nullptr};
    size_t m_unitCount {0};
    size_t m_unitSize {0};
    size_t m_unitAlignedSize {0};

private:
    size_t roudUpToMaxLign (size_t size) {
        return size % alignof(std::max_align_t) + size;
    }

    bool allocateBuffer (size_t size, size_t count) {
        if (m_bufPtr) {
            if (m_unitSize != size || m_unitCount != count) {
                GFX_EMU_ERROR_MESSAGE("Argment buffer of %u elements of size %u already allocated."
                    "Requested to allocate again with count %u of elements of size %u.",
                        m_unitSize, m_unitCount, size, count);
                return false;
            }

            return true;
        }

        GFX_EMU_ASSERT (size && "Requested to allocate buffer of zero size.");
        GFX_EMU_ASSERT_MESSAGE (count, "Requested to allocate buffer of zero count.");

        m_unitCount = count;
        m_unitSize = size;
        m_unitAlignedSize = roudUpToMaxLign (size);
        m_bufPtr.reset (std::calloc(count, m_unitAlignedSize), std::free);
        return true;
    }

public:
    KernelArg () noexcept = default;
    KernelArg (const void* fromPtr, size_t size) {
        if(!setValueFrom (fromPtr, size))
                throw std::runtime_error("GfxEmu::KernelArg: failed to set value on construction.");
    }
    KernelArg (const void* fromPtr, size_t size, size_t tid, size_t count) {
        if(!setValueFrom (fromPtr, size, tid, count))
                throw std::runtime_error("GfxEmu::KernelArg: failed to set per-thread value on construction.");
    }

    bool isSet () const { return m_unitCount; }
    bool isPerThread () const { return m_unitCount > 1; }

    size_t getUnitCount () const { return m_unitCount; }
    size_t getUnitSize () const { return m_unitSize; }
    void reset (size_t newSize = 0, size_t newCount = 1) {
        GfxEmu::DebugMessage(GfxEmu::Log::Flags::fExtraDetail, "GfxEmu::KernelArg: resetting!\n");
        m_bufPtr.reset ();
        m_unitCount = 0;
        m_unitSize = 0;
        m_unitAlignedSize = 0;
        name = "";
        isFloat = false;
        isClass = false;
        isPointer = false;
        if(newSize) {
            allocateBuffer(newSize, newCount);
        }
    }

    // Letting raw in-buffer pointers out for the low-level stuff of libffi.
    void* getBufferPtr (size_t tid = 0) const {
        if (!m_bufPtr)
                return m_bufPtr.get ();

        GFX_EMU_ASSERT (m_unitCount && tid < m_unitCount &&
                "Thread ID must be < than unitCount configured for the argument.");

        return
                static_cast<uint8_t*> (m_bufPtr.get ()) +
                        tid * m_unitAlignedSize;
    }

    bool setValueFrom (
        const void* fromPtr,
        size_t size,
        size_t tid = 0,
        size_t count = 1)
    {
        if (allocateBuffer (size, count)) {
            std::memcpy(getBufferPtr (tid), fromPtr, size);
            return true;
        }

        return false;
    }

    void annotate (const GfxEmu::DbgSymb::SymbDesc& s) {
        isClass = s.isClass;
        isFloat = s.isFloat;
        isPointer = s.isPointer;
        name = s.name;
        typeName = s.typeName;
    }
};

}; // namespace GfxEmu
