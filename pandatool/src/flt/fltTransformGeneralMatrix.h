// Filename: fltTransformGeneralMatrix.h
// Created by:  drose (24Aug00)
// 
////////////////////////////////////////////////////////////////////

#ifndef FLTTRANSFORMGENERALMATRIX_H
#define FLTTRANSFORMGENERALMATRIX_H

#include <pandatoolbase.h>

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


