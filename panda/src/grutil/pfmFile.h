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
class PNMImage;

////////////////////////////////////////////////////////////////////
//       Class : PfmFile
// Description : Defines a pfm file, a 2-d table of floating-point
//               numbers, either 3-component or 1-component, or with a
//               special extension, 4-component.
////////////////////////////////////////////////////////////////////
class EXPCL_PANDA_GRUTIL PfmFile {
PUBLISHED:
  PfmFile();
  PfmFile(const PfmFile &copy);
  void operator = (const PfmFile &copy);

  void clear();
  void clear(int x_size, int y_size, int num_channels);

  BLOCKING bool read(const Filename &fullpath);
  BLOCKING bool read(istream &in, const Filename &fullpath = Filename(),
                     const string &magic_number = string());
  BLOCKING bool write(const Filename &fullpath);
  BLOCKING bool write(ostream &out, const Filename &fullpath = Filename());

  BLOCKING bool load(const PNMImage &pnmimage);
  BLOCKING bool store(PNMImage &pnmimage) const;

  INLINE bool is_valid() const;

  INLINE int get_x_size() const;
  INLINE int get_y_size() const;
  INLINE PN_float32 get_scale() const;
  INLINE int get_num_channels() const;

  INLINE bool has_point(int x, int y) const;
  INLINE const LPoint3f &get_point(int x, int y) const;
  INLINE void set_point(int x, int y, const LVecBase3f &point);
  INLINE LPoint3f &modify_point(int x, int y);
  INLINE const LPoint4f &get_point4(int x, int y) const;
  INLINE void set_point4(int x, int y, const LVecBase4f &point);
  INLINE LPoint4f &modify_point4(int x, int y);

  BLOCKING bool calc_average_point(LPoint3f &result, PN_float32 x, PN_float32 y, PN_float32 radius) const;
  BLOCKING bool calc_min_max(LVecBase3f &min_points, LVecBase3f &max_points) const;
  BLOCKING bool calc_autocrop(int &x_begin, int &x_end, int &y_begin, int &y_end) const;
  BLOCKING INLINE bool calc_autocrop(LVecBase4 &range) const;
 
  bool is_row_empty(int y, int x_begin, int x_end) const;
  bool is_column_empty(int x, int y_begin, int y_end) const;

  INLINE void set_zero_special(bool zero_special);
  INLINE void set_no_data_chan4(bool chan4);
  void set_no_data_value(const LPoint4f &no_data_value);
  INLINE void clear_no_data_value();
  INLINE bool has_no_data_value() const;
  INLINE const LPoint4f &get_no_data_value() const;

  BLOCKING void resize(int new_x_size, int new_y_size);
  BLOCKING void reverse_rows();
  BLOCKING void flip(bool flip_x, bool flip_y, bool transpose);
  BLOCKING void xform(const LMatrix4 &transform);
  BLOCKING void project(const Lens *lens);
  BLOCKING void merge(const PfmFile &other);
  BLOCKING void apply_crop(int x_begin, int x_end, int y_begin, int y_end);

  BLOCKING PT(BoundingHexahedron) compute_planar_bounds(PN_float32 point_dist, PN_float32 sample_radius) const;
  BLOCKING PT(BoundingHexahedron) compute_planar_bounds(const LPoint2 &center, PN_float32 point_dist, PN_float32 sample_radius, bool points_only) const;
  void compute_sample_point(LPoint3f &result,
                            PN_float32 x, PN_float32 y, PN_float32 sample_radius) const;

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

  void box_filter_region(LPoint4f &result,
                         PN_float32 x0, PN_float32 y0, PN_float32 x1, PN_float32 y1) const;
  void box_filter_line(LPoint4f &result, PN_float32 &coverage,
                       PN_float32 x0, int y, PN_float32 x1, PN_float32 y_contrib) const;
  void box_filter_point(LPoint4f &result, PN_float32 &coverage,
                        int x, int y, PN_float32 x_contrib, PN_float32 y_contrib) const;

  class MiniGridCell {
  public:
    MiniGridCell() : _sxi(-1), _syi(-1), _dist(-1) { }
    int _sxi, _syi;
    int _dist;
  };

  void fill_mini_grid(MiniGridCell *mini_grid, int x_size, int y_size, 
                      int xi, int yi, int dist, int sxi, int syi) const;

  static bool has_point_noop(const PfmFile *file, int x, int y);
  static bool has_point_1(const PfmFile *file, int x, int y);
  static bool has_point_3(const PfmFile *file, int x, int y);
  static bool has_point_4(const PfmFile *file, int x, int y);
  static bool has_point_chan4(const PfmFile *file, int x, int y);

private:
  typedef pvector<PN_float32> Table;
  Table _table;

  int _x_size;
  int _y_size;
  PN_float32 _scale;
  int _num_channels;

  bool _has_no_data_value;
  LPoint4f _no_data_value;
  bool _vis_inverse;
  PT(InternalName) _flat_texcoord_name;
  bool _vis_2d;

  typedef bool HasPointFunc(const PfmFile *file, int x, int y);
  HasPointFunc *_has_point;
  
};

#include "pfmFile.I"

#endif
