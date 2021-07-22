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
    unsigned char m_paddingForEmuKernelLauncher;
};

#endif
