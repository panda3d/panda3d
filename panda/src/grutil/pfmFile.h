// Filename: pfmFile.h
// Created by:  drose (23Dec10)
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

#ifndef PFMFILE_H
#define PFMFILE_H

#include "pandabase.h"
#include "luse.h"
#include "nodePath.h"
#include "boundingHexahedron.h"
#include "internalName.h"

class GeomNode;
class Lens;

////////////////////////////////////////////////////////////////////
//       Class : PfmFile
// Description : Defines a pfm file, a 2-d table of floating-point
//               numbers, either 3-component or 1-component.
////////////////////////////////////////////////////////////////////
class EXPCL_PANDA_GRUTIL PfmFile {
PUBLISHED:
  PfmFile();
  PfmFile(const PfmFile &copy);
  void operator = (const PfmFile &copy);

  void clear();
  void clear(int x_size, int y_size, int num_channels);

  BLOCKING bool read(const Filename &fullpath);
  BLOCKING bool read(istream &in);
  BLOCKING bool write(const Filename &fullpath);
  BLOCKING bool write(ostream &out);

  INLINE bool is_valid() const;

  INLINE int get_x_size() const;
  INLINE int get_y_size() const;
  INLINE PN_stdfloat get_scale() const;
  INLINE int get_num_channels() const;

  INLINE bool has_point(int x, int y) const;
  INLINE const LPoint3 &get_point(int x, int y) const;
  INLINE void set_point(int x, int y, const LVecBase3 &point);
  INLINE LPoint3 &modify_point(int x, int y);

  BLOCKING bool calc_average_point(LPoint3 &result, PN_stdfloat x, PN_stdfloat y, PN_stdfloat radius) const;
  BLOCKING bool calc_min_max(LVecBase3 &min_points, LVecBase3 &max_points) const;

  INLINE void set_zero_special(bool zero_special);
  INLINE void set_no_data_value(const LPoint3 &no_data_value);
  INLINE void clear_no_data_value();
  INLINE bool has_no_data_value() const;
  INLINE const LPoint3 &get_no_data_value() const;

  BLOCKING void resize(int new_x_size, int new_y_size);
  BLOCKING void reverse_rows();
  BLOCKING void xform(const LMatrix4 &transform);
  BLOCKING void project(const Lens *lens);
  BLOCKING void merge(const PfmFile &other);

  BLOCKING PT(BoundingHexahedron) compute_planar_bounds(PN_stdfloat point_dist, PN_stdfloat sample_radius) const;
  BLOCKING PT(BoundingHexahedron) compute_planar_bounds(const LPoint2 &center, PN_stdfloat point_dist, PN_stdfloat sample_radius, bool points_only) const;
  void compute_sample_point(LPoint3 &result,
                            PN_stdfloat x, PN_stdfloat y, PN_stdfloat sample_radius) const;

  INLINE void set_vis_inverse(bool vis_inverse);
  INLINE bool get_vis_inverse() const;
  INLINE void set_flat_texcoord_name(InternalName *flat_texcoord_name);
  INLINE void clear_flat_texcoord_name();
  INLINE InternalName *get_flat_texcoord_name() const;
  INLINE void set_vis_2d(bool vis_2d);
  INLINE bool get_vis_2d() const;

  BLOCKING NodePath generate_vis_points() const;

  enum MeshFace {
    MF_front = 0x01,
    MF_back  = 0x02,
    MF_both  = 0x03,
  };
  BLOCKING NodePath generate_vis_mesh(MeshFace face = MF_front) const;

private:
  void make_vis_mesh_geom(GeomNode *gnode, bool inverted) const;

  void box_filter_region(LPoint3 &result,
                         PN_stdfloat x0, PN_stdfloat y0, PN_stdfloat x1, PN_stdfloat y1) const;
  void box_filter_line(LPoint3 &result, PN_stdfloat &coverage,
                       PN_stdfloat x0, int y, PN_stdfloat x1, PN_stdfloat y_contrib) const;
  void box_filter_point(LPoint3 &result, PN_stdfloat &coverage,
                        int x, int y, PN_stdfloat x_contrib, PN_stdfloat y_contrib) const;

  class MiniGridCell {
  public:
    MiniGridCell() : _ti(-1), _dist(-1) { }
    int _ti;
    int _dist;
  };

  void fill_mini_grid(MiniGridCell *mini_grid, int x_size, int y_size, 
                      int xi, int yi, int dist, int ti) const;

private:
  typedef pvector<LPoint3> Table;
  Table _table;

  int _x_size;
  int _y_size;
  PN_stdfloat _scale;
  int _num_channels;

  bool _has_no_data_value;
  LPoint3 _no_data_value;
  bool _vis_inverse;
  PT(InternalName) _flat_texcoord_name;
  bool _vis_2d;
};

#include "pfmFile.I"

#endif
