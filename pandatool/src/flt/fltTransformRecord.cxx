// Filename: fltTransformRecord.cxx
// Created by:  drose (24Aug00)
// 
////////////////////////////////////////////////////////////////////

#include "fltTransformRecord.h"

TypeHandle FltTransformRecord::_type_handle;

////////////////////////////////////////////////////////////////////
//     Function: FltTransformRecord::Constructor
//       Access: Public
//  Description: 
////////////////////////////////////////////////////////////////////
FltTransformRecord::
FltTransformRecord(FltHeader *header) : FltRecord(header) {
  _matrix = LMatrix4d::ident_mat();
}

////////////////////////////////////////////////////////////////////
//     Function: FltTransformRecord::get_matrix
//       Access: Public
//  Description: Returns the transform matrix represented by this
//               particular component of the transform.
////////////////////////////////////////////////////////////////////
const LMatrix4d &FltTransformRecord::
get_matrix() const {
  return _matrix;
}
