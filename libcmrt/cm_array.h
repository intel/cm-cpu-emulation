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
