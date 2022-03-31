/**
 * PANDA 3D SOFTWARE
 * Copyright (c) Carnegie Mellon University.  All rights reserved.
 *
 * All use of this software is subject to the terms of the revised BSD
 * license.  You should have received a copy of this license along
 * with this source code in a file named "LICENSE."
 *
 * @file fisheyeMaker.h
 * @author drose
 * @date 2005-10-03
 */

#ifndef FISHEYEMAKER_H
#define FISHEYEMAKER_H

#include "pandabase.h"

#include "pandaNode.h"
#include "pointerTo.h"
#include "namable.h"

class GeomVertexWriter;

/**
 * This class is similar to CardMaker, but instead of generating ordinary
 * cards, it generates a circular rose that represents the projection of a 3-D
 * scene through a fisheye lens.  The texture coordinates of the rose are
 * defined so that each 2-D vertex has a 3-D UVW that reflects the
 * corresponding position in 3-D space of that particular vertex.
 *
 * This class is particularly suited for converting cube maps to sphere maps.
 */
class EXPCL_PANDA_GRUTIL FisheyeMaker : public Namable {
PUBLISHED:
  INLINE explicit FisheyeMaker(const std::string &name);
  INLINE ~FisheyeMaker();

  void reset();
  void set_fov(PN_stdfloat fov);
  INLINE void set_num_vertices(int num_vertices);
  INLINE void set_square_inscribed(bool square_inscribed, PN_stdfloat square_radius);
  INLINE void set_reflection(bool reflection);

  PT(PandaNode) generate();

private:
  void add_vertex(GeomVertexWriter &vertex, GeomVertexWriter &texcoord,
                  PN_stdfloat r, PN_stdfloat a);

  void add_square_vertex(GeomVertexWriter &vertex, GeomVertexWriter &texcoord,
                         PN_stdfloat a);

  PN_stdfloat _fov;
  PN_stdfloat _half_fov_rad;
  int _num_vertices;
  bool _square_inscribed;
  PN_stdfloat _square_radius;
  PN_stdfloat _reflect;
};

#include "fisheyeMaker.I"

#endif
