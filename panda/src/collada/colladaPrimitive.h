// Filename: colladaPrimitive.h
// Created by:  rdb (23May11)
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

#ifndef COLLADAPRIMITIVE_H
#define COLLADAPRIMITIVE_H

#include "config_collada.h"
#include "referenceCount.h"
#include "geomVertexData.h"
#include "geom.h"
#include "geomPrimitive.h"

#include "colladaInput.h"

class domP;
class domLines;
class domLinestrips;
class domPolygons;
class domPolylist;
class domTriangles;
class domTrifans;
class domTristrips;

////////////////////////////////////////////////////////////////////
//       Class : ColladaPrimitive
// Description : Class that deals with COLLADA primitive structures,
//               such as <triangles> and <polylist>.
////////////////////////////////////////////////////////////////////
class ColladaPrimitive : public ReferenceCount {
public:
  static ColladaPrimitive *from_dom(domLines &lines);
  static ColladaPrimitive *from_dom(domLinestrips &linestrips);
  static ColladaPrimitive *from_dom(domPolygons &polygons);
  static ColladaPrimitive *from_dom(domPolylist &polylist);
  static ColladaPrimitive *from_dom(domTriangles &triangles);
  static ColladaPrimitive *from_dom(domTrifans &trifans);
  static ColladaPrimitive *from_dom(domTristrips &tristrips);

  unsigned int write_data(GeomVertexData *vdata, int start_row, domP &p);

  INLINE PT(Geom) get_geom() const;
  INLINE const string &get_material() const;

private:
  ColladaPrimitive(GeomPrimitive *prim, daeTArray<daeSmartRef<domInput_local_offset> > &inputs);
  void load_primitive(domP &p);
  void load_primitives(daeTArray<daeSmartRef<domP> > &p_array);
  INLINE void add_input(ColladaInput *input);

  typedef pvector<PT(ColladaInput)> Inputs;
  Inputs _inputs;

  unsigned int _stride;
  PT(Geom) _geom;
  PT(GeomVertexData) _vdata;
  PT(GeomPrimitive) _gprim;
  string _material;
};

#include "colladaPrimitive.I"

#endif
