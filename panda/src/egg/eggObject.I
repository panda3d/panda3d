// Filename: eggObject.I
// Created by:  drose (10Feb99)
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
//     Function: EggObject::Constructor
//       Access: Public
//  Description:
////////////////////////////////////////////////////////////////////
INLINE EggObject::
EggObject() {
}


////////////////////////////////////////////////////////////////////
//     Function: EggObject::Copy constructor
//       Access: Public
//  Description:
////////////////////////////////////////////////////////////////////
INLINE EggObject::
EggObject(const EggObject &copy) : 
  TypedReferenceCount(copy),
  _user_data(copy._user_data)
{
}


////////////////////////////////////////////////////////////////////
//     Function: EggObject::Copy assignment operator
//       Access: Public
//  Description:
////////////////////////////////////////////////////////////////////
INLINE EggObject &EggObject::
operator = (const EggObject &copy) {
  TypedReferenceCount::operator = (copy);
  _user_data = copy._user_data;
  return *this;
}

////////////////////////////////////////////////////////////////////
//     Function: EggObject::set_user_data
//       Access: Public
//  Description: Sets the user data associated with this object.  This
//               may be any EggUserData-derived object.  The egg
//               library will do nothing with this pointer, except to
//               hold its reference count and return the pointer on
//               request.
//
//               This pointer is also copied by the copy assignment
//               operator and copy constructor.
////////////////////////////////////////////////////////////////////
INLINE void EggObject::
set_user_data(EggUserData *user_data) {
  _user_data = user_data;
}

////////////////////////////////////////////////////////////////////
//     Function: EggObject::get_user_data
//       Access: Public
//  Description: Returns the user data pointer previously stored on
//               this object, or NULL if nothing was previously
//               stored.
////////////////////////////////////////////////////////////////////
INLINE EggUserData *EggObject::
get_user_data() const {
  return _user_data;
}

////////////////////////////////////////////////////////////////////
//     Function: EggObject::has_user_data
//       Access: Public
//  Description: Returns true if the user data pointer has been set,
//               false otherwise.
////////////////////////////////////////////////////////////////////
INLINE bool EggObject::
has_user_data() const {
  return !_user_data.is_null();
}

////////////////////////////////////////////////////////////////////
//     Function: EggObject::has_user_data
//       Access: Public
//  Description: Returns true if the user data pointer has been set
//               and is of the indicated type, false otherwise.
////////////////////////////////////////////////////////////////////
INLINE bool EggObject::
has_user_data(TypeHandle type) const {
  return !_user_data.is_null() && _user_data->is_of_type(type);
}

////////////////////////////////////////////////////////////////////
//     Function: EggObject::clear_user_data
//       Access: Public
//  Description: Resets the user data pointer to NULL.
////////////////////////////////////////////////////////////////////
INLINE void EggObject::
clear_user_data() {
  _user_data.clear();
}
