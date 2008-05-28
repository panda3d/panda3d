// Filename: fltCurve.h
// Created by:  drose (28Feb01)
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

#ifndef FLTCURVE_H
#define FLTCURVE_H

#include "pandatoolbase.h"

#include "fltBeadID.h"
#include "fltHeader.h"

#include "luse.h"

////////////////////////////////////////////////////////////////////
//       Class : FltCurve
// Description : A single curve, like a Bezier or B-Spline.
////////////////////////////////////////////////////////////////////
class FltCurve : public FltBeadID {
public:
  FltCurve(FltHeader *header);

  enum CurveType {
    CT_b_spline            = 4,
    CT_cardinal            = 5,
    CT_bezier              = 6
  };

  typedef pvector<LPoint3d> ControlPoints;

  CurveType _curve_type;
  ControlPoints _control_points;

public:
  INLINE int get_num_control_points() const;
  INLINE const LPoint3d &get_control_point(int n) const;


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
    register_type(_type_handle, "FltCurve",
                  FltBeadID::get_class_type());
  }

private:
  static TypeHandle _type_handle;
};

#include "fltCurve.I"

#endif


