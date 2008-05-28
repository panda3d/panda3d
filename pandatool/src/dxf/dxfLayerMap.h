// Filename: dxfLayerMap.h
// Created by:  drose (04May04)
//
////////////////////////////////////////////////////////////////////
//
// PANDA 3D SOFTWARE
// Copyright (c) Carnegie Mellon University.  All rights reserved.
//
// All use of this software is subject to the terms of the revised BSD
// license.  You should have received a copy of this license along
// with this source code in a file named "LICENSE."
//
////////////////////////////////////////////////////////////////////

#ifndef DXFLAYERMAP_H
#define DXFLAYERMAP_H

#include "pandatoolbase.h"
#include "pmap.h"

class DXFLayer;
class DXFFile;

////////////////////////////////////////////////////////////////////
//       Class : DXFLayerMap
// Description : A map of string (layer name) to DXFLayer: that is,
//               the layers of a file ordered by name.  This is used
//               as a lookup within DXFFile to locate the layer
//               associated with a particular entity.
////////////////////////////////////////////////////////////////////
class DXFLayerMap : public pmap<string, DXFLayer *> {
public:
  DXFLayer *get_layer(const string &name, DXFFile *dxffile);
};

#endif
