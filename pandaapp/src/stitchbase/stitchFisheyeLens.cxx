// Filename: stitchFisheyeLens.cxx
// Created by:  drose (09Nov99)
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

#include "stitchFisheyeLens.h"
#include "stitchImage.h"
#include "stitchCommand.h"
#include "triangleRasterizer.h"
#include "triangle.h"

#include "pandatoolbase.h"
#include <deg_2_rad.h>

#include <math.h>

// This is the focal-length constant for fisheye lenses.  The focal
// length of a fisheye lens relates to its fov by the equation:

//   w = Fd/k

// Where w is the width of the negative, F is the focal length, and d
// is the total field of view in degrees.

// k is chosen here by simple examination of a couple of actual lenses
// for 35mm film.  Don't know how well this extends to other lenses
// and other negative sizes.

static const double fisheye_k = 60.0;

StitchFisheyeLens::
StitchFisheyeLens() {
}

double StitchFisheyeLens::
get_focal_length(double width_mm) const {
  if (_flags & F_focal_length) {
    return _focal_length;
  }
  if (_flags & F_fov) {
    return width_mm * fisheye_k / _fov;
  }
  return 0.0;
}

double StitchFisheyeLens::
get_hfov(double width_mm) const {
  if (_flags & F_fov) {
    return _fov;
  }
  if (_flags & F_focal_length) {
    return width_mm * fisheye_k / _focal_length;
  }
  return 0.0;
}

LVector3d StitchFisheyeLens::
extrude(const LPoint2d &point_mm, double width_mm) const {
  // This operation is essentially a conversion from Cartesian to
  // polar coordinates.

  // First, get the vector from the center of the film to the point,
  // and normalize it.
  LVector2d v2 = point_mm;

  double r = length(v2);
  if (r == 0.0) {
    // Special case: directly forward.
    return LVector3d::forward();
  }

  v2 /= r;

  // Now get the point r units around the circle in the YZ plane.
  double dist = r * fisheye_k / get_focal_length(width_mm);
  LVector3d p(0.0, cos(deg_2_rad(dist)), sin(deg_2_rad(dist)));

  // And rotate this point around the Y axis.
  LVector3d result = LVector3d::rfu(p[0]*v2[1] + p[2]*v2[0],
                                    p[1],
                                    p[2]*v2[1] - p[0]*v2[0]);
  return result;
}

LPoint2d StitchFisheyeLens::
project(const LVector3d &vec, double width_mm) const {
  // A fisheye lens projection has the property that the distance from
  // the center point to any other point on the projection is
  // proportional to the actual distance on the sphere along the great
  // circle.  Also, the angle to the point on the projection is equal
  // to the angle to the point on the sphere.

  // First, discard the distance by normalizing the vector.
  LVector3d v2 = normalize(vec * LMatrix4d::convert_mat(CS_default,
                                                        CS_zup_right));

  // Now, project the point into the XZ plane and measure its angle
  // to the Z axis.  This is the same angle it will have to the
  // vertical axis on the film.
  LVector2d y(v2[0], v2[2]);
  y = normalize(y);

  if (y == LVector2d(0.0, 0.0)) {
    // Special case.  This point is either directly ahead or directly
    // behind.
    return LPoint2d(0.0, 0.0);
  }

  // Now bring the vector into the YZ plane by rotating about the Y
  // axis.
  LVector2d x(v2[1], v2[0]*y[0]+v2[2]*y[1]);
  x = normalize(x);

  // Now the angle of x to the forward vector represents the distance
  // along the great circle to the point.
  double r = 90.0 - rad_2_deg(atan2(x[0], x[1]));

  return y * (r * get_focal_length(width_mm) / fisheye_k);
}

void StitchFisheyeLens::
draw_triangle(TriangleRasterizer &rast, const LMatrix3d &,
              double, const RasterizerVertex *v0,
              const RasterizerVertex *v1, const RasterizerVertex *v2) {
  // A fisheye lens has a singularity at 180 degrees--this point maps
  // to the entire outer rim of the circle.  Near this singularity,
  // small distances in space map to very large distances on the film,
  // meaning that our use of triangles to approximate curvature
  // becomes very bad near the singularity.  Furthermore, triangles
  // that cross the singularity will be incorrectly drawn across the
  // entire image on the film.

  // We resolve this by simply not drawing any triangles that come
  // with a user-specified angle (the _singularity_tolerance) from the
  // singularity point.


  // Determine which quadrant each of the vertices is in.  The
  // triangle crosses the singularity if all vertices' y coordinate is
  // negative, and if the projection of the triangle into the x, z
  // plane intersects the origin.  It comes within
  // _singularity_tolerance of the singularity if the projection into
  // x, z intersects a circle about the origin with radius
  // _singularity_radius.

  if (dot(v0->_space, LVector3d::forward()) < 0.0 &&
      dot(v1->_space, LVector3d::forward()) < 0.0 &&
      dot(v2->_space, LVector3d::forward()) < 0.0) {
    LPoint2d xz0(dot(v0->_space, LVector3d::right()),
                 dot(v0->_space, LVector3d::up()));
    LPoint2d xz1(dot(v1->_space, LVector3d::right()),
                 dot(v1->_space, LVector3d::up()));
    LPoint2d xz2(dot(v2->_space, LVector3d::right()),
                 dot(v2->_space, LVector3d::up()));

    // This projection will reverse the vertex order.
    if (triangle_contains_circle(LPoint2d(0.0, 0.0),
                                 _singularity_radius,
                                 xz0, xz2, xz1)) {
      // The triangle does cross the singularity!  Reject it.
      /*
      nout << "Rejecting:\n"
           << "   " << v0->_space << "\n"
           << "   " << v1->_space << "\n"
           << "   " << v2->_space << "\n\n";
      */
      _singularity_detected = 1;
      return;
    }
  }

  rast.draw_triangle(v0, v1, v2);
}

void StitchFisheyeLens::
pick_up_singularity(TriangleRasterizer &rast,
                    const LMatrix3d &mm_to_pixels,
                    const LMatrix3d &pixels_to_mm,
                    const LMatrix3d &rotate,
                    double width_mm, StitchImage *input) {
  if (_singularity_detected) {
    nout << "Picking up singularity\n";

    // We will be drawing all the pixels between the circle
    // representing points 180 degrees from forward, and the circle
    // represent points (180 - _singularity_tolerance * 2) degrees
    // from forward.

    double outer_mm =
      (180 * get_focal_length(width_mm) / fisheye_k);
    double inner_mm =
      ((180 - _singularity_tolerance * 2) * get_focal_length(width_mm) / fisheye_k);

    int xsize = rast._output->get_x_size();
    int ysize = rast._output->get_y_size();

    LPoint2d py = LPoint2d(0.0, outer_mm) * mm_to_pixels;
    int top_y = max((int)floor(py[1]), 0);
    py = LPoint2d(0.0, -outer_mm) * mm_to_pixels;
    int bot_y = min((int)ceil(py[1]), ysize - 1);

    py = LPoint2d(0.0, inner_mm) * mm_to_pixels;
    int inner_top_y = (int)floor(py[1]);
    py = LPoint2d(0.0, -inner_mm) * mm_to_pixels;
    int inner_bot_y = (int)ceil(py[1]);

    RasterizerVertex v0;
    v0._p.set(0.0, 0.0);
    v0._uv.set(0.0, 0.0);
    v0._space.set(0.0, 0.0, 0.0);
    v0._alpha = 1.0;
    v0._visibility = 0;

    int xi, yi;
    for (yi = top_y; yi <= bot_y; yi++) {
      int left_x_1, right_x_1;
      int left_x_2, right_x_2;

      // Where are the left and right X pixels at this slice?
      if (yi <= inner_top_y) {
        // This is the top slice of the ring: between the top of the
        // outer circle and the top of the inner circle.

        LPoint2d pmm = LPoint2d(0.0, yi) * pixels_to_mm;
        pmm[0] = sqrt(outer_mm * outer_mm - pmm[1] * pmm[1]);

        LPoint2d px = LPoint2d(-pmm[0], pmm[1]) * mm_to_pixels;
        left_x_1 = max((int)floor(px[0]), 0);
        px = LPoint2d(pmm[0], pmm[1]) * mm_to_pixels;
        right_x_1 = min((int)ceil(px[0]), xsize - 1);

        right_x_2 = right_x_1;
        left_x_2 = right_x_2 + 1;

      } else if (yi < inner_bot_y) {
        // This is the inner section: within the inner circle area.
        // We have both a left and a right section here.

        LPoint2d pmm = LPoint2d(0.0, yi) * pixels_to_mm;
        pmm[0] = sqrt(outer_mm * outer_mm - pmm[1] * pmm[1]);

        LPoint2d px = LPoint2d(-pmm[0], pmm[1]) * mm_to_pixels;
        left_x_1 = max((int)floor(px[0]), 0);
        px = LPoint2d(pmm[0], pmm[1]) * mm_to_pixels;
        right_x_2 = min((int)ceil(px[0]), xsize - 1);

        pmm[0] = sqrt(inner_mm * inner_mm - pmm[1] * pmm[1]);
        px = LPoint2d(-pmm[0], pmm[1]) * mm_to_pixels;
        right_x_1 = max((int)floor(px[0]), 0);
        px = LPoint2d(pmm[0], pmm[1]) * mm_to_pixels;
        left_x_2 = min((int)ceil(px[0]), xsize - 1);

      } else {
        // This is the bottom slice of the ring: between the bottom of
        // the inner circle and the bottom of the outer circle.

        LPoint2d pmm = LPoint2d(0.0, yi) * pixels_to_mm;
        pmm[0] = sqrt(outer_mm * outer_mm - pmm[1] * pmm[1]);

        LPoint2d px = LPoint2d(-pmm[0], pmm[1]) * mm_to_pixels;
        left_x_1 = max((int)floor(px[0]), 0);
        px = LPoint2d(pmm[0], pmm[1]) * mm_to_pixels;
        right_x_1 = min((int)ceil(px[0]), xsize - 1);

        right_x_2 = right_x_1;
        left_x_2 = right_x_2 + 1;

      }

      // Project xi point 1 to determine the radius.
      v0._p.set(left_x_1 + 1, yi);
      v0._space = extrude(v0._p * pixels_to_mm, width_mm) * rotate;
      v0._uv = input->project(v0._space);

      for (xi = left_x_1; xi <= right_x_1; xi++) {
        double last_u = v0._uv[0];

        v0._p.set(xi, yi);
        v0._space = extrude(v0._p * pixels_to_mm, width_mm) * rotate;
        v0._uv = input->project(v0._space);
        rast.draw_pixel(&v0, fabs(v0._uv[0] - last_u));
      }

      // Project xi point 1 to determine the radius.
      v0._p.set(left_x_2 + 1, yi);
      v0._space = extrude(v0._p * pixels_to_mm, width_mm) * rotate;
      v0._uv = input->project(v0._space);

      for (xi = left_x_2; xi <= right_x_2; xi++) {
        double last_u = v0._uv[0];

        v0._p.set(xi, yi);
        v0._space = extrude(v0._p * pixels_to_mm, width_mm) * rotate;
        v0._uv = input->project(v0._space);
        rast.draw_pixel(&v0, fabs(v0._uv[0] - last_u));
      }
    }
  }
}

void StitchFisheyeLens::
make_lens_command(StitchCommand *parent) {
  StitchCommand *lens_cmd = new StitchCommand(parent, StitchCommand::C_lens);
  StitchCommand *cmd;
  cmd = new StitchCommand(lens_cmd, StitchCommand::C_fisheye);
  if (_flags & F_focal_length) {
    cmd = new StitchCommand(lens_cmd, StitchCommand::C_focal_length);
    cmd->set_length(_focal_length);
  }
  if (_flags & F_fov) {
    cmd = new StitchCommand(lens_cmd, StitchCommand::C_fov);
    cmd->set_number(_fov);
  }
}
