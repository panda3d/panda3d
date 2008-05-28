// Filename: fltTransformPut.h
// Created by:  drose (29Aug00)
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

#ifndef FLTTRANSFORMPUT_H
#define FLTTRANSFORMPUT_H

#include "pandatoolbase.h"

#include "fltTransformRecord.h"

////////////////////////////////////////////////////////////////////
//       Class : FltTransformPut
// Description : A "put", which is a MultiGen concept of defining a
//               transformation by mapping three arbitrary points to
//               three new arbitrary points.
////////////////////////////////////////////////////////////////////
class FltTransformPut : public FltTransformRecord {
public:
  FltTransformPut(FltHeader *header);

  void set(const LPoint3d &from_origin,
           const LPoint3d &from_align,
           const LPoint3d &from_track,
           const LPoint3d &to_origin,
           const LPoint3d &to_align,
           const LPoint3d &to_track);

  const LPoint3d &get_from_origin() const;
  const LPoint3d &get_from_align() const;
  const LPoint3d &get_from_track() const;
  const LPoint3d &get_to_origin() const;
  const LPoint3d &get_to_align() const;
  const LPoint3d &get_to_track() const;

private:
  void recompute_matrix();

  LPoint3d _from_origin;
  LPoint3d _from_align;
  LPoint3d _from_track;
  LPoint3d _to_origin;
  LPoint3d _to_align;
  LPoint3d _to_track;

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
    register_type(_type_handle, "FltTransformPut",
                  FltTransformRecord::get_class_type());
  }

private:
  static TypeHandle _type_handle;
};

#endif


