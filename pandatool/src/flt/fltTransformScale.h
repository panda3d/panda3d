// Filename: fltTransformScale.h
// Created by:  drose (30Aug00)
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

#ifndef FLTTRANSFORMSCALE_H
#define FLTTRANSFORMSCALE_H

#include "pandatoolbase.h"

#include "fltTransformRecord.h"

////////////////////////////////////////////////////////////////////
//       Class : FltTransformScale
// Description : A transformation that applies a (possibly nonuniform)
//               scale.
////////////////////////////////////////////////////////////////////
class FltTransformScale : public FltTransformRecord {
public:
  FltTransformScale(FltHeader *header);

  void set(const LPoint3d &center, const LVecBase3 &scale);

  bool has_center() const;
  const LPoint3d &get_center() const;
  const LVecBase3 &get_scale() const;

private:
  void recompute_matrix();

  LPoint3d _center;
  LVecBase3 _scale;

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
    FltTransformRecord::init_type();
    register_type(_type_handle, "FltTransformScale",
                  FltTransformRecord::get_class_type());
  }

private:
  static TypeHandle _type_handle;
};

#endif
