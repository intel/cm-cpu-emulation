/*===================== begin_copyright_notice ==================================

 Copyright (c) 2020, Intel Corporation


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

class CmDevice;
//class CmTask_RT;

class CmThreadGroupSpace
{
public:
    static int32_t Create( CmDevice* pDevice, uint32_t thrdSpaceWidth, uint32_t thrdSpaceHeight, uint32_t grpSpaceWidth, uint32_t grpSpaceHeight, CmThreadGroupSpace* & pTGS);
    static int32_t Create(CmDevice *pDevice, uint32_t thrdSpaceWidth, uint32_t thrdSpaceHeight, uint32_t thrdSpaceDepth, uint32_t grpSpaceWidth, uint32_t grpSpaceHeight, uint32_t grpSpaceDepth, CmThreadGroupSpace *&pTGS);
    static int32_t Destroy( CmThreadGroupSpace* &pTGS );

    int32_t GetThreadGroupSpaceSize(uint32_t & threadSpaceWidth, uint32_t & threadSpaceHeight, uint32_t & groupSpaceWidth, uint32_t & groupSpaceHeight);
    int32_t GetThreadGroupSpaceSize(uint32_t &threadSpaceWidth, uint32_t &threadSpaceHeight, uint32_t &threadSpaceDepth, uint32_t &groupSpaceWidth, uint32_t &groupSpaceHeight, uint32_t &groupSpaceDepth);

protected:
    CmThreadGroupSpace(CmDevice *pCmDev, uint32_t thrdSpaceWidth, uint32_t thrdSpaceHeight, uint32_t thrdSpaceDepth, uint32_t grpSpaceWidth, uint32_t grpSpaceHeight, uint32_t grpSpaceDepth);
    ~CmThreadGroupSpace( void );

    int32_t Initialize( void );

    CmDevice* m_pCmDev;
    uint32_t m_threadSpaceWidth;
    uint32_t m_threadSpaceHeight;
    uint32_t m_threadSpaceDepth;
    uint32_t m_groupSpaceWidth;
    uint32_t m_groupSpaceHeight;
    uint32_t m_groupSpaceDepth;
};
