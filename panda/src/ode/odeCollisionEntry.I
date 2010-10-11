// Filename: odeCollisionEntry.cxx
// Created by:  pro-rsoft (13Mar09)
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
//     Function: OdeCollisionEntry::Constructor
//       Access: Private
//  Description: 
////////////////////////////////////////////////////////////////////
INLINE OdeCollisionEntry::
OdeCollisionEntry() {
}

////////////////////////////////////////////////////////////////////
//     Function: OdeCollisionEntry::get_geom1
//       Access: Published
//  Description: Returns the first geom in the collision.
////////////////////////////////////////////////////////////////////
INLINE OdeGeom OdeCollisionEntry::
get_geom1() const {
  return OdeGeom(_geom1);
}

////////////////////////////////////////////////////////////////////
//     Function: OdeCollisionEntry::get_geom2
//       Access: Published
//  Description: Returns the second geom in the collision.
////////////////////////////////////////////////////////////////////
INLINE OdeGeom OdeCollisionEntry::
get_geom2() const {
  return OdeGeom(_geom2);
}

////////////////////////////////////////////////////////////////////
//     Function: OdeCollisionEntry::get_body1
//       Access: Published
//  Description: Returns the first body in the collision.
////////////////////////////////////////////////////////////////////
INLINE OdeBody OdeCollisionEntry::
get_body1() const {
  return OdeBody(_body1);
}

////////////////////////////////////////////////////////////////////
//     Function: OdeCollisionEntry::get_body2
//       Access: Published
//  Description: Returns the second body in the collision.
////////////////////////////////////////////////////////////////////
INLINE OdeBody OdeCollisionEntry::
get_body2() const {
  return OdeBody(_body2);
}

////////////////////////////////////////////////////////////////////
//     Function: OdeCollisionEntry::get_num_contacts
//       Access: Published
//  Description: Returns the number of contacts in the collision.
////////////////////////////////////////////////////////////////////
INLINE size_t OdeCollisionEntry::
get_num_contacts() const {
  return _num_contacts;
}

////////////////////////////////////////////////////////////////////
//     Function: OdeCollisionEntry::get_contact_geom
//       Access: Published
//  Description: Returns the nth contact geom in the collision.
////////////////////////////////////////////////////////////////////
INLINE OdeContactGeom OdeCollisionEntry::
get_contact_geom(size_t n) const {
  nassertr(n >= 0 && n < _num_contacts, OdeContactGeom());
  return _contact_geoms[n];
}

////////////////////////////////////////////////////////////////////
//     Function: OdeCollisionEntry::operator []
//       Access: Published
//  Description: Returns the nth contact geom in the collision.
////////////////////////////////////////////////////////////////////
INLINE OdeContactGeom OdeCollisionEntry::
operator [] (size_t n) const {
  nassertr(n >= 0 && n < _num_contacts, OdeContactGeom());
  return _contact_geoms[n];
}

////////////////////////////////////////////////////////////////////
//     Function: OdeCollisionEntry::get_contact_point
//       Access: Published
//  Description: Returns the nth contact point in the collision.
//               This does exactly the same as
//               get_contact_geom(n).get_pos().
////////////////////////////////////////////////////////////////////
INLINE LPoint3f OdeCollisionEntry::
get_contact_point(size_t n) const {
  nassertr(n >= 0 && n < _num_contacts, LPoint3f::zero());
  return _contact_geoms[n].get_pos();
}

////////////////////////////////////////////////////////////////////
//     Function: OdeCollisionEntry::operator bool
//       Access: Published
//  Description: An OdeCollisionEntry evaluates to False if it
//               holds no contacts.
////////////////////////////////////////////////////////////////////
INLINE OdeCollisionEntry::
operator bool () const {
  return (_num_contacts != 0);
}

////////////////////////////////////////////////////////////////////
//     Function: OdeCollisionEntry::is_empty
//       Access: Published
//  Description: Returns true if the entry holds no contacts.
////////////////////////////////////////////////////////////////////
INLINE bool OdeCollisionEntry::
is_empty() const {
  return (_num_contacts == 0);
}

