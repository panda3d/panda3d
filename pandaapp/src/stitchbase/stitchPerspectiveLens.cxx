// Filename: stitchPerspectiveLens.cxx
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

#include "stitchPerspectiveLens.h"
#include "stitchCommand.h"

#include "pandatoolbase.h"
#include "deg_2_rad.h"

#include <math.h>

StitchPerspectiveLens::
StitchPerspectiveLens() {
}

void StitchPerspectiveLens::
set_hfov(double fov_deg) {
  StitchLens::set_hfov(fov_deg);
  _tan_fov = tan(deg_2_rad(_fov / 2.0)) * 2.0;
}

double StitchPerspectiveLens::
get_focal_length(double width_mm) const {
  if (_flags & F_focal_length) {
    return _focal_length;
  }
  if (_flags & F_fov) {
    return width_mm / _tan_fov;
  }
  return 0.0;
}

double StitchPerspectiveLens::
get_hfov(double width_mm) const {
  if (_flags & F_fov) {
    return _fov;
  }
  if (_flags & F_focal_length) {
    return 2.0 * rad_2_deg(atan(width_mm / (2.0 * _focal_length)));
  }
  return 0.0;
}

LVector3d StitchPerspectiveLens::
extrude(const LPoint2d &point_mm, double width_mm) const {
  return LVector3d::rfu(point_mm[0], get_focal_length(width_mm), point_mm[1]);
}

LPoint2d StitchPerspectiveLens::
project(const LVector3d &vec, double width_mm) const {
  double r = dot(vec, LVector3d::right());
  double f = dot(vec, LVector3d::forward());
  double u = dot(vec, LVector3d::up());
  if (f <= 0.0) {
    // If the point is in or behind our view plane, project it out to
    // as near to infinity as we can comfortably manage.
    return LPoint2d(r / 0.000001, u / 0.000001);
  } else {
    return LPoint2d(r / f * get_focal_length(width_mm),
                    u / f * get_focal_length(width_mm));
  }
}

void StitchPerspectiveLens::
make_lens_command(StitchCommand *parent) {
  StitchCommand *lens_cmd = new StitchCommand(parent, StitchCommand::C_lens);
  StitchCommand *cmd;
  cmd = new StitchCommand(lens_cmd, StitchCommand::C_perspective);
  if (_flags & F_focal_length) {
    cmd = new StitchCommand(lens_cmd, StitchCommand::C_focal_length);
    cmd->set_length(_focal_length);
  }
  if (_flags & F_fov) {
    cmd = new StitchCommand(lens_cmd, StitchCommand::C_fov);
    cmd->set_number(_fov);
  }
}
