// Filename: osMesaGraphicsStateGuardian.cxx
// Created by:  drose (09Feb04)
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

#include "osMesaGraphicsStateGuardian.h"

TypeHandle OSMesaGraphicsStateGuardian::_type_handle;

////////////////////////////////////////////////////////////////////
//     Function: OSMesaGraphicsStateGuardian::Constructor
//       Access: Public
//  Description:
////////////////////////////////////////////////////////////////////
OSMesaGraphicsStateGuardian::
OSMesaGraphicsStateGuardian(const FrameBufferProperties &properties,
                            OSMesaGraphicsStateGuardian *share_with) : 
  MesaGraphicsStateGuardian(properties)
{
  OSMesaContext share_context = NULL;
  if (share_with != (OSMesaGraphicsStateGuardian *)NULL) {
    share_context = share_with->_context;
    _prepared_objects = share_with->get_prepared_objects();
  }

  _context = OSMesaCreateContext(OSMESA_RGBA, share_context);

  // I think OSMesa always creates single-buffered contexts.  I don't
  // see any documentation to this effect, but it seems to be the
  // case.
  FrameBufferProperties props = get_properties();
  int mode = props.get_frame_buffer_mode();
  mode = (mode & ~FrameBufferProperties::FM_buffer) | FrameBufferProperties::FM_single_buffer;
  props.set_frame_buffer_mode(mode);
  set_properties(props);
}

////////////////////////////////////////////////////////////////////
//     Function: OSMesaGraphicsStateGuardian::Destructor
//       Access: Public
//  Description:
////////////////////////////////////////////////////////////////////
OSMesaGraphicsStateGuardian::
~OSMesaGraphicsStateGuardian() {
  if (_context != (OSMesaContext)NULL) {
    OSMesaDestroyContext(_context);
    _context = (OSMesaContext)NULL;
  }
}
