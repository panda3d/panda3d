// Filename: eggSliderPointer.h
// Created by:  drose (18Jul03)
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

#ifndef EGGSLIDERPOINTER_H
#define EGGSLIDERPOINTER_H

#include "pandatoolbase.h"

#include "eggBackPointer.h"

#include "luse.h"

////////////////////////////////////////////////////////////////////
//       Class : EggSliderPointer
// Description : This is a base class for EggVertexPointer and
//               EggScalarTablePointer.
////////////////////////////////////////////////////////////////////
class EggSliderPointer : public EggBackPointer {
public:
  virtual int get_num_frames() const=0;
  virtual double get_frame(int n) const=0;

public:
  static TypeHandle get_class_type() {
    return _type_handle;
  }
  static void init_type() {
    EggBackPointer::init_type();
    register_type(_type_handle, "EggSliderPointer",
                  EggBackPointer::get_class_type());
  }
  virtual TypeHandle get_type() const {
    return get_class_type();
  }
  virtual TypeHandle force_init_type() {init_type(); return get_class_type();}

private:
  static TypeHandle _type_handle;
};

#endif


