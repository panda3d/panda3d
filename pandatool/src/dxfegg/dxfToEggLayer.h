// Filename: dxfToEggLayer.h
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

////////////////////////////////////////////////////////////////////
// 	 Class : DXFToEggLayer
// Description : The specialization of DXFLayer used by
//               DXFToEggConverter.  It contains a pointer to an
//               EggGroup and a vertex pool; these are used to build
//               up polygons grouped by layer in the egg file as each
//               polygon is read from the DXF file.
////////////////////////////////////////////////////////////////////
class DXFToEggLayer : public DXFLayer {
public:
  DXFToEggLayer(const string &name, EggGroupNode *parent);

  void add_polygon(const DXFToEggConverter *entity);
  void add_line(const DXFToEggConverter *entity);
  EggVertex *add_vertex(const DXFVertex &vertex);
  
  PT(EggVertexPool) _vpool;
  PT(EggGroup) _group;
};


#endif
