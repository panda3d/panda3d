/**
 * PANDA 3D SOFTWARE
 * Copyright (c) Carnegie Mellon University.  All rights reserved.
 *
 * All use of this software is subject to the terms of the revised BSD
 * license.  You should have received a copy of this license along
 * with this source code in a file named "LICENSE."
 *
 * @file fltLOD.h
 * @author drose
 * @date 2000-08-25
 */

#ifndef FLTLOD_H
#define FLTLOD_H

#include "pandatoolbase.h"

#include "fltBeadID.h"

/**
 * A Level-of-Detail record.
 */
class FltLOD : public FltBeadID {
public:
  FltLOD(FltHeader *header);

  enum Flags {
    F_use_previous_slant  = 0x80000000,
    F_freeze_center       = 0x20000000
  };

  double _switch_in;
  double _switch_out;
  int _special_id1, _special_id2;
  unsigned int _flags;
  double _center_x;
  double _center_y;
  double _center_z;
  double _transition_range;

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
    register_type(_type_handle, "FltLOD",
                  FltBeadID::get_class_type());
  }

private:
  static TypeHandle _type_handle;
};

#endif
