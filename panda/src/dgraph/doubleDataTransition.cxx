// Filename: doubleDataTransition.cxx
// Created by:  drose (27Mar00)
// 
////////////////////////////////////////////////////////////////////

#include "doubleDataTransition.h"
#include "doubleDataAttribute.h"

// Tell GCC that we'll take care of the instantiation explicitly here.
#ifdef __GNUC__
#pragma implementation
#endif

TypeHandle DoubleDataTransition::_type_handle;

////////////////////////////////////////////////////////////////////
//     Function: DoubleDataTransition::make_copy
//       Access: Public, Virtual
//  Description: 
////////////////////////////////////////////////////////////////////
NodeTransition *DoubleDataTransition::
make_copy() const {
  return new DoubleDataTransition(*this);
}

////////////////////////////////////////////////////////////////////
//     Function: DoubleDataTransition::make_attrib
//       Access: Public, Virtual
//  Description: 
////////////////////////////////////////////////////////////////////
NodeAttribute *DoubleDataTransition::
make_attrib() const {
  return new DoubleDataAttribute;
}
