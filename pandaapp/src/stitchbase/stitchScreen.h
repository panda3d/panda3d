// Filename: stitchScreen.h
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

#ifndef STITCHSCREEN_H
#define STITCHSCREEN_H

#include "luse.h"
#include "referenceCount.h"
#include "namable.h"

////////////////////////////////////////////////////////////////////
//       Class : StitchScreen
// Description : This is an abstract base class defining the interface
//               to a number of different kinds of screens that images
//               may be projected onto.
//
//               The shape and position of the screen geometry
//               determines the net warping of an image transform, but
//               only when the input projector and the output image
//               camera are not positioned at the same nodal point.
////////////////////////////////////////////////////////////////////
class StitchScreen : public ReferenceCount, public Namable {
public:
  StitchScreen();
  virtual ~StitchScreen();

  void clear_transform();
  void set_transform(const LMatrix4d &transform);
  void set_hpr(const LVecBase3d &hpr);
  void set_pos(const LPoint3d &pos);

  virtual bool intersect(LPoint3d &result,
                         const LPoint3d &origin, 
                         const LVector3d &direction) const;

protected:
  virtual double compute_intersect(const LPoint3d &origin, 
                                   const LVector3d &direction) const=0;

protected:
  bool _hpr_set;
  LVecBase3d _hpr;
  bool _pos_set;
  LPoint3d _pos;
  LMatrix4d _transform;
  LMatrix4d _inv_transform;

public:
  static TypeHandle get_class_type() {
    return _type_handle;
  }
  static void init_type() {
    ReferenceCount::init_type();
    Namable::init_type();
    register_type(_type_handle, "StitchScreen",
                  ReferenceCount::get_class_type(),
                  Namable::get_class_type());
  }

private:
  static TypeHandle _type_handle;

  friend class StitchMultiScreen;
};

#endif

