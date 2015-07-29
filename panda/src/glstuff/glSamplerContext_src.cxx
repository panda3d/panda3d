// Filename: glSamplerContext_src.cxx
// Created by:  rdb (11Dec14)
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

#ifndef OPENGLES

TypeHandle CLP(SamplerContext)::_type_handle;

////////////////////////////////////////////////////////////////////
//     Function: CLP(SamplerContext)::Constructor
//       Access: Public
//  Description:
////////////////////////////////////////////////////////////////////
INLINE CLP(SamplerContext)::
CLP(SamplerContext)(CLP(GraphicsStateGuardian) *glgsg,
  const SamplerState &sampler) :
  SamplerContext(sampler)
{
  _glgsg = glgsg;
  _glgsg->_glGenSamplers(1, &_index);
}

////////////////////////////////////////////////////////////////////
//     Function: CLP(SamplerContext)::Denstructor
//       Access: Public
//  Description:
////////////////////////////////////////////////////////////////////
CLP(SamplerContext)::
~CLP(SamplerContext)() {
  if (_index != 0) {
    _glgsg->_glDeleteSamplers(1, &_index);
    _index = 0;
  }
}

////////////////////////////////////////////////////////////////////
//     Function: GLSamplerContext::evict_lru
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
void CLP(SamplerContext)::
evict_lru() {
  dequeue_lru();

  reset_data();
}

////////////////////////////////////////////////////////////////////
//     Function: GLSamplerContext::reset_data
//       Access: Public
//  Description: Resets the texture object to a new one so a new GL
//               texture object can be uploaded.
////////////////////////////////////////////////////////////////////
void CLP(SamplerContext)::
reset_data() {
  // Free the sampler resource.
  _glgsg->_glDeleteSamplers(1, &_index);
  _index = 0;

  // We still need a valid index number, though, in case we want to
  // re-load the sampler later.
  //glGenSamplers(1, &_index);
}

#endif  // OPENGLES
