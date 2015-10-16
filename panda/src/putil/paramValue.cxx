// Filename: paramValue.cxx
// Created by:  drose (08Feb99)
//
////////////////////////////////////////////////////////////////////
//
// PANDA 3D SOFTWARE
// Copyright (c) Carnegie Mellon University.  All rights reserved.
//
// All use of this software is subject to the terms of the revised BSD
// license.  You should have received a copy of this license along
// with this source code in a file named "LICENSE."
//
////////////////////////////////////////////////////////////////////

#include "paramValue.h"
#include "dcast.h"

// Tell GCC that we'll take care of the instantiation explicitly here.
#ifdef __GNUC__
#pragma implementation
#endif

template class ParamValue<std::string>;
template class ParamValue<std::wstring>;

template class ParamValue<LVecBase2d>;
template class ParamValue<LVecBase2f>;
template class ParamValue<LVecBase2i>;

template class ParamValue<LVecBase3d>;
template class ParamValue<LVecBase3f>;
template class ParamValue<LVecBase3i>;

template class ParamValue<LVecBase4d>;
template class ParamValue<LVecBase4f>;
template class ParamValue<LVecBase4i>;

template class ParamValue<LMatrix3d>;
template class ParamValue<LMatrix3f>;

template class ParamValue<LMatrix4d>;
template class ParamValue<LMatrix4f>;

TypeHandle ParamValueBase::_type_handle;
TypeHandle ParamTypedRefCount::_type_handle;

////////////////////////////////////////////////////////////////////
//     Function: ParamValueBase::Destructor
//       Access: Published, Virtual
//  Description:
////////////////////////////////////////////////////////////////////
ParamValueBase::
~ParamValueBase() {
}

////////////////////////////////////////////////////////////////////
//     Function: ParamTypedRefCount::Destructor
//       Access: Published, Virtual
//  Description:
////////////////////////////////////////////////////////////////////
ParamTypedRefCount::
~ParamTypedRefCount() {
}

////////////////////////////////////////////////////////////////////
//     Function: ParamTypedRefCount::output
//       Access: Published, Virtual
//  Description:
////////////////////////////////////////////////////////////////////
void ParamTypedRefCount::
output(ostream &out) const {
  if (_value == (TypedReferenceCount *)NULL) {
    out << "(empty)";

  } else {
    out << _value->get_type();
  }
}
