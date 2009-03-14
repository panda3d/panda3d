// Filename: geomDrawCallbackData.I
// Created by:  drose (13Mar09)
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
//     Function: GeomDrawCallbackData::Constructor
//       Access: Public
//  Description:
////////////////////////////////////////////////////////////////////
INLINE GeomDrawCallbackData::
GeomDrawCallbackData(CullableObject *obj, GraphicsStateGuardianBase *gsg, 
                     bool force) :
  _obj(obj),
  _gsg(gsg),
  _force(force),
  _lost_state(true)
{
}

////////////////////////////////////////////////////////////////////
//     Function: GeomDrawCallbackData::get_object
//       Access: Published
//  Description: Returns a pointer to the particular object that is
//               being drawn.
////////////////////////////////////////////////////////////////////
INLINE CullableObject *GeomDrawCallbackData::
get_object() const {
  return _obj;
}

////////////////////////////////////////////////////////////////////
//     Function: GeomDrawCallbackData::get_gsg
//       Access: Published
//  Description: Returns a pointer to the current GSG.
////////////////////////////////////////////////////////////////////
INLINE GraphicsStateGuardianBase *GeomDrawCallbackData::
get_gsg() const {
  return _gsg;
}

////////////////////////////////////////////////////////////////////
//     Function: GeomDrawCallbackData::get_force
//       Access: Published
//  Description: Returns true if any required data should be forced
//               into memory if necessary to render the object, or
//               false if the object should be omitted if some of the
//               data is not available (at least until the data
//               becomes available later).
////////////////////////////////////////////////////////////////////
INLINE bool GeomDrawCallbackData::
get_force() const {
  return _force;
}

////////////////////////////////////////////////////////////////////
//     Function: GeomDrawCallbackData::set_lost_state
//       Access: Published
//  Description: Sets the lost_state flag.  If this is true, the
//               callback does not have to be quite so careful to
//               clean up after itself; Panda will assume that the
//               graphics state is in an unknown state after the
//               callback has finished, and will issue all the
//               necessary calls to restore it.  If this is false,
//               Panda will assume the callback will leave the
//               graphics state exactly as it came in, and won't
//               bother to try to restore it.  The default is true.
////////////////////////////////////////////////////////////////////
INLINE void GeomDrawCallbackData::
set_lost_state(bool lost_state) {
  _lost_state = lost_state;
}

////////////////////////////////////////////////////////////////////
//     Function: GeomDrawCallbackData::get_lost_state
//       Access: Published
//  Description: Returns the lost_state flag.  See set_lost_state().
////////////////////////////////////////////////////////////////////
INLINE bool GeomDrawCallbackData::
get_lost_state() const {
  return _lost_state;
}
