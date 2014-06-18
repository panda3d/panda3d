// Filename: glTextureContext.cxx
// Created by:  drose (07Oct99)
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

#include "pnotify.h"

TypeHandle CLP(TextureContext)::_type_handle;


////////////////////////////////////////////////////////////////////
//     Function: GLTextureContext::evict_lru
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
void CLP(TextureContext)::
evict_lru() {
  dequeue_lru();
  reset_data();
  update_data_size_bytes(0);
  mark_unloaded();
}

////////////////////////////////////////////////////////////////////
//     Function: GLTextureContext::reset_data
//       Access: Public
//  Description: Resets the texture object to a new one so a new GL
//               texture object can be uploaded.
////////////////////////////////////////////////////////////////////
void CLP(TextureContext)::
reset_data() {
  // Free the texture resources.
  glDeleteTextures(1, &_index);

  // We still need a valid index number, though, in case we want to
  // re-load the texture later.
  glGenTextures(1, &_index);

  _already_applied = false;
}
