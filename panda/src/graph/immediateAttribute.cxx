// Filename: immediateAttribute.cxx
// Created by:  drose (24Mar00)
//
////////////////////////////////////////////////////////////////////
//
// PANDA 3D SOFTWARE
// Copyright (c) 2001, Disney Enterprises, Inc.  All rights reserved
//
// All use of this software is subject to the terms of the Panda 3d
// Software license.  You should have received a copy of this license
// along with this source code; you will also find a current copy of
// the license at http://www.panda3d.org/license.txt .
//
// To contact the maintainers of this program write to
// panda3d@yahoogroups.com .
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
