/**
 * PANDA 3D SOFTWARE
 * Copyright (c) Carnegie Mellon University.  All rights reserved.
 *
 * All use of this software is subject to the terms of the revised BSD
 * license.  You should have received a copy of this license along
 * with this source code in a file named "LICENSE."
 *
 * @file dxfLayerMap.h
 * @author drose
 * @date 2004-05-04
 */

#ifndef DXFLAYERMAP_H
#define DXFLAYERMAP_H

#include "pandatoolbase.h"
#include "pmap.h"

class DXFLayer;
class DXFFile;

/**
 * A map of string (layer name) to DXFLayer: that is, the layers of a file
 * ordered by name.  This is used as a lookup within DXFFile to locate the
 * layer associated with a particular entity.
 */
class DXFLayerMap : public pmap<std::string, DXFLayer *> {
public:
  DXFLayer *get_layer(const std::string &name, DXFFile *dxffile);
};

#endif
