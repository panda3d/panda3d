// Filename: stitchMultiScreen.h
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

#ifndef STITCHMULTISCREEN_H
#define STITCHMULTISCREEN_H

#include "stitchScreen.h"
#include "pset.h"
#include "pointerTo.h"

////////////////////////////////////////////////////////////////////
//       Class : StitchMultiScreen
// Description : A composite of one or more simple screens.
//
//               If there is at least one screen in the set, the
//               StitchMultiScreen behaves like the intersection of
//               all of the screens.  If there are no screens, it
//               behaves like an infinitely large screen at infinity.
////////////////////////////////////////////////////////////////////
class StitchMultiScreen : public StitchScreen {
public:
  StitchMultiScreen();
  virtual ~StitchMultiScreen();

  void add_screen(StitchScreen *screen);
  void clear_screens();
  bool is_empty() const;

  virtual bool intersect(LPoint3d &result,
                         const LPoint3d &origin, 
                         const LVector3d &direction) const;

protected:
  virtual double compute_intersect(const LPoint3d &origin, 
                                   const LVector3d &direction) const;

private:
  typedef pset< PT(StitchScreen) > Screens;
  Screens _screens;
};

#endif


