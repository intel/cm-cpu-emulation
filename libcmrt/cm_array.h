/*========================== begin_copyright_notice ============================

Copyright (C) 2017 Intel Corporation

SPDX-License-Identifier: MIT

============================= end_copyright_notice ===========================*/

#ifndef CMRTLIB____SHARE_CM_ARRAY_H_
#define CMRTLIB____SHARE_CM_ARRAY_H_
#include "cm_include.h"
#include "cm_mem.h"
class CmDynamicArray
{
public:

    CmDynamicArray( const uint32_t initSize );
    CmDynamicArray();
    ~CmDynamicArray( void );

    void*    GetElement( const uint32_t index ) ;
    bool    SetElement( const uint32_t index, const void* element );

    uint32_t GetSize( void ) const;
    uint32_t GetMaxSize( void ) ;

    void    Delete( void );

    uint32_t GetFirstFreeIndex();
    bool    SetElementIntoFreeSlot(const void* element); //Set the element into the first available slot in the array

    CmDynamicArray& operator= ( const CmDynamicArray &array );

protected:

    void    CreateArray( const uint32_t size );
    void    DeleteArray( void );

    bool    IsValidIndex( const uint32_t index ) ;

    void**   m_pArrayBuffer;

    uint32_t m_UsedSize;
    uint32_t m_ActualSize;

private:

    CmDynamicArray (const CmDynamicArray& other);

};

#endif
