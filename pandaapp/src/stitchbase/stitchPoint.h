// Filename: stitchPoint.h
// Created by:  drose (04Nov99)
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

#ifndef STITCHPOINT_H
#define STITCHPOINT_H

#include "pandaappbase.h"

#include <luse.h>

#include <string>

class StitchImage;

class StitchPoint {
public:
  StitchPoint(const string &name);

  void set_space(const LVector3d &space);

  string _name;
  bool _space_known;
  LVector3d _space;

  typedef set<StitchImage *> Images;
  Images _images;
};

#endif

