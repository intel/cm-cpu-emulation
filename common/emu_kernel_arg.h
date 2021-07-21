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
                GfxEmu::ErrorMessage ("Argment buffer of %u elements of size %u already allocated."
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
    void reset () {
        GfxEmu::DebugMessage(GfxEmu::Log::Flags::fExtraDetail, "GfxEmu::KernelArg: resetting!\n");
        m_bufPtr.reset ();
        m_unitCount = 0;
        m_unitSize = 0;
        m_unitAlignedSize = 0;
        name = "";
        isFloat = false;
        isClass = false;
        isPointer = false;
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
        size_t size)
    {
        if (allocateBuffer (size, 1)) {
            std::memcpy(getBufferPtr (), fromPtr, size);
            return true;
        }

        return false;
    }

    bool setValueFrom (
        const void* fromPtr,
        size_t size,
        size_t tid,
        size_t count)
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

