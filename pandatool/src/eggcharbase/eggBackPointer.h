// Filename: eggBackPointer.h
// Created by:  drose (26Feb01)
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

#ifndef EGGBACKPOINTER_H
#define EGGBACKPOINTER_H

#include "pandatoolbase.h"

#include "typedObject.h"

////////////////////////////////////////////////////////////////////
//       Class : EggBackPointer
// Description : This stores a pointer from an EggJointData or
//               EggSliderData object back to the referencing data in
//               an egg file.  One of these objects corresponds to
//               each model appearing in an egg file, and may
//               reference either a single node, or a table, or a slew
//               of vertices and primitives, depending on the type of
//               data stored.
//
//               This is just an abstract base class.  The actual
//               details are stored in the various subclasses.
////////////////////////////////////////////////////////////////////
class EggBackPointer : public TypedObject {
public:
  EggBackPointer();

  virtual int get_num_frames() const=0;
  virtual void extend_to(int num_frames);
  virtual bool has_vertices() const;

public:
  static TypeHandle get_class_type() {
    return _type_handle;
  }
  static void init_type() {
    TypedObject::init_type();
    register_type(_type_handle, "EggBackPointer",
                  TypedObject::get_class_type());
  }
  virtual TypeHandle get_type() const {
    return get_class_type();
  }
  virtual TypeHandle force_init_type() {init_type(); return get_class_type();}

private:
  static TypeHandle _type_handle;
};

#endif


