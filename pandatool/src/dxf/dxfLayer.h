/**
 * PANDA 3D SOFTWARE
 * Copyright (c) Carnegie Mellon University.  All rights reserved.
 *
 * All use of this software is subject to the terms of the revised BSD
 * license.  You should have received a copy of this license along
 * with this source code in a file named "LICENSE."
 *
 * @file dxfLayer.h
 * @author drose
 * @date 2004-05-04
 */

#ifndef DXFLAYER_H
#define DXFLAYER_H

#include "pandatoolbase.h"
#include "namable.h"

/**
 * This represents a "layer" as read from the DXF file.  A layer may be
 * defined by reading the header part of the file, or it may be implicitly
 * defined by an entity's having referenced it.
 *
 * User code may derive from DXFLayer to associate private data with each
 * layer, if desired.
 */
class DXFLayer : public Namable {
public:
  DXFLayer(const std::string &name);
  virtual ~DXFLayer();
};

#endif
