// Filename: chromium.SavedFrameBuffer.I
// Created by:  drose (06Oct99)
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
//     Function: CRSavedFrameBuffer::Constructor
//       Access: Public
//  Description:
////////////////////////////////////////////////////////////////////
INLINE CRSavedFrameBuffer::
CRSavedFrameBuffer(const RenderBuffer &buffer, CPT(DisplayRegion) dr) :
  SavedFrameBuffer(buffer, dr)
{
}

////////////////////////////////////////////////////////////////////
//     Function: CRSavedFrameBuffer::Destructor
//       Access: Public
//  Description:
////////////////////////////////////////////////////////////////////
INLINE CRSavedFrameBuffer::
~CRSavedFrameBuffer() {
}

