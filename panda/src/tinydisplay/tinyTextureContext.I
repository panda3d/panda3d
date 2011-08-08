// Filename: tinyTextureContext.I
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


////////////////////////////////////////////////////////////////////
//     Function: TinyTextureContext::Constructor
//       Access: Public
//  Description:
////////////////////////////////////////////////////////////////////
INLINE TinyTextureContext::
TinyTextureContext(PreparedGraphicsObjects *pgo, Texture *tex, int view) :
  TextureContext(pgo, tex, view)
{
  _gltex.num_levels = 0;
  _gltex.allocated_buffer = NULL;
  _gltex.total_bytecount = 0;
}

////////////////////////////////////////////////////////////////////
//     Function: TinyTextureContext::Destructor
//       Access: Public
//  Description:
////////////////////////////////////////////////////////////////////
INLINE TinyTextureContext::
~TinyTextureContext() {
  nassertv(_gltex.num_levels == 0 && _gltex.allocated_buffer == NULL && _gltex.total_bytecount == 0);
}
