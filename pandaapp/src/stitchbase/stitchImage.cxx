// Filename: stitchImage.cxx
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

#include "stitchImage.h"
#include "stitchLens.h"
#include "layeredImage.h"
#include "fadeImagePool.h"

#include "compose_matrix.h"
#include "rotate_to.h"
#include "executionEnvironment.h"

#include <stdio.h>  // for sprintf()

StitchImage::
StitchImage(const string &name, const string &filename,
            StitchLens *lens, const LVecBase2d &size_pixels,
            const LVecBase2d &pixels_per_mm,
            const LVecBase2d &film_offset_mm) :
  _lens(lens),
  _size_pixels(size_pixels),
  _pixels_per_mm(pixels_per_mm),
  _film_offset_mm(film_offset_mm),
  _transform(LMatrix4d::ident_mat()),
  _inv_transform(LMatrix4d::ident_mat()),
  _name(name)
{
  _filename = ExecutionEnvironment::expand_string(filename);
  if (_name.empty()) {
    _name = _filename.get_basename_wo_extension();
  }

  _size_mm.set((_size_pixels[0] - 1.0) / _pixels_per_mm[0],
               (_size_pixels[1] - 1.0) / _pixels_per_mm[1]);
  _orig_size_pixels = _size_pixels;
  _orig_pixels_per_mm = _pixels_per_mm;

  // There are several coordinate systems to talk about points on the
  // image.

  // UV's are used for doing most operations.  They range from (0, 0)
  // at the lower-left corner to (1, 1) at the upper-right.

  // Pixels are used when interfacing with the user.  They range from
  // (0, 0) at the upper-left corner to (_size_pixels[0] - 1,
  // _size_pixels[1] - 1) at the lower-right.

  // Millimeters are used when interfacing with the lens.  They start
  // at film_offset_mm at the center, and range from
  // film_offset_mm-size_mm at the lower-left, to
  // film_offset_mm+size_mm at the upper-right.

  /*
  nout << "_pixels_to_uv * _uv_to_pixels is\n"
       << _pixels_to_uv * _uv_to_pixels << "\n"
       << "Corners in pixels:\n"
       << "ll " << LPoint2d(0.0, 0.0) * _uv_to_pixels
       << " lr " << LPoint2d(1.0, 0.0) * _uv_to_pixels
       << " ul " << LPoint2d(0.0, 1.0) * _uv_to_pixels
       << " ur " << LPoint2d(1.0, 1.0) * _uv_to_pixels << "\n"
       << "center " << LPoint2d(0.5, 0.5) * _uv_to_pixels << "\n\n";
  */

  LVector2d mm_per_uv = get_size_mm();

  _uv_to_mm =
    LMatrix3d::translate_mat(LVector2d(-0.5, -0.5)) *
    LMatrix3d::scale_mat(mm_per_uv) *
    LMatrix3d::translate_mat(_film_offset_mm);

  _mm_to_uv =
    LMatrix3d::translate_mat(-_film_offset_mm) *
    LMatrix3d::scale_mat(1.0 / mm_per_uv[0], 1.0 / mm_per_uv[1]) *
    LMatrix3d::translate_mat(LVector2d(0.5, 0.5));

  setup_pixel_scales();

  _show_points = false;
  setup_grid(2, 2);

  _data = NULL;
  _untextured_color.set(1.0, 1.0, 1.0, 1.0);
  _index = 0;
  _hpr_set = false;
  _pos_set = false;
  _hpr.set(0.0, 0.0, 0.0);
  _pos.set(0.0, 0.0, 0.0);
  _layered_type = LT_flat;
  _layer_index = 0;
  _layered_image = NULL;

  if (_filename.get_extension() == "xcf") {
    _layered_type = LT_combined;
  }
}

bool StitchImage::
has_name() const {
  return !_name.empty();
}

string StitchImage::
get_name() const {
  if (_name.empty()) {
    return _filename.get_basename_wo_extension();
  }
  return _name;
}

bool StitchImage::
has_filename() const {
  return !_filename.empty();
}

string StitchImage::
get_filename() const {
  return _filename;
}

bool StitchImage::
has_fade_filename() const {
  return !_fade_filename.empty();
}

string StitchImage::
get_fade_filename() const {
  return _fade_filename;
}

void StitchImage::
set_fade_filename(const Filename &filename) {
  _fade_filename = filename;
}

bool StitchImage::
read_file() {
  if (_data != NULL) {
    // The data is already available; no need to re-read it.
    return true;
  }
  if (!has_filename()) {
    return false;
  }
  _data = new PNMImage;
  nout << "Reading " << _filename << "\n";
  bool result = _data->read(_filename);
  if (!result) {
    delete _data;
    _data = NULL;
  }
  return result;
}

void StitchImage::
clear_file() {
  if (_data != NULL) {
    delete _data;
    _data = NULL;
  }
  if (_layered_image != NULL) {
    delete _layered_image;
    _layered_image = NULL;
  }
}

void StitchImage::
open_output_file() {
  clear_file();
  if (_layered_type == LT_flat) {
    _data = new PNMImage(_size_pixels[0], _size_pixels[1], 4);

  } else if (_layered_type == LT_combined) {
    _layered_image = new LayeredImage(_size_pixels[0], _size_pixels[1]);
  }
}

void StitchImage::
open_layer(const string &layer_name) {
  _layer_name = layer_name;

  if (_layered_type == LT_separate || _layered_type == LT_combined) {
    _data = new PNMImage(_size_pixels[0], _size_pixels[1], 4);
  }
}

bool StitchImage::
close_layer(bool nonempty) {
  bool result = true;

  if (_layered_type == LT_separate) {
    if (_data == NULL) {
      result = false;
    } else {
      if (nonempty) {
        char buff[1024];
        _layer_index++;
        sprintf(buff, _filename.c_str(), _layer_index);
        nout << "Writing layer " << _layer_name << " as " << buff << "\n";
        resize_data();
        result = _data->write(buff);
      }
    }
    clear_file();

  } else if (_layered_type == LT_combined) {
    if (_data == NULL) {
      result = false;
    } else {
      if (nonempty) {
        resize_data();
        _layered_image->add_layer(_layer_name, LVector2d(0.0, 0.0),
                                  _data);
        _data = NULL;
      }
    }
  }
  return result;
}

bool StitchImage::
close_output_file() {
  bool result = true;

  if (_layered_type == LT_separate) {

  } else if (_layered_type == LT_combined) {
    if (_layered_image == NULL) {
      result = false;
    } else {
      nout << "Writing " << _filename << "\n";
      result = _layered_image->write_file(_filename);
    }

  } else { // _layered_type == LT_flat
    if (_data == NULL) {
      result = false;
    } else {
      resize_data();
      if (has_fade_filename()) {
        fade_out();
      }
      nout << "Writing " << _filename << "\n";
      result = _data->write(_filename);
    }
  }

  clear_file();
  return result;
}


void StitchImage::
clear_transform() {
  _transform = LMatrix3d::ident_mat();
  _inv_transform = LMatrix3d::ident_mat();
  _hpr_set = false;
  _pos_set = false;
  _hpr.set(0.0, 0.0, 0.0);
  _pos.set(0.0, 0.0, 0.0);
  _morph.clear();
}

void StitchImage::
set_transform(const LMatrix4d &trans) {
  _transform = trans;
  _inv_transform = invert(trans);
  LVecBase3d scale;
  decompose_matrix(_transform, scale, _hpr, _pos);
  _hpr_set = false;
  _pos_set = false;
}

void StitchImage::
set_hpr(const LVecBase3d &hpr) {
  compose_matrix(_transform, LVecBase3d(1.0, 1.0, 1.0), hpr, _pos);
  _inv_transform.invert_from(_transform);
  _hpr_set = true;
  _hpr = hpr;
}

void StitchImage::
set_pos(const LPoint3d &pos) {
  compose_matrix(_transform, LVecBase3d(1.0, 1.0, 1.0), _hpr, pos);
  _inv_transform.invert_from(_transform);
  _pos_set = true;
  _pos = pos;
}

const LVecBase3d &StitchImage::
get_hpr() const {
  return _hpr;
}

const LPoint3d &StitchImage::
get_pos() const {
  return _pos;
}

void StitchImage::
show_points(double radius, const Colord &color) {
  _show_points = true;
  _point_radius = radius;
  _point_color = color;
}

void StitchImage::
setup_grid(int x_verts, int y_verts) {
  _x_verts = x_verts;
  _y_verts = y_verts;
}

int StitchImage::
get_x_verts() const {
  return _x_verts;
}

int StitchImage::
get_y_verts() const {
  return _y_verts;
}

LPoint2d StitchImage::
get_grid_uv(int xv, int yv) {
  return LPoint2d((double)xv / (double)(_x_verts - 1),
                  1.0 - (double)yv / (double)(_y_verts - 1));

}

LVector3d StitchImage::
get_grid_vector(int xv, int yv) {
  return extrude(get_grid_uv(xv, yv));
}

double StitchImage::
get_grid_alpha(int xv, int yv) {
  return get_alpha(get_grid_uv(xv, yv));
}

const LVecBase2d &StitchImage::
get_size_pixels() const {
  return _size_pixels;
}


////////////////////////////////////////////////////////////////////
//     Function: StitchImage::set_size_pixels
//       Access: Public
//  Description: Redefines the image to be the indicated size in
//               pixels, without changing its size in mm.
//
//               This may only be called before open_output_file() has
//               been called.
////////////////////////////////////////////////////////////////////
void StitchImage::
set_size_pixels(const LVecBase2d &size_pixels) {
  assert(_data == (PNMImage *)NULL);
  
  _size_pixels = size_pixels;
  _orig_size_pixels = _size_pixels;
  _pixels_per_mm.set((_size_pixels[0] - 1.0) / _size_mm[0],
                     (_size_pixels[1] - 1.0) / _size_mm[1]);
}

LVecBase2d StitchImage::
get_size_mm() const {
  return _size_mm;
}

LVector3d StitchImage::
extrude(const LPoint2d &point_uv) const {
  LPoint2d p = _morph.morph_out(point_uv);
  return _lens->extrude(p * _uv_to_mm, _size_mm[0]) * _transform;
}

LPoint2d StitchImage::
project(const LVector3d &vec) const {
  LPoint2d m = _lens->project(vec * _inv_transform, _size_mm[0]);
  return _morph.morph_in(m * _mm_to_uv);
}

double StitchImage::
get_alpha(const LPoint2d &point_uv) const {
  return _morph.get_alpha(point_uv);
}

void StitchImage::
reset_singularity_detected() {
  _lens->reset_singularity_detected();
}

void StitchImage::
draw_triangle(TriangleRasterizer &rast, const RasterizerVertex *v0,
              const RasterizerVertex *v1, const RasterizerVertex *v2) {
  _lens->draw_triangle(rast, _mm_to_pixels, _size_mm[0], v0, v1, v2);
}

void StitchImage::
pick_up_singularity(TriangleRasterizer &rast, StitchImage *input) {
  _lens->pick_up_singularity(rast, _mm_to_pixels, _pixels_to_mm,
                             _transform.get_upper_3(), _size_mm[0], input);
}

void StitchImage::
add_point(const string &name, const LPoint2d &pixel) {
  _points[name] = pixel * _pixels_to_uv;
}

////////////////////////////////////////////////////////////////////
//     Function: StitchImage::set_output_scale_factor
//       Access: Private
//  Description: Sets up the image to inflate its output image by the
//               given factor in each dimension during rasterization,
//               and then reduce it back to its original size just
//               before writing it out, as a poor man's pixel filter.
//
//               This may only be called before open_output_file() has
//               been called.
////////////////////////////////////////////////////////////////////
void StitchImage::
set_output_scale_factor(double factor) {
  assert(_data == (PNMImage *)NULL);

  _size_pixels = _orig_size_pixels * factor;
  _pixels_per_mm = _orig_pixels_per_mm * factor;
  setup_pixel_scales();
}


void StitchImage::
output(ostream &out) const {
  out << "image " << get_name() << ":\n"
      << get_size_pixels() << " pixels, or " << get_size_mm()
      << " mm\n";

  LVecBase3d scale, hpr, trans;
  if (decompose_matrix(_transform, scale, hpr, trans)) {
    if (!scale.almost_equal(LVecBase3d(1.0, 1.0, 1.0))) {
      out << "scale " << scale << "\n";
    }
    out << "hpr " << hpr << "\n";
    if (!trans.almost_equal(LVecBase3d::zero())) {
      out << "translate " << trans << "\n";
    }
  } else {
    out << "Invalid transform:\n";
    _transform.write(out);
  }
}

////////////////////////////////////////////////////////////////////
//     Function: StitchImage::setup_pixel_scales
//       Access: Private
//  Description: Sets up the pixel-based transform matrices, according
//               to _pixel_size and _pixels_per_mm.
////////////////////////////////////////////////////////////////////
void StitchImage::
setup_pixel_scales() {
  LVector2d pixels_per_uv(_size_pixels[0] - 1.0, _size_pixels[1] - 1.0);

  _pixels_to_uv =
    LMatrix3d::translate_mat(LVector2d(0.0, -pixels_per_uv[1])) *
    LMatrix3d::scale_mat(1.0 / pixels_per_uv[0], -1.0 / pixels_per_uv[1]);

  _uv_to_pixels =
    LMatrix3d::scale_mat(pixels_per_uv[0], -pixels_per_uv[1]) *
    LMatrix3d::translate_mat(LVector2d(0.0, pixels_per_uv[1]));

  _pixels_to_mm = _pixels_to_uv * _uv_to_mm;
  _mm_to_pixels = _mm_to_uv * _uv_to_pixels;
}

////////////////////////////////////////////////////////////////////
//     Function: StitchImage::resize_data
//       Access: Private
//  Description: Resizes the image generated in _data back to
//               _orig_size_pixels, if it has been generated larger,
//               in preparation to writing it out.
////////////////////////////////////////////////////////////////////
void StitchImage::
resize_data() {
  assert(_data != (PNMImage *)NULL);
  if (_orig_size_pixels == _size_pixels) {
    return;
  }

  PNMImage *reduced = 
    new PNMImage(_orig_size_pixels[0], _orig_size_pixels[1], 
                 _data->get_color_type());

  cerr << "Filtering to " << reduced->get_x_size() << " by " 
       << reduced->get_y_size() << "\n";

  reduced->box_filter_from(0.5, *_data);
  delete _data;
  _data = reduced;
}

////////////////////////////////////////////////////////////////////
//     Function: StitchImage::fade_out
//       Access: Private
//  Description: Fades out the image (presumably at the edges) by
//               multiplying each pixel by the corresponding pixel in
//               the named fade image.
//
//               This is generally useful for generating output images
//               that are intended to overlap meaningfully, for
//               instance when projected through overlapping
//               projectors.
////////////////////////////////////////////////////////////////////
void StitchImage::
fade_out() {
  assert(_data != (PNMImage *)NULL);

  const PNMImage *fade = 
    FadeImagePool::get_image(get_fade_filename(),
                             _data->get_x_size(), _data->get_y_size());
  if (fade == (PNMImage *)NULL) {
    return;
  }

  // Now apply the fade factor to darken each of our pixels.
  cerr << "Applying fade image.\n";
  for (int y = 0; y < _data->get_y_size(); y++) {
    for (int x = 0; x < _data->get_x_size(); x++) {
      // Because we know the fade image is guaranteed to be grayscale,
      // we can simply ask for its gray component rather than
      // computing its bright component.
      double bright = fade->get_gray(x, y);
      _data->set_red_val(x, y, (xelval)_data->get_red_val(x, y) * bright);
      _data->set_green_val(x, y, (xelval)_data->get_green_val(x, y) * bright);
      _data->set_blue_val(x, y, (xelval)_data->get_blue_val(x, y) * bright);
    }
  }
}
