// Filename: graphicsDevice.cxx
// Created by:  masad (21Jul03)
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

#include "graphicsDevice.h"
#include "graphicsPipe.h"
#include "config_display.h"

TypeHandle GraphicsDevice::_type_handle;

////////////////////////////////////////////////////////////////////
//     Function: GraphicsDevice::Constructor
//       Access: Protected
//  Description: Normally, the GraphicsDevice constructor holds 
//               a reference to the Graphics Pipe that it is part of
////////////////////////////////////////////////////////////////////
GraphicsDevice::
GraphicsDevice(GraphicsPipe *pipe) {
#ifdef DO_MEMORY_USAGE
  MemoryUsage::update_type(this, this);
#endif
  _pipe = pipe;

  if (display_cat.is_debug()) {
    display_cat.debug()
      << "Creating new device using pipe " << (void *)pipe << "\n";
  }
}

////////////////////////////////////////////////////////////////////
//     Function: GraphicsDevice::Copy Constructor
//       Access: Private
//  Description:
////////////////////////////////////////////////////////////////////
GraphicsDevice::
GraphicsDevice(const GraphicsDevice &) {
  nassertv(false);
}

////////////////////////////////////////////////////////////////////
//     Function: GraphicsDevice::Copy Assignment Operator
//       Access: Private
//  Description:
////////////////////////////////////////////////////////////////////
void GraphicsDevice::
operator = (const GraphicsDevice &) {
  nassertv(false);
}

////////////////////////////////////////////////////////////////////
//     Function: GraphicsDevice::Destructor
//       Access: Published, Virtual
//  Description:
////////////////////////////////////////////////////////////////////
GraphicsDevice::
~GraphicsDevice() {
  // And we shouldn't have a GraphicsPipe pointer anymore.
  //  nassertv(_pipe == (GraphicsPipe *)NULL);
}

