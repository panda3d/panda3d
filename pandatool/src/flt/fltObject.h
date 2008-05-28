// Filename: fltObject.h
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

#ifndef FLTOBJECT_H
#define FLTOBJECT_H

#include "pandatoolbase.h"

#include "fltBeadID.h"

////////////////////////////////////////////////////////////////////
//       Class : FltObject
// Description : The main objecting bead of the flt file.
////////////////////////////////////////////////////////////////////
class FltObject : public FltBeadID {
public:
  FltObject(FltHeader *header);

  enum Flags {
    F_no_daylight    = 0x80000000,
    F_no_dusk        = 0x40000000,
    F_no_night       = 0x20000000,
    F_no_illuminate  = 0x10000000,
    F_flat_shaded    = 0x08000000,
    F_shadow_object  = 0x04000000,
  };

  unsigned int _flags;
  int _relative_priority;
  int _transparency;
  int _special_id1, _special_id2;
  int _significance;

protected:
  virtual bool extract_record(FltRecordReader &reader);
  virtual bool build_record(FltRecordWriter &writer) const;

public:
  virtual TypeHandle get_type() const {
    return get_class_type();
  }
  virtual TypeHandle force_init_type() {init_type(); return get_class_type();}
  static TypeHandle get_class_type() {
    return _type_handle;
  }
  static void init_type() {
    FltBeadID::init_type();
    register_type(_type_handle, "FltObject",
                  FltBeadID::get_class_type());
  }

private:
  static TypeHandle _type_handle;
};

#endif


