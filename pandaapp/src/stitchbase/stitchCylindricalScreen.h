// Filename: stitchCylindricalScreen.h
// Created by:  drose (16Jul01)
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

#ifndef STITCHCYLINDRICALSCREEN_H
#define STITCHCYLINDRICALSCREEN_H

#include "stitchScreen.h"

////////////////////////////////////////////////////////////////////
//       Class : StitchCylindricalScreen
// Description : A cylindrical screen shape, axis-aligned with the Z
//               axis (which is normally the up axis), and with an
//               arbitrary height and radius.  The screen may also
//               consist of only a portion of the cylinder, specified
//               by an angle limit.
////////////////////////////////////////////////////////////////////
class StitchCylindricalScreen : public StitchScreen {
public:
  StitchCylindricalScreen();

  void set_angle(double start, double end);
  void set_height(double bottom, double top);
  void set_radius(double radius);

protected:
  virtual double compute_intersect(const LPoint3d &origin, 
                                   const LVector3d &direction) const;

  void validate_point(double &t,
                      const LPoint3d &origin, 
                      const LVector3d &direction) const;

private:
  double _start_angle, _end_angle;
  double _bottom, _top;
  double _radius;

  enum {
    F_angle   = 0x001,
    F_height  = 0x002
  };

  int _flags;
};

#endif


