// Filename: depthWriteTransition.cxx
// Created by:  drose (31Mar00)
// 
////////////////////////////////////////////////////////////////////

#include "depthWriteTransition.h"
#include "depthWriteAttribute.h"

TypeHandle DepthWriteTransition::_type_handle;

////////////////////////////////////////////////////////////////////
//     Function: DepthWriteTransition::make_copy
//       Access: Public, Virtual
//  Description: Returns a newly allocated DepthWriteTransition just like
//               this one.
////////////////////////////////////////////////////////////////////
NodeTransition *DepthWriteTransition::
make_copy() const {
  return new DepthWriteTransition(*this);
}

////////////////////////////////////////////////////////////////////
//     Function: DepthWriteTransition::make_attrib
//       Access: Public, Virtual
//  Description: Returns a newly allocated DepthWriteAttribute.
////////////////////////////////////////////////////////////////////
NodeAttribute *DepthWriteTransition::
make_attrib() const {
  return new DepthWriteAttribute;
}
