// Filename: wglGraphicsStateGuardian.I
// Created by:  drose (27Jan03)
//
////////////////////////////////////////////////////////////////////
//
// PANDA 3D SOFTWARE
// Copyright (c) 2001, Disney Enterprises, Inc.  All rights reserved
//
// All use of this software is subject to the terms of the Panda 3d
// Software license.  You should have received a copy of this license
// along with this source code; you will also find a current copy of
// the license at http://www.panda3d.org/license.txt .
//
// To contact the maintainers of this program write to
// panda3d@yahoogroups.com .
//
////////////////////////////////////////////////////////////////////

////////////////////////////////////////////////////////////////////
//     Function: wglGraphicsStateGuardian::get_pfnum
//       Access: Public
//  Description: Returns the pixel format number chosen for windows
//               that use this context.
////////////////////////////////////////////////////////////////////
INLINE int wglGraphicsStateGuardian::
get_pfnum() const {
  return _pfnum;
}

////////////////////////////////////////////////////////////////////
//     Function: wglGraphicsStateGuardian::get_context
//       Access: Public
//  Description: Returns the GL context associated with the GSG.  If
//               the context has not yet been created, this creates a
//               suitable context for rendering to the indicated
//               window.  This means that this method may only be
//               called from within the draw thread.
////////////////////////////////////////////////////////////////////
INLINE HGLRC wglGraphicsStateGuardian::
get_context(HDC hdc) {
  if (!_made_context) {
    make_context(hdc);
  }
  return _context;
}
