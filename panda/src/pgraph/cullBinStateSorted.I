// Filename: cullBinStateSorted.I
// Created by:  drose (22Mar05)
//
////////////////////////////////////////////////////////////////////
//
// PANDA 3D SOFTWARE
// Copyright (c) 2001 - 2004, Disney Enterprises, Inc.  All rights reserved
//
// All use of this software is subject to the terms of the Panda 3d
// Software license.  You should have received a copy of this license
// along with this source code; you will also find a current copy of
// the license at http://etc.cmu.edu/panda3d/docs/license/ .
//
// To contact the maintainers of this program write to
// panda3d-general@lists.sourceforge.net .
//
////////////////////////////////////////////////////////////////////


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
  if (_object->_transform != other._object->_transform) {
    return _object->_transform < other._object->_transform;
  }
      
  // Then, sort by all the other states, in no particular order,
  // just as long as objects with identical state are all grouped
  // together.
  return sa < sb;
}


////////////////////////////////////////////////////////////////////
//     Function: CullBinStateSorted::Constructor
//       Access: Public
//  Description: 
////////////////////////////////////////////////////////////////////
INLINE CullBinStateSorted::
CullBinStateSorted(const string &name, GraphicsStateGuardianBase *gsg) :
  CullBin(name, gsg)
{
}
