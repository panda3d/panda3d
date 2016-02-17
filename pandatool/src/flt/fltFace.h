/**
 * PANDA 3D SOFTWARE
 * Copyright (c) Carnegie Mellon University.  All rights reserved.
 *
 * All use of this software is subject to the terms of the revised BSD
 * license.  You should have received a copy of this license along
 * with this source code in a file named "LICENSE."
 *
 * @file fltFace.h
 * @author drose
 * @date 2000-08-25
 */

#ifndef FLTFACE_H
#define FLTFACE_H

#include "pandatoolbase.h"

#include "fltGeometry.h"

/**
 * A single face bead, e.g.  a polygon.
 */
class FltFace : public FltGeometry {
public:
  FltFace(FltHeader *header);


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
    FltGeometry::init_type();
    register_type(_type_handle, "FltFace",
                  FltGeometry::get_class_type());
  }

private:
  static TypeHandle _type_handle;
};

#include "fltFace.I"

#endif
