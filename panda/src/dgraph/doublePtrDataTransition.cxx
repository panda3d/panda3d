// Filename: doublePtrDataTransition.cxx
// Created by:  jason (07Aug00)
// 
////////////////////////////////////////////////////////////////////

#include "doublePtrDataTransition.h"
#include "doublePtrDataAttribute.h"

// Tell GCC that we'll take care of the instantiation explicitly here.
#ifdef __GNUC__
#pragma implementation
#endif

TypeHandle DoublePtrDataTransition::_type_handle;

////////////////////////////////////////////////////////////////////
//     Function: DoublePtrDataTransition::make_copy
//       Access: Public, Virtual
//  Description: 
////////////////////////////////////////////////////////////////////
NodeTransition *DoublePtrDataTransition::
make_copy() const {
  return new DoublePtrDataTransition(*this);
}

////////////////////////////////////////////////////////////////////
//     Function: DoublePtrDataTransition::make_attrib
//       Access: Public, Virtual
//  Description: 
////////////////////////////////////////////////////////////////////
NodeAttribute *DoublePtrDataTransition::
make_attrib() const {
  return new DoublePtrDataAttribute;
}
