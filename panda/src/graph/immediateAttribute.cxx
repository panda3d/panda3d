// Filename: immediateAttribute.cxx
// Created by:  drose (24Mar00)
// 
////////////////////////////////////////////////////////////////////

#include "immediateAttribute.h"
#include "immediateTransition.h"

#include <indent.h>

TypeHandle ImmediateAttribute::_type_handle;

////////////////////////////////////////////////////////////////////
//     Function: ImmediateAttribute::make_copy
//       Access: Public, Virtual
//  Description: 
////////////////////////////////////////////////////////////////////
NodeAttribute *ImmediateAttribute::
make_copy() const {
  return new ImmediateAttribute(*this);
}

////////////////////////////////////////////////////////////////////
//     Function: ImmediateAttribute::make_initial
//       Access: Public, Virtual
//  Description: 
////////////////////////////////////////////////////////////////////
NodeAttribute *ImmediateAttribute::
make_initial() const {
  return new ImmediateAttribute;
}

////////////////////////////////////////////////////////////////////
//     Function: ImmediateAttribute::get_handle
//       Access: Public, Virtual
//  Description: 
////////////////////////////////////////////////////////////////////
TypeHandle ImmediateAttribute::
get_handle() const {
  return ImmediateTransition::get_class_type();
}

////////////////////////////////////////////////////////////////////
//     Function: ImmediateAttribute::internal_compare_to
//       Access: Protected, Virtual
//  Description: 
////////////////////////////////////////////////////////////////////
int ImmediateAttribute::
internal_compare_to(const NodeAttribute *) const {
  return 0;
}
