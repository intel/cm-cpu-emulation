/*========================== begin_copyright_notice ============================

Copyright (C) 2017 Intel Corporation

SPDX-License-Identifier: MIT

============================= end_copyright_notice ===========================*/

#include <assert.h>
#include "cm_lib.h"
#include "genx_lib.h"
#include "cm_list.h"
//#include "dyn_helper.cpp"

using namespace std;

const int maxbsize = 255;
bool workarroundForIpoBugOrFeature = false;

cm_list<CmSys::iobuffer> CmSys::iobuffers; /* List of allocated (loaded) genx i/o buffers */

/* This function searches data i/o buffer by its ID */
cm_list<CmSys::iobuffer>::iterator CmSys::search_buffer(int id)
{
    cm_list<CmSys::iobuffer>::iterator it;

    for (it = CmSys::iobuffers.begin(); it != CmSys::iobuffers.end(); ++it) {
        if (it->id == id) {
            break;
        }
    }
    return it;
}

void
CM_unregister_buffer(int buf_id)
{
	DYN_unregister_buffer(buf_id);
	/*
    cm_list<CmSys::iobuffer>::iterator buff_iter = CmSys::search_buffer(buf_id);
	printf("CM_unregister_buffer: Please compile with /Qxcm_emu if you want to run .exe\n");
    if (buff_iter != CmSys::iobuffers.end()) {
        CmSys::iobuffers.remove(buff_iter);
    }
	*/
}
void
CM_register_buffer(int buf_id, CmBufferType bclass, void *src, uint width)
{
    assert((width != 0));
    DYN_register_buffer(buf_id,bclass,src,width,1,R8G8B8A8_UINT,1,0, OWORD);
}
void
CM_register_buffer(int buf_id, CmBufferType bclass, void *src, uint width, uint height,
                   CmSurfaceFormatID surfFormat, uint depth, uint pitch)
{
    assert((width != 0) && (height != 0));
    assert((buf_id < maxbsize) && (CmSys::iobuffers.size() < maxbsize));
    DYN_register_buffer(buf_id,bclass,src,width,height,surfFormat,depth,pitch, MEDIA);
	/*
    // The following implementation is obsolete, which only supports
    // Media word read/write in EXO mode. The new interfaces for
    // OWord read/write and sampler are not supported.

    assert((width != 0) && (height != 0));

    assert((buf_id < maxbsize) && (CmSys::iobuffers.size() < maxbsize));
    void *bdesc = (void*)0;
    CM_unregister_buffer(buf_id);

    CmSys::iobuffer new_buff;

    new_buff.id = buf_id;
    new_buff.buf_desc = bdesc;
    CmSys::iobuffers.add(new_buff);
	*/
}

void
CM_modify_buffer(int buf_id, CmBufferDescField field, int value)
{
    cm_list<CmSys::iobuffer>::iterator buff_iter = CmSys::search_buffer(buf_id);
    if (buff_iter != CmSys::iobuffers.end()) {
        printf("Please compile with /Qxcm_emu if you want to run .exe\n");
    }
}

void
CM_unregister_buffer(SurfaceIndex buf_id)
{
    DYN_unregister_buffer(buf_id.get_data());
	/*
    cm_list<CmSys::iobuffer>::iterator buff_iter = CmSys::search_buffer(buf_id);
	printf("CM_unregister_buffer: Please compile with /Qxcm_emu if you want to run .exe\n");
    if (buff_iter != CmSys::iobuffers.end()) {
        CmSys::iobuffers.remove(buff_iter);
    }
	*/
}
void
CM_register_buffer(SurfaceIndex buf_id, CmBufferType bclass, void *src, uint width)
{
    assert((width != 0));
    DYN_register_buffer(buf_id.get_data(),bclass,src,width,1,R8G8B8A8_UINT,1,0, OWORD);
}
void
CM_register_buffer(SurfaceIndex buf_id, CmBufferType bclass, void *src, uint width, uint height,
                   CmSurfaceFormatID surfFormat, uint depth, uint pitch)
{
    assert((width != 0) && (height != 0));
    assert((buf_id.get_data() < maxbsize) && (CmSys::iobuffers.size() < maxbsize));
    DYN_register_buffer(buf_id.get_data(),bclass,src,width,height,surfFormat,depth,pitch, MEDIA);
	/*
    // The following implementation is obsolete, which only supports
    // Media word read/write in EXO mode. The new interfaces for
    // OWord read/write and sampler are not supported.

    assert((width != 0) && (height != 0));

    assert((buf_id < maxbsize) && (CmSys::iobuffers.size() < maxbsize));
    void *bdesc = (void*)0;
    CM_unregister_buffer(buf_id);

    CmSys::iobuffer new_buff;

    new_buff.id = buf_id;
    new_buff.buf_desc = bdesc;
    CmSys::iobuffers.add(new_buff);
	*/
}

void CM_modify_buffer(SurfaceIndex buf_id, CmBufferDescField field, int value)
{
    cm_list<CmSys::iobuffer>::iterator buff_iter = CmSys::search_buffer(buf_id.get_data());
    if (buff_iter != CmSys::iobuffers.end()) {
        printf("Please compile with /Qxcm_emu if you want to run .exe\n");
    }
}
