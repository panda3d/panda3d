// Filename: matrixDataTransition.cxx
// Created by:  drose (27Mar00)
// 
////////////////////////////////////////////////////////////////////

#include "matrixDataTransition.h"
#include "matrixDataAttribute.h"

// Tell GCC that we'll take care of the instantiation explicitly here.
#ifdef __GNUC__
#pragma implementation
#endif

TypeHandle MatrixDataTransition::_type_handle;

////////////////////////////////////////////////////////////////////
//     Function: MatrixDataTransition::make_copy
//       Access: Public, Virtual
//  Description: 
////////////////////////////////////////////////////////////////////
NodeTransition *MatrixDataTransition::
make_copy() const {
  return new MatrixDataTransition(*this);
}

////////////////////////////////////////////////////////////////////
//     Function: MatrixDataTransition::make_attrib
//       Access: Public, Virtual
//  Description: 
////////////////////////////////////////////////////////////////////
NodeAttribute *MatrixDataTransition::
make_attrib() const {
  return new MatrixDataAttribute;
}
