// Filename: stitchCylindricalLens.cxx
// Created by:  drose (09Nov99)
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

#include "stitchCylindricalLens.h"
#include "stitchCommand.h"
#include "triangleRasterizer.h"

#include "pandatoolbase.h"
#include "deg_2_rad.h"

#include <math.h>

// This is the focal-length constant for fisheye lenses.  See
// stitchFisheyeLens.h.
static const double cylindrical_k = 60.0;

StitchCylindricalLens::
StitchCylindricalLens() {
}

double StitchCylindricalLens::
get_focal_length(double width_mm) const {
  if (_flags & F_focal_length) {
    return _focal_length;
  }
  if (_flags & F_fov) {
    return width_mm * cylindrical_k / _fov;
  }
  return 0.0;
}

double StitchCylindricalLens::
get_hfov(double width_mm) const {
  if (_flags & F_fov) {
    return _fov;
  }
  if (_flags & F_focal_length) {
    return width_mm * cylindrical_k / _focal_length;
  }
  return 0.0;
}

double StitchCylindricalLens::
get_vfov(double height_mm) const {
  return 2.0 * rad_2_deg(atan(height_mm /
                              (2.0 * get_focal_length(height_mm))));
}

LVector3d StitchCylindricalLens::
extrude(const LPoint2d &point_mm, double width_mm) const {
  LVector2d v2 = point_mm;

  double fl = get_focal_length(width_mm);
  return LVector3d(sin(deg_2_rad(v2[0] * cylindrical_k / fl)) * fl,
                   cos(deg_2_rad(v2[0] * cylindrical_k / fl)) * fl,
                   v2[1]);
}


LPoint2d StitchCylindricalLens::
project(const LVector3d &vec, double width_mm) const {
  // A cylindrical lens is a cross between a fisheye and a normal
  // lens.  It is curved in the horizontal direction, and straight in
  // the vertical direction.

  LVector3d v3 = vec * LMatrix4d::convert_mat(CS_default, CS_zup_right);

  // To compute the x position on the frame, we only need to consider
  // the angle of the vector about the Z axis.  Project the vector
  // into the XY plane to do this.

  LVector2d xy(v3[0], v3[1]);

  // The x position is the angle about the Z axis.
  double x =
    rad_2_deg(atan2(xy[0], xy[1])) * get_focal_length(width_mm) / cylindrical_k;

  // The y position is the Z height divided by the perspective
  // distance.
  double y = v3[2] / length(xy) * get_focal_length(width_mm);

  return LPoint2d(x, y);
}

LPoint2d StitchCylindricalLens::
project_left(const LVector3d &vec, double width_mm) const {
  // This is just like project(), except that if the vertex extends
  // below -180 degrees, it remains on the left side of the film
  // (instead of wrapping around to the right side).

  LVector3d v3 = vec * LMatrix4d::convert_mat(CS_default, CS_zup_right);
  LVector2d xy(v3[0], v3[1]);
  double x =
    (rad_2_deg(atan2(-xy[0], -xy[1])) - 180.0) *
    get_focal_length(width_mm) / cylindrical_k;

  double y = v3[2] / length(xy) * get_focal_length(width_mm);
  return LPoint2d(x, y);
}

LPoint2d StitchCylindricalLens::
project_right(const LVector3d &vec, double width_mm) const {
  // This is just like project(), except that if the vertex extends
  // above 180 degrees, it remains on the right side of the film
  // (instead of wrapping around to the left side).

  LVector3d v3 = vec * LMatrix4d::convert_mat(CS_default, CS_zup_right);
  LVector2d xy(v3[0], v3[1]);
  double x =
    (rad_2_deg(atan2(-xy[0], -xy[1])) + 180.0) *
    get_focal_length(width_mm) / cylindrical_k;

  double y = v3[2] / length(xy) * get_focal_length(width_mm);
  return LPoint2d(x, y);
}

void StitchCylindricalLens::
draw_triangle(TriangleRasterizer &rast, const LMatrix3d &mm_to_pixels,
              double width_mm, const RasterizerVertex *v0,
              const RasterizerVertex *v1, const RasterizerVertex *v2) {
  // A cylindrical lens has a seam at 180 and -180 degrees (regardless
  // of its field of view).  If the triangle crosses that seam, we'll
  // simply draw it twice: once at each side.

  // Determine which quadrant each of the vertices is in.  The
  // triangle crosses the seam if no vertices are in quadrants I and
  // II, and some vertices are in quadrant III and others are in
  // quadrant IV.

  LVector2d xy0(dot(v0->_space, LVector3d::right()),
                dot(v0->_space, LVector3d::forward()));
  LVector2d xy1(dot(v1->_space, LVector3d::right()),
                dot(v1->_space, LVector3d::forward()));
  LVector2d xy2(dot(v2->_space, LVector3d::right()),
                dot(v2->_space, LVector3d::forward()));

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

void StitchCylindricalLens::
make_lens_command(StitchCommand *parent) {
  StitchCommand *lens_cmd = new StitchCommand(parent, StitchCommand::C_lens);
  StitchCommand *cmd;
  cmd = new StitchCommand(lens_cmd, StitchCommand::C_cylindrical);
  if (_flags & F_focal_length) {
    cmd = new StitchCommand(lens_cmd, StitchCommand::C_focal_length);
    cmd->set_length(_focal_length);
  }
  if (_flags & F_fov) {
    cmd = new StitchCommand(lens_cmd, StitchCommand::C_fov);
    cmd->set_number(_fov);
  }
}
