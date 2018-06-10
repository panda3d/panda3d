/**
 * PANDA 3D SOFTWARE
 * Copyright (c) Carnegie Mellon University.  All rights reserved.
 *
 * All use of this software is subject to the terms of the revised BSD
 * license.  You should have received a copy of this license along
 * with this source code in a file named "LICENSE."
 *
 * @file dxfToEggLayer.h
 * @author drose
 * @date 2004-05-04
 */

#ifndef DXFTOEGGLAYER_H
#define DXFTOEGGLAYER_H

#include "pandatoolbase.h"

#include "dxfLayer.h"
#include "eggVertexPool.h"
#include "eggGroup.h"
#include "pointerTo.h"

class EggGroupNode;
class EggVertex;
class DXFVertex;
class DXFToEggConverter;

/**
 * The specialization of DXFLayer used by DXFToEggConverter.  It contains a
 * pointer to an EggGroup and a vertex pool; these are used to build up
 * polygons grouped by layer in the egg file as each polygon is read from the
 * DXF file.
 */
class DXFToEggLayer : public DXFLayer {
public:
  DXFToEggLayer(const std::string &name, EggGroupNode *parent);

  void add_polygon(const DXFToEggConverter *entity);
  void add_line(const DXFToEggConverter *entity);
  EggVertex *add_vertex(const DXFVertex &vertex);

  PT(EggVertexPool) _vpool;
  PT(EggGroup) _group;
};


#endif
