// Filename: stitchImageRasterizer.cxx
// Created by:  drose (06Nov99)
// 
////////////////////////////////////////////////////////////////////

#include "stitchImageRasterizer.h"
#include "triangleRasterizer.h"
#include "stitchImage.h"
#include "stitcher.h"

#include <pnmImage.h>

StitchImageRasterizer::
StitchImageRasterizer() {
  _filter_output = true;
}


void StitchImageRasterizer::
add_input_image(StitchImage *image) {
  _input_images.push_back(image);
}

void StitchImageRasterizer::
add_output_image(StitchImage *image) {
  _output_images.push_back(image);
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
  rast._filter_output = _filter_output;
  rast._untextured_color = input->_untextured_color;

  int x_verts = input->get_x_verts();
  int y_verts = input->get_y_verts();

  // Build up the table of verts.
  typedef vector<RasterizerVertex> VRow;
  typedef vector<VRow> VTable;
  VTable _table(x_verts, VRow());

  int xi, yi;
  for (xi = 0; xi < x_verts; xi++) {
    _table[xi] = VRow(y_verts, RasterizerVertex());

    for (yi = 0; yi < y_verts; yi++) {
      LVector3d space = input->get_grid_vector(xi, yi);
      double alpha = input->get_grid_alpha(xi, yi);
      LPoint2d to = output->project(space);
      LPoint2d from = input->get_grid_uv(xi, yi);

      _table[xi][yi]._p = to * output->_uv_to_pixels;
      _table[xi][yi]._uv = from;
      _table[xi][yi]._space = space * output->_inv_rotate;
      _table[xi][yi]._alpha = alpha;

      _table[xi][yi]._space = normalize(_table[xi][yi]._space);

      // We assign one bit for each quadrant the vertex may be out of
      // bounds.  If all three vertices of a triangle are out in the
      // same quadrant, then the entire triangle is out of bounds.
      _table[xi][yi]._visibility =
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

  // Now draw all of the triangles, top-to-bottom.
  output->reset_singularity_detected();

  for (yi = 0; yi < y_verts - 1; yi++) {
    for (xi = 0; xi < x_verts - 1; xi++) {
      output->draw_triangle(rast,
                            &_table[xi][yi],
                            &_table[xi][yi + 1],
                            &_table[xi + 1][yi + 1]);
      output->draw_triangle(rast,
                            &_table[xi][yi],
                            &_table[xi + 1][yi + 1],
                            &_table[xi + 1][yi]);
    }
  }
  output->pick_up_singularity(rast, input);
  output->close_layer(rast._read_input);

  input->clear_file();
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

