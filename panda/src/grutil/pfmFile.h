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

  bool read(const Filename &fullpath);
  bool read(istream &in);
  bool write(const Filename &fullpath);
  bool write(ostream &out);

  INLINE bool is_valid() const;

  INLINE int get_x_size() const;
  INLINE int get_y_size() const;
  INLINE PN_stdfloat get_scale() const;
  INLINE int get_num_channels() const;

  INLINE bool has_point(int x, int y) const;
  INLINE const LPoint3 &get_point(int x, int y) const;
  INLINE void set_point(int x, int y, const LVecBase3 &point);
  INLINE LPoint3 &modify_point(int x, int y);

  bool calc_average_point(LPoint3 &result, double x, double y, double radius) const;
  bool calc_min_max(LVecBase3 &min_points, LVecBase3 &max_points) const;

  INLINE void set_zero_special(bool zero_special);
  INLINE bool get_zero_special() const;

  void resize(int new_x_size, int new_y_size);
  void reverse_rows();
  void xform(const LMatrix4 &transform);
  void project(const Lens *lens);

  PT(BoundingHexahedron) compute_planar_bounds(double point_dist, double sample_radius) const;

  INLINE void set_vis_inverse(bool vis_inverse);
  INLINE bool get_vis_inverse() const;
  INLINE void set_vis_2d(bool vis_2d);
  INLINE bool get_vis_2d() const;

  NodePath generate_vis_points() const;
  NodePath generate_vis_mesh(bool double_sided) const;

private:
  void make_vis_mesh_geom(GeomNode *gnode, bool inverted) const;

  void compute_sample_point(LPoint3 &result,
                            double x, double y, double sample_radius) const;
  void box_filter_region(LPoint3 &result,
                         double x0, double y0, double x1, double y1) const;
  void box_filter_line(LPoint3 &result, double &coverage,
                       double x0, int y, double x1, double y_contrib) const;
  void box_filter_point(LPoint3 &result, double &coverage,
                        int x, int y, double x_contrib, double y_contrib) const;

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

  bool _zero_special;
  bool _vis_inverse;
  bool _vis_2d;
};

#include "pfmFile.I"

#endif
