// Filename: nullTransitionWrapper.I
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

#include "nodeRelation.h"
#include "nodeTransitionCacheEntry.h"

////////////////////////////////////////////////////////////////////
//     Function: NullTransitionWrapper::Constructor
//       Access: Public
//  Description:
////////////////////////////////////////////////////////////////////
INLINE NullTransitionWrapper::
NullTransitionWrapper() {
}

////////////////////////////////////////////////////////////////////
//     Function: NullTransitionWrapper::Copy Constructor
//       Access: Public
//  Description:
////////////////////////////////////////////////////////////////////
INLINE NullTransitionWrapper::
NullTransitionWrapper(const NullTransitionWrapper &) {
}

////////////////////////////////////////////////////////////////////
//     Function: NullTransitionWrapper::Copy Assignment Operator
//       Access: Public
//  Description:
////////////////////////////////////////////////////////////////////
INLINE void NullTransitionWrapper::
operator = (const NullTransitionWrapper &) {
}

////////////////////////////////////////////////////////////////////
//     Function: NullTransitionWrapper::init_from
//       Access: Public, Static
//  Description:
////////////////////////////////////////////////////////////////////
INLINE NullTransitionWrapper NullTransitionWrapper::
init_from(const NullTransitionWrapper &) {
  return NullTransitionWrapper();
}

////////////////////////////////////////////////////////////////////
//     Function: NullTransitionWrapper::is_identity
//       Access: Public
//  Description:
////////////////////////////////////////////////////////////////////
INLINE bool NullTransitionWrapper::
is_identity() const {
  return true;
}

////////////////////////////////////////////////////////////////////
//     Function: NullTransitionWrapper::compare_to
//       Access: Public
//  Description:
////////////////////////////////////////////////////////////////////
INLINE int NullTransitionWrapper::
compare_to(const NullTransitionWrapper &) const {
  return 0;
}

////////////////////////////////////////////////////////////////////
//     Function: NullTransitionWrapper::make_identity
//       Access: Public
//  Description:
////////////////////////////////////////////////////////////////////
INLINE void NullTransitionWrapper::
make_identity() {
}

////////////////////////////////////////////////////////////////////
//     Function: NullTransitionWrapper::extract_from
//       Access: Public
//  Description:
////////////////////////////////////////////////////////////////////
INLINE void NullTransitionWrapper::
extract_from(const NodeRelation *) {
}

////////////////////////////////////////////////////////////////////
//     Function: NullTransitionWrapper::store_to
//       Access: Public
//  Description:
////////////////////////////////////////////////////////////////////
INLINE void NullTransitionWrapper::
store_to(NodeRelation *) const {
}

////////////////////////////////////////////////////////////////////
//     Function: NullTransitionWrapper::compose_in_place
//       Access: Public
//  Description:
////////////////////////////////////////////////////////////////////
INLINE void NullTransitionWrapper::
compose_in_place(const NullTransitionWrapper &) {
}

////////////////////////////////////////////////////////////////////
//     Function: NullTransitionWrapper::invert_in_place
//       Access: Public
//  Description:
////////////////////////////////////////////////////////////////////
INLINE void NullTransitionWrapper::
invert_in_place() {
}

////////////////////////////////////////////////////////////////////
//     Function: NullTransitionWrapper::invert_compose_in_place
//       Access: Public
//  Description:
////////////////////////////////////////////////////////////////////
INLINE void NullTransitionWrapper::
invert_compose_in_place(const NullTransitionWrapper &) {
}

////////////////////////////////////////////////////////////////////
//     Function: NullTransitionWrapper::extract_from_cache
//       Access: Public
//  Description:
////////////////////////////////////////////////////////////////////
INLINE Node *NullTransitionWrapper::
extract_from_cache(const NodeRelation *) {
  return NULL;
}

////////////////////////////////////////////////////////////////////
//     Function: NullTransitionWrapper::store_to_cache
//       Access: Public
//  Description: Stores this transition into the arc's cache, and
//               updates the arc's top_subtree.
////////////////////////////////////////////////////////////////////
INLINE void NullTransitionWrapper::
store_to_cache(NodeRelation *, Node *) {
}

////////////////////////////////////////////////////////////////////
//     Function: NullTransitionWrapper::is_cache_verified
//       Access: Public
//  Description:
////////////////////////////////////////////////////////////////////
INLINE bool NullTransitionWrapper::
is_cache_verified(UpdateSeq) const {
  return true;
}

////////////////////////////////////////////////////////////////////
//     Function: NullTransitionWrapper::set_computed_verified
//       Access: Public
//  Description:
////////////////////////////////////////////////////////////////////
INLINE void NullTransitionWrapper::
set_computed_verified(UpdateSeq) {
}

////////////////////////////////////////////////////////////////////
//     Function: NullTransitionWrapper::cached_compose
//       Access: Public
//  Description: Computes the composition of this wrapper's value with
//               that indicated by value, using the cache as a helper,
//               and stores the result in this wrapper.
////////////////////////////////////////////////////////////////////
INLINE void NullTransitionWrapper::
cached_compose(const NullTransitionWrapper &,
               const NullTransitionWrapper &,
               UpdateSeq) {
}

////////////////////////////////////////////////////////////////////
//     Function: NullTransitionWrapper::output
//       Access: Public, Virtual
//  Description:
////////////////////////////////////////////////////////////////////
INLINE void NullTransitionWrapper::
output(ostream &) const {
}

////////////////////////////////////////////////////////////////////
//     Function: NullTransitionWrapper::write
//       Access: Public, Virtual
//  Description:
////////////////////////////////////////////////////////////////////
INLINE void NullTransitionWrapper::
write(ostream &, int) const {
}

INLINE ostream &operator << (ostream &out, const NullTransitionWrapper &ntw) {
  ntw.output(out);
  return out;
}
