// Filename: tinyTextureContext.cxx
// Created by:  drose (30Apr08)
//
////////////////////////////////////////////////////////////////////
//
// PANDA 3D SOFTWARE
// Copyright (c) 2001 - 2004, Disney Enterprises, Inc.  All rights reserved
//
// All use of this software is subject to the terms of the Panda 3d
// Software license.  You should have received a copy of this license
// along with this source code; you will also find a current copy of
// the license at http://etc.cmu.edu/panda3d/docs/license/ .
//
// To contact the maintainers of this program write to
// panda3d-general@lists.sourceforge.net .
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

  for (int i = 0; i < _gltex->num_levels; ++i) {
    gl_free(_gltex->levels[i].pixmap);
  }
  if (_gltex->levels != NULL) {
    gl_free(_gltex->levels);
    _gltex->levels = NULL;
  }
  _gltex->num_levels = 0;

  set_resident(false);
}
