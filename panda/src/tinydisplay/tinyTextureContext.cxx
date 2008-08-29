// Filename: tinyTextureContext.cxx
// Created by:  drose (30Apr08)
//
////////////////////////////////////////////////////////////////////
//
// PANDA 3D SOFTWARE
// Copyright (c) Carnegie Mellon University.  All rights reserved.
//
// All use of this software is subject to the terms of the revised BSD
// license.  You should have received a copy of this license along
// with this source code in a file named "LICENSE."
//
////////////////////////////////////////////////////////////////////

#include "tinyTextureContext.h"
#include "zgl.h"

TypeHandle TinyTextureContext::_type_handle;

////////////////////////////////////////////////////////////////////
//     Function: TinyTextureContext::evict_lru
//       Access: Public, Virtual
//  Description: Evicts the page from the LRU.  Called internally when
//               the LRU determines that it is full.  May also be
//               called externally when necessary to explicitly evict
//               the page.
//
//               It is legal for this method to either evict the page
//               as requested, do nothing (in which case the eviction
//               will be requested again at the next epoch), or
//               requeue itself on the tail of the queue (in which
//               case the eviction will be requested again much
//               later).
////////////////////////////////////////////////////////////////////
void TinyTextureContext::
evict_lru() {
  dequeue_lru();

  GLTexture *gltex = &_gltex;
  if (gltex->allocated_buffer != NULL) {
    nassertv(gltex->num_levels != 0);
    TinyTextureContext::get_class_type().dec_memory_usage(TypeHandle::MC_array, gltex->total_bytecount);
    PANDA_FREE_ARRAY(gltex->allocated_buffer);
    gltex->allocated_buffer = NULL;
    gltex->total_bytecount = 0;
    gltex->num_levels = 0;
  } else {
    nassertv(gltex->num_levels == 0);
  }

  update_data_size_bytes(0);
  mark_unloaded();
}
