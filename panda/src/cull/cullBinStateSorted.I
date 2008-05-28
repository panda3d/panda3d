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
  const RenderState *sa = _object->_state;
  const RenderState *sb = other._object->_state;

  if (sa != sb) {
    // First, group objects by texture, since conventional wisdom is
    // that texture changes are the most expensive state changes in a
    // graphics context.
    const TextureAttrib *ta = sa->get_texture();
    const TextureAttrib *tb = sb->get_texture();
    if (ta != tb) {
      return ta < tb;
    }
  }

  // Then group objects by transform, since these are supposed to be
  // expensive too.
  if (_object->_modelview_transform != other._object->_modelview_transform) {
    return _object->_modelview_transform < other._object->_modelview_transform;
  }
      
  // Then, sort by all the other states, in no particular order,
  // just as long as objects with identical state are all grouped
  // together.
  return sa < sb;
}

