/**
 * PANDA 3D SOFTWARE
 * Copyright (c) Carnegie Mellon University.  All rights reserved.
 *
 * All use of this software is subject to the terms of the revised BSD
 * license.  You should have received a copy of this license along
 * with this source code in a file named "LICENSE."
 *
 * @file eggToDXFLayer.h
 * @author drose
 * @date 2004-05-04
 */

#ifndef EGGTODXFLAYER_H
#define EGGTODXFLAYER_H

#include "pandatoolbase.h"
#include "pmap.h"
#include "pvector.h"
#include "luse.h"

class EggToDXF;
class EggPolygon;
class EggGroupNode;

/**
 * A single layer in the DXF file to be written by EggToDXF.
 */
class EggToDXFLayer {
public:
  EggToDXFLayer(EggToDXF *egg2dxf, EggGroupNode *group);
  EggToDXFLayer(const EggToDXFLayer &copy);
  void operator = (const EggToDXFLayer &copy);

  void add_color(const LColor &color);
  void choose_overall_color();

  void write_layer(std::ostream &out);
  void write_polyline(EggPolygon *poly, std::ostream &out);
  void write_3d_face(EggPolygon *poly, std::ostream &out);
  void write_entities(std::ostream &out);

private:
  int get_autocad_color(const LColor &color);

  typedef pmap<int, int> ColorCounts;
  ColorCounts _color_counts;

  EggToDXF *_egg2dxf;
  EggGroupNode *_group;
  int _layer_color;
};

typedef pvector<EggToDXFLayer> EggToDXFLayers;

#endif
