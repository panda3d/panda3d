// Filename: wglGraphicsStateGuardian.cxx
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

#include "wglGraphicsStateGuardian.h"


TypeHandle wglGraphicsStateGuardian::_type_handle;

////////////////////////////////////////////////////////////////////
//     Function: wglGraphicsStateGuardian::Constructor
//       Access: Public
//  Description:
////////////////////////////////////////////////////////////////////
wglGraphicsStateGuardian::
wglGraphicsStateGuardian(const FrameBufferProperties &properties) : 
  GLGraphicsStateGuardian(properties)
{
  _context = (HGLRC)0;
}

////////////////////////////////////////////////////////////////////
//     Function: wglGraphicsStateGuardian::Destructor
//       Access: Public
//  Description:
////////////////////////////////////////////////////////////////////
wglGraphicsStateGuardian::
~wglGraphicsStateGuardian() {
  if (_context != (HGLRC)NULL) {
    wglDeleteContext(_context);
    _context = (HGLRC)NULL;
  }
}
