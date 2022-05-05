/*========================== begin_copyright_notice ============================

Copyright (C) 2017 Intel Corporation

SPDX-License-Identifier: MIT

============================= end_copyright_notice ===========================*/

#ifndef CMRTLIB____SHARE_SURFACE_ALIAS_TABLE_H_
#define CMRTLIB____SHARE_SURFACE_ALIAS_TABLE_H_

#include <map>
#include <vector>
#include "cm_include.h"
#include "emu_log.h"
#include "cm_index_types.h"
#include "type_buffer_state_param.h"
#include "type_surface_2d_state_param.h"

namespace cmrt {
//!
//! Array of buffer states for each alias.
//!
template<typename T>
class SurfaceAliasTable {
 public:
  static const unsigned int MAX_SURFACE_ALIAS_COUNT = 10;

  SurfaceAliasTable() {
      active_alias_index_ = 0;
  }
  //====================

  ~SurfaceAliasTable() {
    for (std::vector<SurfaceIndex*>::iterator iter = alias_indices_.begin();
         iter != alias_indices_.end();
         ++iter) {
      delete *iter;
    }
    return;
  }//======

  SurfaceIndex* CreateAlias(unsigned int original_index,
                            unsigned int surface_array_size) {
    if (alias_indices_.size() >= MAX_SURFACE_ALIAS_COUNT) {
      GFX_EMU_ASSERT(0);
      return nullptr;
    }
    unsigned int new_idx = original_index
        + (static_cast<unsigned int>(alias_indices_.size()) + 1)
        *surface_array_size;
    alias_indices_.push_back(new SurfaceIndex(new_idx));
    return *(alias_indices_.rbegin());
  }//=================================

  bool AddSurfaceState(unsigned int surface_index,
                       const T &new_state) {
    std::pair<unsigned, T> new_element(surface_index, new_state);
    surface_states_.insert(new_element);
    return true;
  }//===========

  const T* GetSurfaceState(unsigned int surface_index) {
    typename std::map<unsigned int, T>::iterator iter
        = surface_states_.find(surface_index);
    if (surface_states_.end() == iter) {
      return nullptr;
    } else {
      active_alias_index_ = surface_index;
      return &(iter->second);
    }
  }//========================

  unsigned int ActiveAliasIndex() const {
    return active_alias_index_;
  }//==========================

 private:
  std::map<unsigned int, T> surface_states_;

  std::vector<SurfaceIndex*> alias_indices_;

  unsigned int active_alias_index_;
};

typedef ::CM_BUFFER_STATE_PARAM BufferState;
typedef SurfaceAliasTable<BufferState> BufferAliasTable;
typedef ::CM_SURFACE2D_STATE_PARAM Surface2DState;
typedef SurfaceAliasTable<Surface2DState> Surface2DAliasTable;

}
#endif  // #ifndef CMRTLIB____SHARE_SURFACE_ALIAS_TABLE_H_
