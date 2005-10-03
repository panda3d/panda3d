// Filename: fisheyeMaker.h
// Created by:  drose (3Oct05)
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

#ifndef FISHEYEMAKER_H
#define FISHEYEMAKER_H

#include "pandabase.h"

#include "pandaNode.h"
#include "pointerTo.h"
#include "namable.h"

class GeomVertexWriter;

////////////////////////////////////////////////////////////////////
//       Class : FisheyeMaker
// Description : This class is similar to CardMaker, but instead of
//               generating ordinary cards, it generates a circular
//               rose that represents the projection of a 3-D scene
//               through a fisheye lens.  The texture coordinates of
//               the rose are defined so that each 2-D vertex has a
//               3-D UVW that reflects the corresponding position in
//               3-D space of that particular vertex.
//
//               This class is particularly suited for converting cube
//               maps to sphere maps.
////////////////////////////////////////////////////////////////////
class EXPCL_PANDA FisheyeMaker : public Namable {
PUBLISHED:
  INLINE FisheyeMaker(const string &name);
  INLINE ~FisheyeMaker();

  void reset();
  void set_fov(float fov);
  INLINE void set_num_vertices(int num_vertices);
  INLINE void set_square_inscribed(bool square_inscribed, float square_radius);

  PT(PandaNode) generate();

private:
  void add_vertex(GeomVertexWriter &vertex, GeomVertexWriter &texcoord,
                  float r, float a);

  void add_square_vertex(GeomVertexWriter &vertex, GeomVertexWriter &texcoord,
                         float a);

  float _fov;
  float _half_fov_rad;
  int _num_vertices;
  bool _square_inscribed;
  float _square_radius;
};

#include "fisheyeMaker.I"

#endif

