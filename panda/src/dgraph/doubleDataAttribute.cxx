// Filename: doubleDataAttribute.cxx
// Created by:  drose (27Mar00)
// 
////////////////////////////////////////////////////////////////////

#include "doubleDataAttribute.h"
#include "doubleDataTransition.h"

// Tell GCC that we'll take care of the instantiation explicitly here.
#ifdef __GNUC__
#pragma implementation
#endif

TypeHandle DoubleDataAttribute::_type_handle;

////////////////////////////////////////////////////////////////////
//     Function: DoubleDataAttribute::make_copy
//       Access: Public, Virtual
//  Description: 
////////////////////////////////////////////////////////////////////
NodeAttribute *DoubleDataAttribute::
make_copy() const {
  return new DoubleDataAttribute(*this);
}

////////////////////////////////////////////////////////////////////
//     Function: DoubleDataAttribute::make_initial
//       Access: Public, Virtual
//  Description: 
////////////////////////////////////////////////////////////////////
NodeAttribute *DoubleDataAttribute::
make_initial() const {
  return new DoubleDataAttribute;
}
