// Filename: fltTransformRotateAboutEdge.h
// Created by:  drose (30Aug00)
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

#ifndef FLTTRANSFORMROTATEABOUTEDGE_H
#define FLTTRANSFORMROTATEABOUTEDGE_H

#include "pandatoolbase.h"

#include "fltTransformRecord.h"

////////////////////////////////////////////////////////////////////
//       Class : FltTransformRotateAboutEdge
// Description : A transformation that rotates about a particular axis
//               in space, defined by two endpoints.
////////////////////////////////////////////////////////////////////
class FltTransformRotateAboutEdge : public FltTransformRecord {
public:
  FltTransformRotateAboutEdge(FltHeader *header);

  void set(const LPoint3d &point_a, const LPoint3d &point_b, float angle);

  const LPoint3d &get_point_a() const;
  const LPoint3d &get_point_b() const;
  float get_angle() const;

private:
  void recompute_matrix();

  LPoint3d _point_a;
  LPoint3d _point_b;
  float _angle;

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
    register_type(_type_handle, "FltTransformRotateAboutEdge",
                  FltTransformRecord::get_class_type());
  }

private:
  static TypeHandle _type_handle;
};

#endif
