// Filename: fltTransformRotateScale.h
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

#ifndef FLTTRANSFORMROTATESCALE_H
#define FLTTRANSFORMROTATESCALE_H

#include "pandatoolbase.h"

#include "fltTransformRecord.h"

////////////////////////////////////////////////////////////////////
//       Class : FltTransformRotateScale
// Description : A combination rotation and scale.  This is sometimes
//               called "Rotate To Point" within MultiGen.
////////////////////////////////////////////////////////////////////
class FltTransformRotateScale : public FltTransformRecord {
public:
  FltTransformRotateScale(FltHeader *header);

  void set(const LPoint3d &center, const LPoint3d &reference_point,
           const LPoint3d &to_point, bool axis_scale);

  const LPoint3d &get_center() const;
  const LPoint3d &get_reference_point() const;
  const LPoint3d &get_to_point() const;
  PN_stdfloat get_overall_scale() const;
  PN_stdfloat get_axis_scale() const;
  PN_stdfloat get_angle() const;

private:
  void recompute_matrix();

  LPoint3d _center;
  LPoint3d _reference_point;
  LPoint3d _to_point;
  PN_stdfloat _overall_scale;
  PN_stdfloat _axis_scale;
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
    register_type(_type_handle, "FltTransformRotateScale",
                  FltTransformRecord::get_class_type());
  }

private:
  static TypeHandle _type_handle;
};

#endif


