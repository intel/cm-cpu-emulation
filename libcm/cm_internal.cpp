/*========================== begin_copyright_notice ============================

Copyright (C) 2017 Intel Corporation

SPDX-License-Identifier: MIT

============================= end_copyright_notice ===========================*/

#define CM_EMU

#include <assert.h>
#include <stdlib.h>
#include "cm_lib.h"
#include "cm_internal_emu.h"
#include "emu_log.h"

namespace __CMInternal__ {

thread_local bool outerloop_simd_size_determined = false;
thread_local volatile uint __cm_internal_simd_marker = 0;
thread_local Stack* workingStack = nullptr;
thread_local Stack* breakStack = nullptr;

template <typename T>
inline
T *pop(Stack *s)
{
    T *top = static_cast<T *>(s->pop());
    if (top){
        return top;
    }
    GFX_EMU_ERROR_MESSAGE("stack empty.");
    exit(EXIT_FAILURE);
}

CM_API Stack* getWorkingStack()
{
	return workingStack;
}

CM_API void setWorkingStack(Stack *s)
{
	workingStack = s;
	return;
}

CM_API Stack* getBreakStack()
{
	return breakStack;
}

CM_API void setBreakStack(Stack *s)
{
	breakStack = s;
	return;
}

CM_API uint getSIMDMarker()
{
	return __cm_internal_simd_marker;
}

CM_API void setSIMDMarker(uint marker)
{
	__cm_internal_simd_marker = marker;
	return;
}

CM_API uint __cm_internal_simd()
{
    return uint(__cm_internal_simd_marker);
}

CM_API uint __cm_internal_simd_then_end()
{
    maskItem *it = pop<maskItem>(workingStack);
    uint simd_mask = ~(it->getExecutedMask());

    it->setMask(simd_mask);
    if (!workingStack->isEmpty()) {
        simd_mask =(( maskItem *)workingStack->top())->getMask();
        it->setMask(it->getMask() & simd_mask);
    }

    workingStack->push(it);

    return it->getMask();
}

CM_API uint __cm_internal_simd_else_begin()
{
    return (( maskItem *)workingStack->top())->getMask();
}

CM_API uint __cm_internal_simd_if_end()
{
    workingStack->pop();

    if (!workingStack->isEmpty()) {
        return (( maskItem *)workingStack->top())->getMask();
    } else {
        return 0;
    }
}

CM_API uint __cm_internal_simd_if_join()
{
    if (!workingStack->isEmpty()) {
        return (( maskItem *)workingStack->top())->getMask();
    } else {
        return 0;
    }
}

CM_API void __cm_internal_simd_do_while_before()
{
     maskItem* mi_1;
     maskItem* mi_2;
    uint m = 0xffffffff;
    const type_info* t = &typeid(uint);

    if (!workingStack)
        workingStack = new Stack();
    if (!breakStack)
        breakStack = new Stack();

    //Initialize the value of the masks
    if (!workingStack->isEmpty())
        m &= (( maskItem *)workingStack->top())->getMask();
    mi_1 = new breakMaskItem(m, t, MAX_MASK_WIDTH);
    mi_2 = new maskItem(m, t, MAX_MASK_WIDTH);

    breakStack->push((void *) mi_1);
    workingStack->push((void *) mi_2);

    (( breakMaskItem *)breakStack->top())->setWorkingDepth(workingStack->getDepth());

    outerloop_simd_size_determined = false;

    return;
}

CM_API uint __cm_internal_simd_do_while_begin()
{
    if (!outerloop_simd_size_determined)
        outerloop_simd_size_determined = true;
    return ((maskItem *)breakStack->top())->getMask();
}

CM_API uint __cm_internal_before_do_while_end()
{
    return ((maskItem *)breakStack->top())->getMask();
}

CM_API uint __cm_internal_simd_after_do_while_end()
{
    breakStack->pop();
    workingStack->pop();

    outerloop_simd_size_determined = true;
    if (!workingStack->isEmpty())
        return ((maskItem *)workingStack->top())->getMask();
    else
        return 0xffffffff;
}

CM_API uint __cm_internal_simd_break()
{
    Stack *tmp_s;
    maskItem * tmp_mask;
    int depth = workingStack->getDepth() - (( breakMaskItem *)breakStack->top())->getWorkingDepth();
    uint simd_mask = ((maskItem *)workingStack->top())->getMask();

    simd_mask = ~simd_mask;

    ((maskItem *)breakStack->top())->setMask(simd_mask & ((maskItem *)breakStack->top())->getMask());

    tmp_mask = pop<maskItem>(workingStack);
    tmp_mask->setMask(tmp_mask->getMask() & simd_mask);

    tmp_s = new Stack();
    tmp_s->push((void *)tmp_mask);

    while(depth) {
        tmp_mask = pop<maskItem>(workingStack);
        tmp_mask->setMask(tmp_mask->getMask() & simd_mask);
        tmp_s->push((void *)tmp_mask);
        depth --;
    }

    while (!tmp_s->isEmpty()) {
        tmp_mask = (maskItem *)tmp_s->pop();
        workingStack->push((void *)tmp_mask);
    }

    delete tmp_s;

    return ~simd_mask;
}

CM_API uint __cm_internal_simd_continue()
{
    Stack *tmp_s;
    maskItem * tmp_mask;
    int depth = workingStack->getDepth() - (( breakMaskItem *)breakStack->top())->getWorkingDepth();
    uint simd_mask = ((maskItem *)workingStack->top())->getMask();

    simd_mask = ~simd_mask;

    tmp_mask = pop<maskItem>(workingStack);
    tmp_mask->setMask(tmp_mask->getMask() & simd_mask);

    tmp_s = new Stack();
    tmp_s->push((void *)tmp_mask);
    while(depth) {
        tmp_mask = pop<maskItem>(workingStack);
        tmp_mask->setMask(tmp_mask->getMask() & simd_mask);
        tmp_s->push((void *)tmp_mask);
        depth --;
    }
    while (!tmp_s->isEmpty()) {
        tmp_mask = (maskItem *)tmp_s->pop();
        workingStack->push((void *)tmp_mask);
    }

    delete tmp_s;

    return ~simd_mask;
}

#define SIMD_DO_WHILE_END(T) \
	CM_API uint __cm_internal_simd_do_while_end(T cond) \
	{ \
		int width = MAX_MASK_WIDTH; \
		vector<unsigned int, MAX_MASK_WIDTH> v = (unsigned int)cond; \
        unsigned int simd_mask = 0; \
        \
 		assert(sizeof(T) <= MAX_MASK_WIDTH);	\
        for (unsigned int i = 1; i <= sizeof(T); i++) { \
            T e = v.get(i - 1); \
            if (e) \
                simd_mask |= 1 << (MAX_MASK_WIDTH - i); \
        } \
        assert(!getBreakStack()->isEmpty()); \
        	\
        simd_mask &= ((maskItem *)breakStack->top())->getMask(); \
        ((maskItem *)breakStack->top())->setMask(simd_mask); \
        ((maskItem *)workingStack->top())->setMask(simd_mask); \
        return simd_mask; \
	}

SIMD_DO_WHILE_END(bool);
SIMD_DO_WHILE_END(int);
SIMD_DO_WHILE_END(unsigned int);
SIMD_DO_WHILE_END(char);
SIMD_DO_WHILE_END(unsigned char);
SIMD_DO_WHILE_END(short);
SIMD_DO_WHILE_END(unsigned short);
};
