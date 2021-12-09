/*========================== begin_copyright_notice ============================

Copyright (C) 2020 Intel Corporation

SPDX-License-Identifier: MIT

============================= end_copyright_notice ===========================*/

#ifndef CM_INDEX_TYPES_H
#define CM_INDEX_TYPES_H

class SurfaceIndex
{
public:
    /*CM_NOINLINE*/ SurfaceIndex() { m_index = 0; };
    /*CM_NOINLINE*/ SurfaceIndex(const SurfaceIndex& src) { m_index = src.m_index; };
    /*CM_NOINLINE*/ SurfaceIndex(const unsigned int& n) { m_index = n; };
    /*CM_NOINLINE*/ SurfaceIndex& operator = (const unsigned int& n) { this->m_index = n; return *this; };
    /*CM_NOINLINE*/ SurfaceIndex& operator + (const unsigned int& n) { this->m_index += n; return *this; };
    virtual unsigned int get_data(void) { return m_index; };
    virtual ~SurfaceIndex(){};

private:
    unsigned int m_index;
#if defined(CM_EMU) && !defined(_WIN32)
    unsigned char m_paddingForEmuKernelLauncher;
#endif
};

#endif
