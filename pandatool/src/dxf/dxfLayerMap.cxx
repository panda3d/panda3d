// Filename: dxfLayerMap.cxx
// Created by:  drose (04May04)
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

#include "dxfLayerMap.h"
#include "dxfFile.h"

////////////////////////////////////////////////////////////////////
//     Function: DXFLayerMap::get_layer
//       Access: Public
//  Description: Looks up the layer name in the map, and returns a
//               pointer to the associated DXFLayer.  If this is the
//               first time this layer name has been used, creates a
//               new DXFLayer by the given name.  In this case, it
//               calls dxffile->new_layer() to create the layer,
//               allowing user code to override this function to
//               create a specialized time, if desired.
////////////////////////////////////////////////////////////////////
DXFLayer *DXFLayerMap::
get_layer(const string &name, DXFFile *dxffile) {
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

