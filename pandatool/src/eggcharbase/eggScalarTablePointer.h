// Filename: eggScalarTablePointer.h
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

#ifndef EGGSCALARTABLEPOINTER_H
#define EGGSCALARTABLEPOINTER_H

#include "pandatoolbase.h"

#include "eggSliderPointer.h"

#include "eggSAnimData.h"
#include "pointerTo.h"

////////////////////////////////////////////////////////////////////
//       Class : EggScalarTablePointer
// Description : This stores a pointer back to an EggSAnimData table
//               (i.e. an <S$Anim> entry in an egg file),
//               corresponding to the animation data from a single
//               bundle for this slider.
////////////////////////////////////////////////////////////////////
class EggScalarTablePointer : public EggSliderPointer {
public:
  EggScalarTablePointer(EggObject *object);

  virtual int get_num_frames() const;
  virtual void extend_to(int num_frames);
  virtual double get_frame(int n) const;

private:
  PT(EggSAnimData) _data;

public:
  static TypeHandle get_class_type() {
    return _type_handle;
  }
  static void init_type() {
    EggSliderPointer::init_type();
    register_type(_type_handle, "EggScalarTablePointer",
                  EggSliderPointer::get_class_type());
  }
  virtual TypeHandle get_type() const {
    return get_class_type();
  }
  virtual TypeHandle force_init_type() {init_type(); return get_class_type();}

private:
  static TypeHandle _type_handle;
};

#endif


