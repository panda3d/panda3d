// Filename: stitchImage.h
// Created by:  drose (04Nov99)
//
////////////////////////////////////////////////////////////////////
//
// PANDA 3D SOFTWARE
// Copyright (c) 2001, Disney Enterprises, Inc.  All rights reserved
//
// All use of this software is subject to the terms of the Panda 3d
// Software license.  You should have received a copy of this license
// along with this source code; you will also find a current copy of
// the license at http://www.panda3d.org/license.txt .
//
// To contact the maintainers of this program write to
// panda3d@yahoogroups.com .
//
////////////////////////////////////////////////////////////////////

#ifndef STITCHIMAGE_H
#define STITCHIMAGE_H

#include "pandaappbase.h"

#include "stitchPoint.h"
#include "morphGrid.h"

#include <lmatrix.h>
#include "luse.h"
#include <pnmImage.h>
#include "filename.h"

#include <map>

class StitchLens;
class TriangleRasterizer;
class RasterizerVertex;
class LayeredImage;

class StitchImage {
public:
  StitchImage(const string &name, const string &filename,
              StitchLens *lens,
              const LVecBase2d &size_pixels,
              const LVecBase2d &pixels_per_mm,
              const LVecBase2d &film_offset_mm);

  bool has_name() const;
  string get_name() const;
  bool has_filename() const;
  string get_filename() const;
  bool has_fade_filename() const;
  string get_fade_filename() const;
  void set_fade_filename(const Filename &filename);

  // This function reads the image file if it is available.
  bool read_file();
  void clear_file();

  // These functions handle the writing of the image file.
  // open_output_file() should be called first.  open_layer() and
  // close_layer() should be called in pairs; after a call to
  // open_layer(), the _data member is guaranteed to contain a
  // PNMImage that may be empty or may contain the contents of
  // previous layers.  close_layer() should be called as each layer is
  // filled, and close_output_file() should be called when the image
  // is completely done.
  void open_output_file();
  void open_layer(const string &layer_name);
  bool close_layer(bool nonempty);
  bool close_output_file();

  void clear_transform();
  void set_transform(const LMatrix4d &transform);
  void set_hpr(const LVecBase3d &hpr);
  void set_pos(const LPoint3d &pos);
  const LVecBase3d &get_hpr() const;
  const LPoint3d &get_pos() const;

  void show_points(double radius, const Colord &color);
  void setup_grid(int x_verts, int y_verts);
  int get_x_verts() const;
  int get_y_verts() const;

  LPoint2d get_grid_uv(int xv, int yv);
  LPoint2d get_grid_pixel(int xv, int yv);
  LVector3d get_grid_vector(int xv, int yv);
  double get_grid_alpha(int xv, int yv);

  const LVecBase2d &get_size_pixels() const;
  void set_size_pixels(const LVecBase2d &size_pixels);
  LVecBase2d get_size_mm() const;

  LVector3d extrude(const LPoint2d &point_uv) const;
  LPoint2d project(const LVector3d &vec) const;
  double get_alpha(const LPoint2d &point_uv) const;

  void reset_singularity_detected();

  // This function simply passes the indicated triangle on to the
  // rasterizer.  It exists here in the lens so that the lens may do
  // something special if the triangle crosses a seam or singularity
  // in the lens' coordinate space.
  void draw_triangle(TriangleRasterizer &rast,
                     const RasterizerVertex *v0,
                     const RasterizerVertex *v1,
                     const RasterizerVertex *v2);

  // This function is to be called after all triangles have been
  // drawn; it will draw pixel-by-pixel all the points within
  // _singularity_radius of any singularity points the lens may have
  // (these points were not draw by draw_triangle(), above).
  void pick_up_singularity(TriangleRasterizer &rast,
                           StitchImage *input);

  void add_point(const string &name, const LPoint2d &pixel);
  void set_output_scale_factor(double factor);

  void output(ostream &out) const;

  PNMImage *_data;
  StitchLens *_lens;
  LVecBase2d _size_pixels, _size_mm;
  LVecBase2d _pixels_per_mm;
  LVecBase2d _film_offset_mm;
  LMatrix3d _mm_to_uv, _uv_to_mm;
  LMatrix3d _pixels_to_mm, _mm_to_pixels;
  LMatrix3d _pixels_to_uv, _uv_to_pixels;
  bool _hpr_set;
  LVecBase3d _hpr;
  bool _pos_set;
  LPoint3d _pos;

  enum LayeredType {
    LT_flat,        // One flat image--no layers.
    LT_separate,    // A separate image file for each layer.
    LT_combined,    // A single image file with multiple layers.
  };
  LayeredType _layered_type;

  bool _show_points;
  double _point_radius;
  Colord _point_color;
  Colord _untextured_color;

  typedef map<string, LPoint2d> Points;
  Points _points;
  LMatrix4d _transform, _inv_transform;
  MorphGrid _morph;

  // This index number is filled in by the Stitcher.  It allows us to
  // sort the images in order as they are specified in the command
  // file.
  int _index;

private:
  void setup_pixel_scales();
  void resize_data();
  void fade_out();

private:
  Filename _filename;
  Filename _fade_filename;
  string _name;

  int _x_verts, _y_verts;
  int _layer_index;
  string _layer_name;
  LayeredImage *_layered_image;
  LVecBase2d _orig_size_pixels;
  LVecBase2d _orig_pixels_per_mm;
};

inline ostream &operator << (ostream &out, const StitchImage &i) {
  i.output(out);
  return out;
}

// An STL function object to sort image pointers by index number.
class StitchImageByIndex {
public:
  bool operator()(const StitchImage *a, const StitchImage *b) const {
    return a->_index < b->_index;
  }
};

#endif

