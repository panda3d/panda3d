// Filename: matrixDataAttribute.cxx
// Created by:  drose (27Mar00)
// 
////////////////////////////////////////////////////////////////////

#include "matrixDataAttribute.h"
#include "matrixDataTransition.h"

// Tell GCC that we'll take care of the instantiation explicitly here.
#ifdef __GNUC__
#pragma implementation
#endif

TypeHandle MatrixDataAttribute::_type_handle;

////////////////////////////////////////////////////////////////////
//     Function: MatrixDataAttribute::make_copy
//       Access: Public, Virtual
//  Description: 
////////////////////////////////////////////////////////////////////
NodeAttribute *MatrixDataAttribute::
make_copy() const {
  return new MatrixDataAttribute(*this);
}

////////////////////////////////////////////////////////////////////
//     Function: MatrixDataAttribute::make_initial
//       Access: Public, Virtual
//  Description: 
////////////////////////////////////////////////////////////////////
NodeAttribute *MatrixDataAttribute::
make_initial() const {
  return new MatrixDataAttribute;
}
