/**
 * PANDA 3D SOFTWARE
 * Copyright (c) Carnegie Mellon University.  All rights reserved.
 *
 * All use of this software is subject to the terms of the revised BSD
 * license.  You should have received a copy of this license along
 * with this source code in a file named "LICENSE."
 *
 * @file heightfieldTesselator.h
 * @author jyelon
 * @date 2006-07-17
 */

#ifndef HEIGHTFIELDTESSELATOR_H
#define HEIGHTFIELDTESSELATOR_H

#include "pandabase.h"

#include "luse.h"
#include "pandaNode.h"
#include "pointerTo.h"
#include "namable.h"
#include "pnmImage.h"
#include "geom.h"
#include "geomTristrips.h"
#include "geomTriangles.h"
#include "geomVertexWriter.h"
#include "geomVertexFormat.h"
#include "nodePath.h"

/**
 * Converts a height field in the form of a greyscale image into a scene
 * consisting of a number of GeomNodes.
 *
 * The tesselation uses an LOD algorithm.  You supply a "focal point" (X,Y)
 * which tells the tesselator where the bulk of the detail should be
 * concentrated.  The intent is that as the player walks around the terrain,
 * you should occasionally move the focal point to wherever the player is.
 * You should not move the focal point every frame: tesselation is not that
 * fast.  Also, changing the focal point may cause popping, so it is best to
 * minimize the number of changes.  There are a number of parameters that you
 * can use to control tesselation, such as a target polygon count, and a
 * visibility radius.
 *
 * The heightfield needs to be a multiple of 128 pixels in each dimension.  It
 * does not need to be square, and it does not need to be a power of two.  For
 * example, a 384 x 640 heightfield is fine.  Be aware that tesselation time
 * is proportional to heightfield area, so if you plan to use a size larger
 * than about 512x512, it may be desirable to benchmark.
 *
 * Altering parameters, such as the poly count, the view radius, or the focal
 * point, does not alter any GeomNodes already generated.  Parameter changes
 * only affect subsequently-generated GeomNodes.  It is possible to cache many
 * different tesselations of the same terrain.
 *
 */

class EXPCL_PANDA_GRUTIL HeightfieldTesselator : public Namable {
PUBLISHED:
  INLINE explicit HeightfieldTesselator(const std::string &name);
  INLINE ~HeightfieldTesselator();

  INLINE PNMImage &heightfield();
  INLINE bool set_heightfield(const Filename &filename, PNMFileType *type = nullptr);
  INLINE void set_poly_count(int n);
  INLINE void set_visibility_radius(int r);
  INLINE void set_focal_point(int x, int y);
  INLINE void set_horizontal_scale(double h);
  INLINE void set_vertical_scale(double v);
  INLINE void set_max_triangles(int n);

  double get_elevation(double x, double y);

  NodePath generate();

private:

  // These are initialized during the first 'generate'
  int  _radii[16];
  bool _radii_calculated;

  // These are only valid during the generate process.
  int *_triangle_totals;
  int *_vertex_index;
  int *_dirty_vertices;

  // These are only valid when a geom is open.
  int _next_index;
  int _last_vertex_a;
  int _last_vertex_b;
  PT(GeomVertexData) _vdata;
  GeomVertexWriter *_vertex_writer;
  GeomVertexWriter *_normal_writer;
  PT(GeomTriangles) _triangles;

  INLINE bool subdivide(int scale, int x, int y);
  void calculate_radii(int scale);
  void generate_square(NodePath root, int scale, int x, int y, bool forceclose);
  int  count_triangles(int scale, int x, int y);
  int  get_vertex(int x, int y);
  void add_quad_to_strip(int v1, int v2, int v3, int v4);
  void add_quad(int v1, int v2, int v3, int v4);
  void fix_heightfield(int size);
  void open_geom();
  void close_geom(NodePath root);

  PNMImage _heightfield;
  int _poly_count;
  int _visibility_radius;
  int _focal_x;
  int _focal_y;
  double _horizontal_scale;
  double _vertical_scale;
  int _max_triangles;
};

#include "heightfieldTesselator.I"

#endif
