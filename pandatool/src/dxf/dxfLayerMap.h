// Filename: dxfLayerMap.h
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

#ifndef DXFLAYERMAP_H
#define DXFLAYERMAP_H

#include "pandatoolbase.h"
#include "pmap.h"

class DXFLayer;
class DXFFile;

////////////////////////////////////////////////////////////////////
// 	 Class : DXFLayerMap
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
