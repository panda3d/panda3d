// Filename: nullAttributeWrapper.I
// Created by:  drose (22Mar00)
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

////////////////////////////////////////////////////////////////////
//     Function: NullAttributeWrapper::Constructor
//       Access: Public
//  Description:
////////////////////////////////////////////////////////////////////
INLINE_GRAPH NullAttributeWrapper::
NullAttributeWrapper() {
}

////////////////////////////////////////////////////////////////////
//     Function: NullAttributeWrapper::Copy Constructor
//       Access: Public
//  Description:
////////////////////////////////////////////////////////////////////
INLINE_GRAPH NullAttributeWrapper::
NullAttributeWrapper(const NullAttributeWrapper &) {
}

////////////////////////////////////////////////////////////////////
//     Function: NullAttributeWrapper::Copy Assignment Operator
//       Access: Public
//  Description:
////////////////////////////////////////////////////////////////////
INLINE_GRAPH void NullAttributeWrapper::
operator = (const NullAttributeWrapper &) {
}

////////////////////////////////////////////////////////////////////
//     Function: NullAttributeWrapper::init_from
//       Access: Public, Static
//  Description:
////////////////////////////////////////////////////////////////////
INLINE_GRAPH NullAttributeWrapper NullAttributeWrapper::
init_from(const NullTransitionWrapper &) {
  return NullAttributeWrapper();
}

////////////////////////////////////////////////////////////////////
//     Function: NullAttributeWrapper::is_initial
//       Access: Public
//  Description:
////////////////////////////////////////////////////////////////////
INLINE_GRAPH bool NullAttributeWrapper::
is_initial() const {
  return true;
}

////////////////////////////////////////////////////////////////////
//     Function: NullAttributeWrapper::compare_to
//       Access: Public
//  Description:
////////////////////////////////////////////////////////////////////
INLINE_GRAPH int NullAttributeWrapper::
compare_to(const NullAttributeWrapper &) const {
  return 0;
}

////////////////////////////////////////////////////////////////////
//     Function: NullAttributeWrapper::make_initial
//       Access: Public
//  Description:
////////////////////////////////////////////////////////////////////
INLINE_GRAPH void NullAttributeWrapper::
make_initial() {
}

////////////////////////////////////////////////////////////////////
//     Function: NullAttributeWrapper::apply_in_place
//       Access: Public
//  Description:
////////////////////////////////////////////////////////////////////
INLINE_GRAPH void NullAttributeWrapper::
apply_in_place(const NullTransitionWrapper &) {
}

////////////////////////////////////////////////////////////////////
//     Function: NullAttributeWrapper::output
//       Access: Public, Virtual
//  Description:
////////////////////////////////////////////////////////////////////
INLINE_GRAPH void NullAttributeWrapper::
output(ostream &) const {
}

////////////////////////////////////////////////////////////////////
//     Function: NullAttributeWrapper::write
//       Access: Public, Virtual
//  Description:
////////////////////////////////////////////////////////////////////
INLINE_GRAPH void NullAttributeWrapper::
write(ostream &, int) const {
}

INLINE_GRAPH ostream &operator << (ostream &out, const NullAttributeWrapper &naw) {
  naw.output(out);
  return out;
}

