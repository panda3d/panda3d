/**
 * PANDA 3D SOFTWARE
 * Copyright (c) Carnegie Mellon University.  All rights reserved.
 *
 * All use of this software is subject to the terms of the revised BSD
 * license.  You should have received a copy of this license along
 * with this source code in a file named "LICENSE."
 *
 * @file fltGroup.h
 * @author drose
 * @date 2000-08-24
 */

#ifndef FLTGROUP_H
#define FLTGROUP_H

#include "pandatoolbase.h"

#include "fltBeadID.h"

/**
 * The main grouping bead of the flt file.
 */
class FltGroup : public FltBeadID {
public:
  FltGroup(FltHeader *header);

  enum Flags {
    F_forward_animation   = 0x40000000,
    F_swing_animation     = 0x20000000,
    F_bounding_box        = 0x10000000,
    F_freeze_bounding_box = 0x08000000,
    F_default_parent      = 0x04000000,
  };

  int _relative_priority;
  unsigned int _flags;
  int _special_id1, _special_id2;
  int _significance;
  int _layer_id;

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
    register_type(_type_handle, "FltGroup",
                  FltBeadID::get_class_type());
  }

private:
  static TypeHandle _type_handle;
};

#endif
