// Filename: fltTransformTranslate.h
// Created by:  drose (30Aug00)
//
////////////////////////////////////////////////////////////////////
//
// PANDA 3D SOFTWARE
// Copyright (c) 2001, Disney Enterprises, Inc.  All rights reserved
//
// All use of this software is subject to the terms of the Panda 3d
// Software license.  You should have received a copy of this license
// along with this source code; you will also find a current copy of
// the license at http://www.panda3d.org/license.txt .
//
// To contact the maintainers of this program write to
// panda3d@yahoogroups.com .
//
////////////////////////////////////////////////////////////////////

#ifndef FLTTRANSFORMTRANSLATE_H
#define FLTTRANSFORMTRANSLATE_H

#include "pandatoolbase.h"

#include "fltTransformRecord.h"

////////////////////////////////////////////////////////////////////
//       Class : FltTransformTranslate
// Description : A transformation that applies a translation.
////////////////////////////////////////////////////////////////////
class FltTransformTranslate : public FltTransformRecord {
public:
  FltTransformTranslate(FltHeader *header);

  void set(const LPoint3d &from, const LVector3d &delta);

  const LPoint3d &get_from() const;
  const LVector3d &get_delta() const;

private:
  void recompute_matrix();

  LPoint3d _from;
  LVector3d _delta;

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
    register_type(_type_handle, "FltTransformTranslate",
                  FltTransformRecord::get_class_type());
  }

private:
  static TypeHandle _type_handle;
};

#endif
