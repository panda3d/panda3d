/**
 * PANDA 3D SOFTWARE
 * Copyright (c) Carnegie Mellon University.  All rights reserved.
 *
 * All use of this software is subject to the terms of the revised BSD
 * license.  You should have received a copy of this license along
 * with this source code in a file named "LICENSE."
 *
 * @file dxfLayerMap.cxx
 * @author drose
 * @date 2004-05-04
 */

#include "dxfLayerMap.h"
#include "dxfFile.h"

/**
 * Looks up the layer name in the map, and returns a pointer to the associated
 * DXFLayer.  If this is the first time this layer name has been used, creates
 * a new DXFLayer by the given name.  In this case, it calls
 * dxffile->new_layer() to create the layer, allowing user code to override
 * this function to create a specialized time, if desired.
 */
DXFLayer *DXFLayerMap::
get_layer(const std::string &name, DXFFile *dxffile) {
  iterator lmi;
  lmi = find(name);
  if (lmi != end()) {
    // The layer was already here.
    return (*lmi).second;
  }

  // Need a new layer.
  DXFLayer *layer = dxffile->new_layer(name);
  (*this)[name] = layer;

  return layer;
}
