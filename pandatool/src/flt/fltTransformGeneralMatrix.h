// Filename: fltTransformGeneralMatrix.h
// Created by:  drose (24Aug00)
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

#ifndef FLTTRANSFORMGENERALMATRIX_H
#define FLTTRANSFORMGENERALMATRIX_H

#include "pandatoolbase.h"

#include "fltTransformRecord.h"

////////////////////////////////////////////////////////////////////
//       Class : FltTransformGeneralMatrix
// Description : A general 4x4 matrix.  This appears in the flt file
//               when there is no record of the composition of the
//               transform.
////////////////////////////////////////////////////////////////////
class FltTransformGeneralMatrix : public FltTransformRecord {
public:
  FltTransformGeneralMatrix(FltHeader *header);

  void set_matrix(const LMatrix4d &matrix);
  void set_matrix(const LMatrix4f &matrix);

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
    register_type(_type_handle, "FltTransformGeneralMatrix",
                  FltTransformRecord::get_class_type());
  }

private:
  static TypeHandle _type_handle;
};

#endif


