// Filename: stitchImageRasterizer.cxx
// Created by:  drose (06Nov99)
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

#include "stitchImageRasterizer.h"
#include "triangleRasterizer.h"
#include "stitchImage.h"
#include "stitcher.h"

#include "pnmImage.h"

StitchImageRasterizer::
StitchImageRasterizer() {
  _filter_factor = 1.0;
  _got_output_size = false;
}


void StitchImageRasterizer::
add_input_image(StitchImage *image) {
  if (has_input_name(image->get_name())) {
    cerr << "Input image " << image->get_name() << "\n";
    _input_images.push_back(image);
  }
}

void StitchImageRasterizer::
add_output_image(StitchImage *image) {
  if (has_output_name(image->get_name())) {
    cerr << "Output image " << image->get_name() << "\n";
    if (_got_output_size) {
      image->set_size_pixels(LPoint2d(_output_xsize, _output_ysize));
    }
    image->set_output_scale_factor(_filter_factor);
    _output_images.push_back(image);
  }
}

void StitchImageRasterizer::
add_stitcher(Stitcher *stitcher) {
  _stitchers.push_back(stitcher);
}

void StitchImageRasterizer::
execute() {
  Images::iterator oi;
  for (oi = _output_images.begin(); oi != _output_images.end(); ++oi) {
    StitchImage *output = (*oi);
    if (!output->has_filename()) {
      nout << "Output image has no filename; cannot generate.\n";
    } else {
      nout << "Generating " << output->get_name() << "\n";
      output->open_output_file();

      Images::const_iterator ii;
      for (ii = _input_images.begin(); ii != _input_images.end(); ++ii) {
        StitchImage *input = (*ii);
        draw_image(output, input);
      }

      output->open_layer("points");
      bool shown_points = false;
      Stitchers::const_iterator si;
      for (si = _stitchers.begin(); si != _stitchers.end(); ++si) {
        Stitcher *stitcher = (*si);
        if (stitcher->_show_points && !stitcher->_loose_points.empty()) {
          draw_points(output, stitcher, stitcher->_point_color,
                      stitcher->_point_radius);
          shown_points = true;
        }
      }
      for (ii = _input_images.begin(); ii != _input_images.end(); ++ii) {
        StitchImage *input = (*ii);
        if (input->_show_points && !input->_points.empty()) {
          draw_points(output, input, input->_point_color,
                      input->_point_radius);
          shown_points = true;
        }
      }
      output->close_layer(shown_points);

      if (!output->close_output_file()) {
        nout << "Error in writing.\n";
      }
    }
  }
}

////////////////////////////////////////////////////////////////////
//     Function: StitchImageRasterizer::add_input_name
//       Access: Public
//  Description: Adds the indicated name to the set of input image
//               names that will be generated.  The name may include
//               globbing symbols.  If no names are added, all input
//               images will be generated.
////////////////////////////////////////////////////////////////////
void StitchImageRasterizer::
add_input_name(const string &name) {
  _input_names.push_back(GlobPattern(name));
}

////////////////////////////////////////////////////////////////////
//     Function: StitchImageRasterizer::has_input_name
//       Access: Public
//  Description: Returns true if the indicated input image name is
//               one that should be generated.  This will be true if
//               this name has been added via add_input_name(), or if
//               no names at all have been added.
////////////////////////////////////////////////////////////////////
bool StitchImageRasterizer::
has_input_name(const string &name) const {
  if (_input_names.empty()) {
    // If no names have been added, all names are good.
    return true;
  }

  // Otherwise, the name is good only if it matches a name on the list.
  Names::const_iterator ni;
  for (ni = _input_names.begin(); ni != _input_names.end(); ++ni) {
    if ((*ni).matches(name)) {
      return true;
    }
  }

  return false;
}

////////////////////////////////////////////////////////////////////
//     Function: StitchImageRasterizer::add_output_name
//       Access: Public
//  Description: Adds the indicated name to the set of output image
//               names that will be generated.  The name may include
//               globbing symbols.  If no names are added, all output
//               images will be generated.
////////////////////////////////////////////////////////////////////
void StitchImageRasterizer::
add_output_name(const string &name) {
  _output_names.push_back(GlobPattern(name));
}

////////////////////////////////////////////////////////////////////
//     Function: StitchImageRasterizer::has_output_name
//       Access: Public
//  Description: Returns true if the indicated output image name is
//               one that should be generated.  This will be true if
//               this name has been added via add_output_name(), or if
//               no names at all have been added.
////////////////////////////////////////////////////////////////////
bool StitchImageRasterizer::
has_output_name(const string &name) const {
  if (_output_names.empty()) {
    // If no names have been added, all names are good.
    return true;
  }

  // Otherwise, the name is good only if it matches a name on the list.
  Names::const_iterator ni;
  for (ni = _output_names.begin(); ni != _output_names.end(); ++ni) {
    if ((*ni).matches(name)) {
      return true;
    }
  }

  return false;
}

////////////////////////////////////////////////////////////////////
//     Function: StitchImageRasterizer::set_filter_factor
//       Access: Public
//  Description: Sets the factor by which the output images will be
//               scaled internally in each dimension while generating
//               them.  They will be reduced again to their original
//               size for writing the images out.  This provides a
//               simple kind of pixel filtering.
////////////////////////////////////////////////////////////////////
void StitchImageRasterizer::
set_filter_factor(double filter_factor) {
  _filter_factor = filter_factor;
}

////////////////////////////////////////////////////////////////////
//     Function: StitchImageRasterizer::set_output_size
//       Access: Public
//  Description: Overrides the image size each output image specifies
//               with the indicated size.
////////////////////////////////////////////////////////////////////
void StitchImageRasterizer::
set_output_size(int xsize, int ysize) {
  _output_xsize = xsize;
  _output_ysize = ysize;
  _got_output_size = true;
}

void StitchImageRasterizer::
draw_points(StitchImage *output, StitchImage *input,
            const Colord &color, double radius) {
  StitchImage::Points::const_iterator pi;
  for (pi = input->_points.begin(); pi != input->_points.end(); ++pi) {
    LPoint2d to = output->project(input->extrude((*pi).second));
    draw_spot(output, to * output->_uv_to_pixels, color, radius);
  }
}

void StitchImageRasterizer::
draw_points(StitchImage *output, Stitcher *input,
            const Colord &color, double radius) {
  Stitcher::LoosePoints::const_iterator pi;
  for (pi = input->_loose_points.begin();
       pi != input->_loose_points.end(); ++pi) {
    LPoint2d to = output->project((*pi)->_space);
    draw_spot(output, to * output->_uv_to_pixels, color, radius);
  }
}


void StitchImageRasterizer::
draw_image(StitchImage *output, StitchImage *input) {
  nout << "Rasterizing " << input->get_name() << "\n";
  output->open_layer(input->get_name());

  TriangleRasterizer rast;
  rast._output = output->_data;
  rast._input = input;
  rast._filter_output = false;
  rast._untextured_color = input->_untextured_color;

  int x_verts = input->get_x_verts();
  int y_verts = input->get_y_verts();

  // Build up the table of verts.
  typedef vector<RasterizerVertex> VRow;
  typedef vector<VRow> VTable;
  VTable table(x_verts, VRow());

  int xi, yi;
  for (xi = 0; xi < x_verts; xi++) {
    table[xi] = VRow(y_verts, RasterizerVertex());

    for (yi = 0; yi < y_verts; yi++) {
      LVector3d space = input->get_grid_vector(xi, yi);
      double alpha = input->get_grid_alpha(xi, yi);
      LPoint3d point;
      if (!_screen->intersect(point, input->get_pos(), space)) {
        // No intersection with the screen.  What should we do now?
        LPoint2d from = input->get_grid_uv(xi, yi);

        table[xi][yi]._p.set(0.0, 0.0);
        table[xi][yi]._uv = from;
        table[xi][yi]._space = point * output->_inv_transform;
        table[xi][yi]._alpha = alpha;
        
        table[xi][yi]._space = normalize(table[xi][yi]._space);

        // Tag any triangle including this vertex as totally invalid.
        table[xi][yi]._visibility = -1;

      } else {
        LPoint2d to = output->project(point - output->get_pos());
        LPoint2d from = input->get_grid_uv(xi, yi);
        
        table[xi][yi]._p = to * output->_uv_to_pixels;
        table[xi][yi]._uv = from;
        table[xi][yi]._space = point * output->_inv_transform;
        table[xi][yi]._alpha = alpha;
        
        table[xi][yi]._space = normalize(table[xi][yi]._space);

        // We assign one bit for each quadrant the vertex may be out of
        // bounds.  If all three vertices of a triangle are out in the
        // same quadrant, then the entire triangle is out of bounds.
        table[xi][yi]._visibility =
          ((to[0] < 0.0) |
           ((to[0] > 1.0) << 1) |
           ((to[1] < 0.0) << 2) |
           ((to[1] > 1.0) << 3) |
           ((from[0] < 0.0) << 4) |
           ((from[0] > 1.0) << 5) |
           ((from[1] < 0.0) << 6) |
           ((from[1] > 1.0) << 7));
      }
    }
  }

  // Now draw all of the triangles, top-to-bottom.
  output->reset_singularity_detected();

  for (yi = 0; yi < y_verts - 1; yi++) {
    for (xi = 0; xi < x_verts - 1; xi++) {
      output->draw_triangle(rast,
                            &table[xi][yi],
                            &table[xi][yi + 1],
                            &table[xi + 1][yi + 1]);
      output->draw_triangle(rast,
                            &table[xi][yi],
                            &table[xi + 1][yi + 1],
                            &table[xi + 1][yi]);
    }
  }
  output->pick_up_singularity(rast, input);
  output->close_layer(rast._read_input);

  // We don't necessarily want to clear the input file now.  Although
  // that would reduce memory consumption, maybe we've got plenty of
  // memory and we're more concerned with reducing load time.
  //  input->clear_file();
}


void StitchImageRasterizer::
draw_spot(StitchImage *output,
          const LPoint2d pixel_center, const Colord &color, double radius) {
  LPoint2d minp = pixel_center - LPoint2d(radius, radius);
  LPoint2d maxp = pixel_center + LPoint2d(radius, radius);

  int min_x = (int)floor(minp[0]);
  int max_x = (int)ceil(maxp[0]);

  int min_y = (int)floor(minp[1]);
  int max_y = (int)ceil(maxp[1]);

  double r2 = radius * radius;

  for (int yi = min_y; yi <= max_y; yi++) {
    if (yi >= 0 && yi < output->_data->get_y_size()) {
      for (int xi = min_x; xi <= max_x; xi++) {
        if (xi >= 0 && xi < output->_data->get_x_size()) {
          // Check the coverage of the four points around the pixel, and
          // the pixel center.
          LPoint2d ul = pixel_center - LPoint2d(xi - 0.5, yi - 0.5);
          LPoint2d ll = pixel_center - LPoint2d(xi - 0.5, yi + 0.5);
          LPoint2d ur = pixel_center - LPoint2d(xi + 0.5, yi - 0.5);
          LPoint2d lr = pixel_center - LPoint2d(xi + 0.5, yi + 0.5);
          LPoint2d pc = pixel_center - LPoint2d(xi, yi);

          // Net coverage.
          int coverage =
            (dot(ul, ul) <= r2) +
            (dot(ll, ll) <= r2) +
            (dot(ur, ur) <= r2) +
            (dot(lr, lr) <= r2) +
            (dot(pc, pc) <= r2);

          if (coverage != 0) {
            output->_data->blend(xi, yi, color[0], color[1], color[2],
                                 color[3] * (double)coverage / 5.0);
          }
        }
      }
    }
  }
}

