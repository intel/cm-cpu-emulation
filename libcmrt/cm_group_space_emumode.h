/*========================== begin_copyright_notice ============================

Copyright (C) 2017 Intel Corporation

SPDX-License-Identifier: MIT

============================= end_copyright_notice ===========================*/

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
