/**
 * PANDA 3D SOFTWARE
 * Copyright (c) Carnegie Mellon University.  All rights reserved.
 *
 * All use of this software is subject to the terms of the revised BSD
 * license.  You should have received a copy of this license along
 * with this source code in a file named "LICENSE."
 *
 * @file graphicsDevice.cxx
 * @author masad
 * @date 2003-07-21
 */

#include "graphicsDevice.h"
#include "graphicsPipe.h"
#include "config_display.h"

TypeHandle GraphicsDevice::_type_handle;

/**
 * Normally, the GraphicsDevice constructor holds a reference to the Graphics
 * Pipe that it is part of
 */
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

/**
 *
 */
GraphicsDevice::
~GraphicsDevice() {
  // And we shouldn't have a GraphicsPipe pointer anymore.  nassertv(_pipe ==
  // (GraphicsPipe *)NULL);
}
