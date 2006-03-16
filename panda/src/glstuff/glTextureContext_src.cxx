// Filename: glTextureContext.cxx
// Created by:  drose (07Oct99)
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

#include "notify.h"

TypeHandle CLP(TextureContext)::_type_handle;

////////////////////////////////////////////////////////////////////
//     Function: GLTextureContext::estimate_texture_memory
//       Access: Public, Virtual
//  Description: Estimates the amount of texture memory that will be
//               consumed by loading this texture.  This is mainly
//               useful for debugging and reporting purposes.
//
//               Returns a value in bytes.
////////////////////////////////////////////////////////////////////
size_t CLP(TextureContext)::
estimate_texture_memory() {
  nassertr(_texture_memory_size != 0, TextureContext::estimate_texture_memory());
  return _texture_memory_size;
}
