// Filename: odeContactCollection.I
// Created by:  pro-rsoft (17Dec08)
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
//     Function: OdeContactCollection::Destructor
//       Access: Published
//  Description:
////////////////////////////////////////////////////////////////////
INLINE OdeContactCollection::
~OdeContactCollection() {
}

////////////////////////////////////////////////////////////////////
//     Function: OdeContactCollection::operator +=
//       Access: Published
//  Description: Appends the other list onto the end of this one.
////////////////////////////////////////////////////////////////////
INLINE void OdeContactCollection::
operator += (const OdeContactCollection &other) {
  add_contacts_from(other);
}

////////////////////////////////////////////////////////////////////
//     Function: OdeContactCollection::operator +
//       Access: Published
//  Description: Returns a OdeContactCollection representing the
//               concatenation of the two lists.
////////////////////////////////////////////////////////////////////
INLINE OdeContactCollection OdeContactCollection::
operator + (const OdeContactCollection &other) const {
  OdeContactCollection a(*this);
  a += other;
  return a;
}

////////////////////////////////////////////////////////////////////
//     Function: OdeContactCollection::clear
//       Access: Published
//  Description: Clears the contact collection.
////////////////////////////////////////////////////////////////////
INLINE void OdeContactCollection::
clear() {
  _contacts.clear();
}

////////////////////////////////////////////////////////////////////
//     Function: OdeContactCollection::is_empty
//       Access: Published
//  Description: Returns true if the collection is empty.
////////////////////////////////////////////////////////////////////
INLINE bool OdeContactCollection::
is_empty() const {
  return _contacts.empty();
}

////////////////////////////////////////////////////////////////////
//     Function: OdeContactCollection::get_num_contacts
//       Access: Published
//  Description: Returns the number of Contacts in the collection.
//               This is the same thing as size().
////////////////////////////////////////////////////////////////////
INLINE int OdeContactCollection::
get_num_contacts() const {
  return _contacts.size();
}

////////////////////////////////////////////////////////////////////
//     Function: OdeContactCollection::size
//       Access: Published
//  Description: Returns the number of Contacts in the collection.
//               This is the same thing as get_num_contacts().
////////////////////////////////////////////////////////////////////
INLINE int OdeContactCollection::
size() const {
  return _contacts.size();
}

