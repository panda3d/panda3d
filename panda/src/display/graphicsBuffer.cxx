// Filename: graphicsBuffer.cxx
// Created by:  drose (06Feb04)
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

#include "graphicsBuffer.h"

TypeHandle GraphicsBuffer::_type_handle;

////////////////////////////////////////////////////////////////////
//     Function: GraphicsBuffer::Constructor
//       Access: Protected
//  Description: Normally, the GraphicsBuffer constructor is not
//               called directly; these are created instead via the
//               GraphicsEngine::make_buffer() function.
////////////////////////////////////////////////////////////////////
GraphicsBuffer::
GraphicsBuffer(GraphicsPipe *pipe, GraphicsStateGuardian *gsg,
               int x_size, int y_size) :
  GraphicsOutput(pipe, gsg)
{
#ifdef DO_MEMORY_USAGE
  MemoryUsage::update_type(this, this);
#endif

  if (display_cat.is_debug()) {
    display_cat.debug()
      << "Creating new offscreen buffer using GSG " << (void *)gsg << "\n";
  }

  _x_size = x_size;
  _y_size = y_size;
  _has_size = true;
}

////////////////////////////////////////////////////////////////////
//     Function: GraphicsBuffer::Destructor
//       Access: Published, Virtual
//  Description: 
////////////////////////////////////////////////////////////////////
GraphicsBuffer::
~GraphicsBuffer() {
}
