// Filename: doublePtrDataAttribute.cxx
// Created by:  jason (07Aug00)
// 
////////////////////////////////////////////////////////////////////

#include "doublePtrDataAttribute.h"
#include "doublePtrDataTransition.h"

// Tell GCC that we'll take care of the instantiation explicitly here.
#ifdef __GNUC__
#pragma implementation
#endif

TypeHandle DoublePtrDataAttribute::_type_handle;

////////////////////////////////////////////////////////////////////
//     Function: DoublePtrDataAttribute::make_copy
//       Access: Public, Virtual
//  Description: 
////////////////////////////////////////////////////////////////////
NodeAttribute *DoublePtrDataAttribute::
make_copy() const {
  return new DoublePtrDataAttribute(*this);
}

////////////////////////////////////////////////////////////////////
//     Function: DoublePtrDataAttribute::make_initial
//       Access: Public, Virtual
//  Description: 
////////////////////////////////////////////////////////////////////
NodeAttribute *DoublePtrDataAttribute::
make_initial() const {
  return new DoublePtrDataAttribute;
}
