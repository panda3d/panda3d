// Filename: tinyTextureContext.I
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


////////////////////////////////////////////////////////////////////
//     Function: TinyTextureContext::Constructor
//       Access: Public
//  Description:
////////////////////////////////////////////////////////////////////
INLINE TinyTextureContext::
TinyTextureContext(PreparedGraphicsObjects *pgo, Texture *tex) :
  TextureContext(pgo, tex)
{
  _gltex = NULL;
}

////////////////////////////////////////////////////////////////////
//     Function: TinyTextureContext::Destructor
//       Access: Public
//  Description:
////////////////////////////////////////////////////////////////////
INLINE TinyTextureContext::
~TinyTextureContext() {
  nassertv(_gltex == NULL);
}
