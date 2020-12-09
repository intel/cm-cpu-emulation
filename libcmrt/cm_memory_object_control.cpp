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

#include "cm_memory_object_control.h"
#include <string>

CM_SURFACE_MEM_OBJ_CTRL::CM_SURFACE_MEM_OBJ_CTRL()
    : mem_ctrl(MEMORY_OBJECT_CONTROL_SKL_DEFAULT),
      mem_type(CM_USE_PTE),
      age(0) {}
//=============

CM_SURFACE_MEM_OBJ_CTRL::CM_SURFACE_MEM_OBJ_CTRL(
    const CM_SURFACE_MEM_OBJ_CTRL &another): mem_ctrl(another.mem_ctrl),
                                             mem_type(another.mem_type),
                                             age(another.age) {}
//==============================================================

CM_SURFACE_MEM_OBJ_CTRL& CM_SURFACE_MEM_OBJ_CTRL::operator=(
    const CM_SURFACE_MEM_OBJ_CTRL& another) {
  if (this == &another)  return *this;
  mem_ctrl = another.mem_ctrl;
  mem_type = another.mem_type;
  age = another.age;
  return *this;
}//============

namespace mocs {
using std::ostream;
using std::string;
using std::endl;

static bool GenerateSettingSkl(MEMORY_OBJECT_CONTROL option,
                               CmEmuPlatformUse current_platform,
                               ostream &gsf_stream) {
  string l3_control;
  switch (option) {
    case MEMORY_OBJECT_CONTROL_SKL_NO_L3:
    case MEMORY_OBJECT_CONTROL_SKL_NO_CACHE:
    case MEMORY_OBJECT_CONTROL_SKL_NO_LLC_L3:
    case MEMORY_OBJECT_CONTROL_SKL_NO_ELLC_L3:
      l3_control.assign("uncacheable");
      break;

    case MEMORY_OBJECT_CONTROL_SKL_NO_LLC_ELLC:
    case MEMORY_OBJECT_CONTROL_SKL_NO_LLC:
    case MEMORY_OBJECT_CONTROL_SKL_NO_ELLC:
    case MEMORY_OBJECT_CONTROL_SKL_DEFAULT:
    default:
      l3_control.assign("writeback");
      break;
  }

  string llc_cacheability_control, llc_cache_inclusion;
  switch (option) {
    case MEMORY_OBJECT_CONTROL_SKL_NO_CACHE:
    case MEMORY_OBJECT_CONTROL_SKL_NO_LLC_ELLC:
    case MEMORY_OBJECT_CONTROL_SKL_NO_LLC_L3:
    case MEMORY_OBJECT_CONTROL_SKL_NO_LLC:
      llc_cacheability_control.assign("uncacheable");
      break;

    case MEMORY_OBJECT_CONTROL_SKL_NO_ELLC_L3:
    case MEMORY_OBJECT_CONTROL_SKL_NO_ELLC:
      llc_cacheability_control.assign("writeback");
      llc_cache_inclusion.assign("llc");
      break;

    case MEMORY_OBJECT_CONTROL_SKL_NO_L3:
      llc_cacheability_control.assign("writeback");;
      llc_cache_inclusion.assign("llcellc");
      break;

    case MEMORY_OBJECT_CONTROL_SKL_DEFAULT:
    default:
        llc_cacheability_control.assign("writeback");
      llc_cache_inclusion.assign("llcellc");
      break;
  }

  gsf_stream << "  :memoryObjectControl => MemoryObjectControl.new(\n";
  gsf_stream << "    :l3Control => {:cacheabilityControl => \""
             << l3_control << "\"},\n";
  gsf_stream << "    :llcControl => {:cacheabilityControl => \""
                << llc_cacheability_control << "\",\n";
  if (!llc_cache_inclusion.empty()) {
    gsf_stream << "                    :cacheInclusion => \""
               << llc_cache_inclusion << "\",\n";
  }
  gsf_stream << "                    :age => 3}\n";
  gsf_stream << "  )," << endl;
  return gsf_stream.good();
}//========================

//========================

void EnableSurfaceL3Cache(char *surface_name,
                          std::ostream &gsf_stream)
{
    gsf_stream << surface_name << ".memory_object_control = MemoryObjectControl.new(\n";
    gsf_stream << "    :l3Control => {\n";
    gsf_stream << "        :cacheabilityControl => \"writeback\", \n";
    gsf_stream << "        :skipCachingEnable   => false, \n";
    gsf_stream << "        :globalGOEnable => false \n";
    gsf_stream << "    }\n";
    gsf_stream << ")\n";

    return;
}

}
