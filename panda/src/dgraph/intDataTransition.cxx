// Filename: intDataTransition.cxx
// Created by:  drose (27Mar00)
// 
////////////////////////////////////////////////////////////////////

#include "intDataTransition.h"
#include "intDataAttribute.h"

// Tell GCC that we'll take care of the instantiation explicitly here.
#ifdef __GNUC__
#pragma implementation
#endif

TypeHandle IntDataTransition::_type_handle;

////////////////////////////////////////////////////////////////////
//     Function: IntDataTransition::make_copy
//       Access: Public, Virtual
//  Description: 
////////////////////////////////////////////////////////////////////
NodeTransition *IntDataTransition::
make_copy() const {
  return new IntDataTransition(*this);
}

////////////////////////////////////////////////////////////////////
//     Function: IntDataTransition::make_attrib
//       Access: Public, Virtual
//  Description: 
////////////////////////////////////////////////////////////////////
NodeAttribute *IntDataTransition::
make_attrib() const {
  return new IntDataAttribute;
}
