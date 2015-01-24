// Filename: cullBinStateSorted.I
// Created by:  drose (22Mar05)
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


////////////////////////////////////////////////////////////////////
//     Function: CullBinStateSorted::Constructor
//       Access: Public
//  Description: 
////////////////////////////////////////////////////////////////////
INLINE CullBinStateSorted::
CullBinStateSorted(const string &name, GraphicsStateGuardianBase *gsg,
                   const PStatCollector &draw_region_pcollector) :
  CullBin(name, BT_state_sorted, gsg, draw_region_pcollector),
  _objects(get_class_type())
{
}

////////////////////////////////////////////////////////////////////
//     Function: CullBinStateSorted::ObjectData::Constructor
//       Access: Public
//  Description: 
////////////////////////////////////////////////////////////////////
INLINE CullBinStateSorted::ObjectData::
ObjectData(CullableObject *object) :
  _object(object)
{
}

////////////////////////////////////////////////////////////////////
//     Function: CullBinStateSorted::ObjectData::operator <
//       Access: Public
//  Description: Specifies the correct sort ordering for these
//               objects.
////////////////////////////////////////////////////////////////////
INLINE bool CullBinStateSorted::ObjectData::
operator < (const ObjectData &other) const {
  // First group objects by transform, since transform changes are
  // supposed to be expensive.
  if (_object->_internal_transform != other._object->_internal_transform) {
    return _object->_internal_transform < other._object->_internal_transform;
  }

  // Then group by other state changes, in approximate order from
  // heaviest change to lightest change.
  const RenderState *sa = _object->_state;
  const RenderState *sb = other._object->_state;
  return sa->compare_sort(*sb) < 0;
}

