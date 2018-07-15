/**
 * PANDA 3D SOFTWARE
 * Copyright (c) Carnegie Mellon University.  All rights reserved.
 *
 * All use of this software is subject to the terms of the revised BSD
 * license.  You should have received a copy of this license along
 * with this source code in a file named "LICENSE."
 *
 * @file lineSegs.h
 * @author drose
 * @date 2002-03-16
 */

#ifndef LINESEGS_H
#define LINESEGS_H

#include "pandabase.h"

#include "luse.h"
#include "geom.h"
#include "geomNode.h"
#include "geomVertexData.h"
#include "namable.h"

#include "pvector.h"

/**
 * Encapsulates creation of a series of connected or disconnected line
 * segments or points, for drawing paths or rays.  This class doesn't attempt
 * to be the smartest it could possibly be; it's intended primarily as a
 * visualization and editing tool.
 */
class EXPCL_PANDA_GRUTIL LineSegs : public Namable {
PUBLISHED:
  explicit LineSegs(const std::string &name = "lines");
  ~LineSegs();

  void reset();
  INLINE void set_color(PN_stdfloat r, PN_stdfloat g, PN_stdfloat b, PN_stdfloat a = 1.0f);
  INLINE void set_color(const LColor &color);
  INLINE void set_thickness(PN_stdfloat thick);

  INLINE void move_to(PN_stdfloat x, PN_stdfloat y, PN_stdfloat z);
  void move_to(const LVecBase3 &v);

  INLINE void draw_to(PN_stdfloat x, PN_stdfloat y, PN_stdfloat z);
  void draw_to(const LVecBase3 &v);

  const LVertex &get_current_position();
  bool is_empty();

  INLINE GeomNode *create(bool dynamic = false);
  GeomNode *create(GeomNode *previous, bool dynamic = false);

  // Functions to move the line vertices after they have been created.
  INLINE int get_num_vertices() const;
  LVertex get_vertex(int n) const;
  MAKE_SEQ(get_vertices, get_num_vertices, get_vertex);
  void set_vertex(int n, const LVertex &vert);
  INLINE void set_vertex(int vertex, PN_stdfloat x, PN_stdfloat y, PN_stdfloat z);

  LColor get_vertex_color(int vertex) const;
  MAKE_SEQ(get_vertex_colors, get_num_vertices, get_vertex_color);
  void set_vertex_color(int vertex, const LColor &c);
  INLINE void set_vertex_color(int vertex, PN_stdfloat r, PN_stdfloat g, PN_stdfloat b, PN_stdfloat a = 1.0f);

private:
  class Point {
  public:
    INLINE Point();
    INLINE Point(const LVecBase3 &point, const LColor &color);
    INLINE Point(const Point &copy);
    INLINE void operator = (const Point &copy);

    LVertex _point;
    UnalignedLVecBase4 _color;
  };

  typedef pvector<Point> SegmentList;
  typedef pvector<SegmentList> LineList;

  LineList _list;
  LColor _color;
  PN_stdfloat _thick;

  PT(GeomVertexData) _created_data;
};

#include "lineSegs.I"

#endif
