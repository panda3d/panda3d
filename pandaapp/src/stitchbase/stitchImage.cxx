// Filename: stitchImage.cxx
// Created by:  drose (04Nov99)
// 
////////////////////////////////////////////////////////////////////

#include "stitchImage.h"
#include "stitchLens.h"
#include "layeredImage.h"

#include <compose_matrix.h>
#include <rotate_to.h>

StitchImage::
StitchImage(const string &name, const string &filename,
	    StitchLens *lens, const LVecBase2d &size_pixels,
	    const LVecBase2d &pixels_per_mm,
	    const LVecBase2d &film_offset_mm) :
  _lens(lens),
  _size_pixels(size_pixels),
  _pixels_per_mm(pixels_per_mm),
  _film_offset_mm(film_offset_mm),
  _rotate(LMatrix3d::ident_mat()),
  _inv_rotate(LMatrix3d::ident_mat()),
  _filename(filename),
  _name(name)
{
  _size_mm.set((_size_pixels[0] - 1.0) / _pixels_per_mm[0],
	       (_size_pixels[1] - 1.0) / _pixels_per_mm[1]);

  // There are several coordinate systems to talk about points on the
  // image.

  // UV's are used for doing most operations.  They range from (0, 0)
  // at the lower-left corner to (1, 1) at the upper-right.
  
  // Pixels are used when interfacing with the user.  They range from
  // (0, 0) at the upper-left corner to (_size_pixels[0] - 1,
  // _size_pixels[1] - 1) at the lower-right.

  // Millimeters are used when interfacing with the lens.  They start
  // at -film_offset_mm at the center, and range from
  // -film_offset_mm-size_mm at the lower-left, to
  // -film_offset_mm+size_mm at the upper-right.

  LVector2d pixels_per_uv(_size_pixels[0] - 1.0, _size_pixels[1] - 1.0);

  _pixels_to_uv = 
    LMatrix3d::translate_mat(LVector2d(0.0, -pixels_per_uv[1])) *
    LMatrix3d::scale_mat(1.0 / pixels_per_uv[0], -1.0 / pixels_per_uv[1]);

  _uv_to_pixels =
    LMatrix3d::scale_mat(pixels_per_uv[0], -pixels_per_uv[1]) *
    LMatrix3d::translate_mat(LVector2d(0.0, pixels_per_uv[1]));

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
    LMatrix3d::translate_mat(-_film_offset_mm);

  _mm_to_uv =
    LMatrix3d::translate_mat(_film_offset_mm) *
    LMatrix3d::scale_mat(1.0 / mm_per_uv[0], 1.0 / mm_per_uv[1]) *
    LMatrix3d::translate_mat(LVector2d(0.5, 0.5) + _film_offset_mm);

  _pixels_to_mm = _pixels_to_uv * _uv_to_mm;
  _mm_to_pixels = _mm_to_uv * _uv_to_pixels;

  _show_points = false;
  setup_grid(2, 2);

  _data = NULL;
  _untextured_color.set(1.0, 1.0, 1.0, 1.0);
  _index = 0;
  _hpr_set = false;
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
read_file() {
  if (_data != NULL) {
    delete _data;
    _data = NULL;
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
	result = _data->write(buff);
      }
    }
    clear_file();

  } else if (_layered_type == LT_combined) {
    if (_data == NULL) {
      result = false;
    } else {
      if (nonempty) {
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
      nout << "Writing " << _filename << "\n";
      result = _data->write(_filename);
    }
  }

  clear_file();
  return result;
}

void StitchImage::
clear_transform() {
  _rotate = LMatrix3d::ident_mat();
  _inv_rotate = LMatrix3d::ident_mat();
  _morph.clear();
}

void StitchImage::
set_transform(const LMatrix3d &rot) {
  _rotate = rot;
  _inv_rotate = invert(rot);
}

void StitchImage::
set_hpr(const LVecBase3d &hpr) {
  compose_matrix(_rotate, LVecBase3d(1.0, 1.0, 1.0), hpr);
  _inv_rotate = invert(_rotate);
  _hpr_set = true;
  _hpr = hpr;
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

LVecBase2d StitchImage::
get_size_mm() const {
  return _size_mm;
}

LVector3d StitchImage::
extrude(const LPoint2d &point_uv) const {
  LPoint2d p = _morph.morph_out(point_uv);
  return _lens->extrude(p * _uv_to_mm, _size_mm[0]) * _rotate;
}

LPoint2d StitchImage::
project(const LVector3d &vec) const {
  LPoint2d m = _lens->project(vec * _inv_rotate, _size_mm[0]);
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
			     _rotate, _size_mm[0], input);
}

void StitchImage::
add_point(const string &name, const LPoint2d &pixel) {
  _points[name] = pixel * _pixels_to_uv;
}


void StitchImage::
output(ostream &out) const {
  out << "image " << get_name() << ":\n"
      << get_size_pixels() << " pixels, or " << get_size_mm()
      << " mm\n";

  LVecBase3d scale, hpr;
  if (decompose_matrix(_rotate, scale, hpr)) {
    out << "rotate " << hpr << "\n";
  } else {
    out << "Invalid rotation transform:\n";
    _rotate.write(out);
  }
}

