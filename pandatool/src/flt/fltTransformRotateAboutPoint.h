// Filename: fltTransformRotateAboutPoint.h
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

#ifndef FLTTRANSFORMROTATEABOUTPOINT_H
#define FLTTRANSFORMROTATEABOUTPOINT_H

#include "pandatoolbase.h"

#include "fltTransformRecord.h"

////////////////////////////////////////////////////////////////////
//       Class : FltTransformRotateAboutPoint
// Description : A transformation that rotates about a particular axis
//               in space, defined by a point and vector.
////////////////////////////////////////////////////////////////////
class FltTransformRotateAboutPoint : public FltTransformRecord {
public:
  FltTransformRotateAboutPoint(FltHeader *header);

  void set(const LPoint3d &center, const LVector3 &axis, PN_stdfloat angle);

  const LPoint3d &get_center() const;
  const LVector3 &get_axis() const;
  PN_stdfloat get_angle() const;

private:
  void recompute_matrix();

  LPoint3d _center;
  LVector3 _axis;
  PN_stdfloat _angle;

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
    register_type(_type_handle, "FltTransformRotateAboutPoint",
                  FltTransformRecord::get_class_type());
  }

private:
  static TypeHandle _type_handle;
};

#endif
