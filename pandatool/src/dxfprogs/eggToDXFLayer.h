// Filename: eggToDXFLayer.h
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

#ifndef EGGTODXFLAYER_H
#define EGGTODXFLAYER_H

#include "pandatoolbase.h"
#include "pmap.h"
#include "pvector.h"
#include "luse.h"

class EggToDXF;
class EggPolygon;
class EggGroupNode;

////////////////////////////////////////////////////////////////////
//       Class : EggToDXFLayer
// Description : A single layer in the DXF file to be written by
//               EggToDXF.
////////////////////////////////////////////////////////////////////
class EggToDXFLayer {
public:
  EggToDXFLayer(EggToDXF *egg2dxf, EggGroupNode *group);
  EggToDXFLayer(const EggToDXFLayer &copy);
  void operator = (const EggToDXFLayer &copy);

  void add_color(const Colorf &color);
  void choose_overall_color();

  void write_layer(ostream &out);
  void write_polyline(EggPolygon *poly, ostream &out);
  void write_3d_face(EggPolygon *poly, ostream &out);
  void write_entities(ostream &out);

private:
  int get_autocad_color(const Colorf &color);

  typedef pmap<int, int> ColorCounts;
  ColorCounts _color_counts;

  EggToDXF *_egg2dxf;
  EggGroupNode *_group;
  int _layer_color;
};

typedef pvector<EggToDXFLayer> EggToDXFLayers;

#endif
