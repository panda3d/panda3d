// Filename: wglGraphicsStateGuardian.I
// Created by:  drose (27Jan03)
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
//     Function: wglGraphicsStateGuardian::get_pfnum
//       Access: Public
//  Description: Returns the pixel format number chosen for windows
//               that use this context.  In OpenGL under Microsoft
//               Windows, the window must be created first and then
//               the GL context is created from the window, and the
//               context inherits the pixel format of the window.
//               Therefore, all windows that share a particular
//               context must also share the same pixel format.
////////////////////////////////////////////////////////////////////
INLINE int wglGraphicsStateGuardian::
get_pfnum() const {
  return _pfnum;
}

////////////////////////////////////////////////////////////////////
//     Function: wglGraphicsStateGuardian::made_context
//       Access: Public
//  Description: Returns true if the GSG has had a context made for it
//               already, false otherwise.  If this returns false, the
//               next call to get_context() will quietly make a new
//               context.
////////////////////////////////////////////////////////////////////
INLINE bool wglGraphicsStateGuardian::
made_context() const {
  return _made_context;
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

////////////////////////////////////////////////////////////////////
//     Function: wglGraphicsStateGuardian::get_twindow_dc
//       Access: Public
//  Description: Returns the DC associated with the temporary,
//               invisible window that was created with the gsg to
//               query WGL extensions.
////////////////////////////////////////////////////////////////////
INLINE HDC wglGraphicsStateGuardian::
get_twindow_dc() {
  if (_twindow_dc == 0) {
    make_twindow();
  }
  return _twindow_dc;
}

