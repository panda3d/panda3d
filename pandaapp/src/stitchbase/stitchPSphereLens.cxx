// Filename: stitchPSphereLens.cxx
// Created by:  drose (16Nov99)
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

#include "stitchPSphereLens.h"
#include "stitchImage.h"
#include "stitchCommand.h"
#include "triangleRasterizer.h"
#include "triangle.h"

#include "pandatoolbase.h"
#include <deg_2_rad.h>

#include <math.h>

// This is the focal-length constant for fisheye lenses.  See
// stitchFisheyeLens.h.
static const double psphere_k = 60.0;

StitchPSphereLens::
StitchPSphereLens() {
}

double StitchPSphereLens::
get_focal_length(double width_mm) const {
  if (_flags & F_focal_length) {
    return _focal_length;
  }
  if (_flags & F_fov) {
    return width_mm * psphere_k / _fov;
  }
  return 0.0;
}

double StitchPSphereLens::
get_hfov(double width_mm) const {
  if (_flags & F_fov) {
    return _fov;
  }
  if (_flags & F_focal_length) {
    return width_mm * psphere_k / _focal_length;
  }
  return 0.0;
}

LVector3d StitchPSphereLens::
extrude(const LPoint2d &point_mm, double width_mm) const {
  LVector2d v2 = point_mm;

  double fl = get_focal_length(width_mm);
  return LVector3d::forward() *
    LMatrix3d::rotate_mat(v2[1] * psphere_k / fl, LVector3d::right()) *
    LMatrix3d::rotate_mat(-v2[0] * psphere_k / fl, LVector3d::up());
}


LPoint2d StitchPSphereLens::
project(const LVector3d &vec, double width_mm) const {
  // A PSphere lens is a toroidal lens. It is independently curved in
  // the horizontal and vertical directions.

  LVector3d v3 = vec * LMatrix4d::convert_mat(CS_default, CS_zup_right);

  // To compute the x position on the frame, we only need to consider
  // the angle of the vector about the Z axis.  Project the vector
  // into the XY plane to do this.

  LVector2d xy(v3[0], v3[1]);

  // The x position is the angle about the Z axis.
  double x =
    rad_2_deg(atan2(xy[0], xy[1])) * get_focal_length(width_mm) / psphere_k;

  // Unroll the Z angle, and the y position is the angle about the X
  // axis.
  xy = normalize(xy);
  LVector2d yz(v3[0]*xy[0] + v3[1]*xy[1], v3[2]);
  double y =
    rad_2_deg(atan2(yz[1], yz[0])) * get_focal_length(width_mm) / psphere_k;

  return LPoint2d(x, y);
}

LPoint2d StitchPSphereLens::
project_left(const LVector3d &vec, double width_mm) const {
  // This is just like project(), except that if the vertex extends
  // below -180 degrees, it remains on the left side of the film
  // (instead of wrapping around to the right side).

  LVector3d v3 = vec * LMatrix4d::convert_mat(CS_default, CS_zup_right);
  LVector2d xy(v3[0], v3[1]);
  double x =
    (rad_2_deg(atan2(-xy[0], -xy[1])) - 180.0) *
    get_focal_length(width_mm) / psphere_k;

  xy = normalize(xy);
  LVector2d yz(v3[0]*xy[0] + v3[1]*xy[1], v3[2]);
  double y =
    rad_2_deg(atan2(yz[1], yz[0])) * get_focal_length(width_mm) / psphere_k;

  return LPoint2d(x, y);
}

LPoint2d StitchPSphereLens::
project_right(const LVector3d &vec, double width_mm) const {
  // This is just like project(), except that if the vertex extends
  // above 180 degrees, it remains on the right side of the film
  // (instead of wrapping around to the left side).

  LVector3d v3 = vec * LMatrix4d::convert_mat(CS_default, CS_zup_right);
  LVector2d xy(v3[0], v3[1]);
  double x =
    (rad_2_deg(atan2(-xy[0], -xy[1])) + 180.0) *
    get_focal_length(width_mm) / psphere_k;

  xy = normalize(xy);
  LVector2d yz(v3[0]*xy[0] + v3[1]*xy[1], v3[2]);
  double y =
    rad_2_deg(atan2(yz[1], yz[0])) * get_focal_length(width_mm) / psphere_k;

  return LPoint2d(x, y);
}

void StitchPSphereLens::
draw_triangle(TriangleRasterizer &rast, const LMatrix3d &mm_to_pixels,
              double width_mm, const RasterizerVertex *v0,
              const RasterizerVertex *v1, const RasterizerVertex *v2) {
  // A PSphere lens has two singularities, at the north and south
  // poles, as well as a seam at 180 and -180 degrees.

  // First, we reject any triangles within _singularity_tolerance of
  // either pole, similar to the fisheye lens.

  LVector2d xy0(dot(v0->_space, LVector3d::right()),
                dot(v0->_space, LVector3d::forward()));
  LVector2d xy1(dot(v1->_space, LVector3d::right()),
                dot(v1->_space, LVector3d::forward()));
  LVector2d xy2(dot(v2->_space, LVector3d::right()),
                dot(v2->_space, LVector3d::forward()));

  double z0 = dot(v0->_space, LVector3d::up());
  double z1 = dot(v0->_space, LVector3d::up());
  double z2 = dot(v0->_space, LVector3d::up());

  if (z0 < 0.0 && z1 < 0.0 && z2 < 0.0) {
    // A triangle on the southern hemisphere.  This projection will
    // reverse the vertex order.
    if (triangle_contains_circle(LPoint2d(0.0, 0.0),
                                 _singularity_radius,
                                 xy0, xy2, xy1)) {
      // The triangle does cross the singularity!  Reject it.
      _singularity_detected |= 1;
      return;
    }
  } else if (z0 > 0.0 && z1 > 0.0 && z2 > 0.0) {
    // A triangle on the northern hemisphere.  This projection will
    // preserve the vertex order.
    if (triangle_contains_circle(LPoint2d(0.0, 0.0),
                                 _singularity_radius,
                                 xy0, xy1, xy2)) {
      // The triangle does cross the singularity!  Reject it.
      _singularity_detected |= 2;
      return;
    }
  }

  // So the triangle is not at the north or south poles.  But it might
  // cross the seam at the back.  If it does, we'll simply draw it
  // twice: once at each side.

  // Determine which quadrant each of the vertices is in.  The
  // triangle crosses the seam if no vertices are in quadrants I and
  // II, and some vertices are in quadrant III and others are in
  // quadrant IV.

  if (xy0[1] >= 0.0 || xy1[1] >= 0.0 || xy2[1] >= 0.0) {
    // Some vertices are in quadrants I or II.
    rast.draw_triangle(v0, v1, v2);

  } else if (xy0[0] > 0.0 && xy1[0] > 0.0 && xy2[0] > 0.0) {
    // All vertices are in quadrant IV.
    rast.draw_triangle(v0, v1, v2);

  } else if (xy0[0] < 0.0 && xy1[0] < 0.0 && xy2[0] < 0.0) {
    // All vertices are in quadrant III.
    rast.draw_triangle(v0, v1, v2);

  } else {
    // The triangle crosses the seam.  Draw it twice.
    RasterizerVertex v0a = *v0;
    RasterizerVertex v1a = *v1;
    RasterizerVertex v2a = *v2;

    v0a._p = project_left(v0a._space, width_mm) * mm_to_pixels;
    v1a._p = project_left(v1a._space, width_mm) * mm_to_pixels;
    v2a._p = project_left(v2a._space, width_mm) * mm_to_pixels;
    rast.draw_triangle(&v0a, &v1a, &v2a);

    v0a._p = project_right(v0a._space, width_mm) * mm_to_pixels;
    v1a._p = project_right(v1a._space, width_mm) * mm_to_pixels;
    v2a._p = project_right(v2a._space, width_mm) * mm_to_pixels;
    rast.draw_triangle(&v0a, &v1a, &v2a);
  }
}

void StitchPSphereLens::
pick_up_singularity(TriangleRasterizer &rast,
                    const LMatrix3d &mm_to_pixels,
                    const LMatrix3d &pixels_to_mm,
                    const LMatrix3d &rotate,
                    double width_mm, StitchImage *input) {
  if (_singularity_detected & 2) {
    nout << "Picking up north pole singularity\n";
    // Determine what the bottom y pixel is of the circle around the
    // north pole.
    double d = deg_2_rad(_singularity_tolerance * 2.0);
    LPoint2d pmm = project(LVector3d(0.0, sin(d), cos(d)), width_mm);
    LPoint2d p = pmm * mm_to_pixels;

    int xsize = rast._output->get_x_size();
    int ysize = rast._output->get_y_size();

    int bot_y = min((int)ceil(p[1]), ysize - 1);
    RasterizerVertex v0;
    v0._p.set(0.0, 0.0);
    v0._uv.set(0.0, 0.0);
    v0._space.set(0.0, 0.0, 0.0);
    v0._alpha = 1.0;
    v0._visibility = 0;

    int xi, yi;
    for (yi = 0; yi <= bot_y; yi++) {
      // Project xi point 1 to determine the radius.
      v0._p.set(0, yi);
      v0._space = extrude(v0._p * pixels_to_mm, width_mm) * rotate;
      v0._uv = input->project(v0._space);

      for (xi = 0; xi < xsize; xi++) {
        double last_u = v0._uv[0];

        v0._p.set(xi, yi);
        v0._space = extrude(v0._p * pixels_to_mm, width_mm) * rotate;
        v0._uv = input->project(v0._space);
        rast.draw_pixel(&v0, fabs(v0._uv[0] - last_u));
      }
    }
  }
  if (_singularity_detected & 1) {
    nout << "Picking up south pole singularity\n";
    // Determine what the top y pixel is of the circle around the
    // south pole.
    double d = deg_2_rad(_singularity_tolerance * 2.0);
    LPoint2d pmm = project(LVector3d(0.0, sin(d), -cos(d)), width_mm);
    LPoint2d p = pmm * mm_to_pixels;

    int xsize = rast._output->get_x_size();
    int ysize = rast._output->get_y_size();

    int top_y = max((int)floor(p[1]), 0);
    RasterizerVertex v0;
    v0._p.set(0.0, 0.0);
    v0._uv.set(0.0, 0.0);
    v0._space.set(0.0, 0.0, 0.0);
    v0._alpha = 1.0;
    v0._visibility = 0;

    int xi, yi;
    for (yi = top_y; yi < ysize; yi++) {
      // Project xi point 1 to determine the radius.
      v0._p.set(0, yi);
      v0._space = extrude(v0._p * pixels_to_mm, width_mm) * rotate;
      v0._uv = input->project(v0._space);

      for (xi = 0; xi < xsize; xi++) {
        double last_u = v0._uv[0];

        v0._p.set(xi, yi);
        v0._space = extrude(v0._p * pixels_to_mm, width_mm) * rotate;
        v0._uv = input->project(v0._space);
        rast.draw_pixel(&v0, fabs(v0._uv[0] - last_u));
      }
    }
  }
}

void StitchPSphereLens::
make_lens_command(StitchCommand *parent) {
  StitchCommand *lens_cmd = new StitchCommand(parent, StitchCommand::C_lens);
  StitchCommand *cmd;
  cmd = new StitchCommand(lens_cmd, StitchCommand::C_psphere);
  if (_flags & F_focal_length) {
    cmd = new StitchCommand(lens_cmd, StitchCommand::C_focal_length);
    cmd->set_length(_focal_length);
  }
  if (_flags & F_fov) {
    cmd = new StitchCommand(lens_cmd, StitchCommand::C_fov);
    cmd->set_number(_fov);
  }
}
