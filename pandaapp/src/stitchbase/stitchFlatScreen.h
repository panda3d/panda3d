// Filename: stitchFlatScreen.h
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

#ifndef STITCHFLATSCREEN_H
#define STITCHFLATSCREEN_H

#include "stitchScreen.h"
#include "plane.h"

////////////////////////////////////////////////////////////////////
//       Class : StitchFlatScreen
// Description : A simple flat screen.  This is really an infinite
//               plane.
////////////////////////////////////////////////////////////////////
class StitchFlatScreen : public StitchScreen {
public:
  StitchFlatScreen();

  void set_plane(const Planed &plane);

protected:
  virtual double compute_intersect(const LPoint3d &origin, 
                                   const LVector3d &direction) const;

private:
  Planed _plane;
};

#endif


