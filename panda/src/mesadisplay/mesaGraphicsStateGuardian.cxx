// Filename: mesaGraphicsStateGuardian.cxx
// Created by:  drose (09Feb04)
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

#include "mesaGraphicsStateGuardian.h"

TypeHandle MesaGraphicsStateGuardian::_type_handle;

////////////////////////////////////////////////////////////////////
//     Function: MesaGraphicsStateGuardian::Constructor
//       Access: Public
//  Description:
////////////////////////////////////////////////////////////////////
MesaGraphicsStateGuardian::
MesaGraphicsStateGuardian(const FrameBufferProperties &properties) : 
  GLGraphicsStateGuardian(properties)
{
  _context = OSMesaCreateContext(OSMESA_RGBA, NULL);
}

////////////////////////////////////////////////////////////////////
//     Function: MesaGraphicsStateGuardian::Destructor
//       Access: Public
//  Description:
////////////////////////////////////////////////////////////////////
MesaGraphicsStateGuardian::
~MesaGraphicsStateGuardian() {
  if (_context != (OSMesaContext)NULL) {
    OSMesaDestroyContext(_context);
    _context = (OSMesaContext)NULL;
  }
}
