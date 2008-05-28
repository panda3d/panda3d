// Filename: fltTransformRecord.h
// Created by:  drose (24Aug00)
//
////////////////////////////////////////////////////////////////////
//
// PANDA 3D SOFTWARE
// Copyright (c) Carnegie Mellon University.  All rights reserved.
//
// All use of this software is subject to the terms of the revised BSD
// license.  You should have received a copy of this license along
// with this source code in a file named "LICENSE."
//
////////////////////////////////////////////////////////////////////

#ifndef FLTTRANSFORMRECORD_H
#define FLTTRANSFORMRECORD_H

#include "pandatoolbase.h"

#include "fltRecord.h"

#include "luse.h"

////////////////////////////////////////////////////////////////////
//       Class : FltTransformRecord
// Description : A base class for a number of types of ancillary
//               records that follow beads and indicate some kind of a
//               transformation.  Pointers of this type are collected
//               in the FltTransformation class.
////////////////////////////////////////////////////////////////////
class FltTransformRecord : public FltRecord {
public:
  FltTransformRecord(FltHeader *header);

  const LMatrix4d &get_matrix() const;

protected:
  LMatrix4d _matrix;

public:
  virtual TypeHandle get_type() const {
    return get_class_type();
  }
  virtual TypeHandle force_init_type() {init_type(); return get_class_type();}
  static TypeHandle get_class_type() {
    return _type_handle;
  }
  static void init_type() {
    FltRecord::init_type();
    register_type(_type_handle, "FltTransformRecord",
                  FltRecord::get_class_type());
  }

private:
  static TypeHandle _type_handle;

  friend class FltBead;
};

#endif


