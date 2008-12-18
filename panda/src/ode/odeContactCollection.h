// Filename: odeContactCollection.h
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

#ifndef ODECONTACTCOLLECTION_H
#define ODECONTACTCOLLECTION_H

#include "odeContact.h"

////////////////////////////////////////////////////////////////////
//       Class : OdeContactCollection
// Description : This is a set of zero or more OdeContacts. It's
//               handy for returning from functions that need to
//               return multiple OdeContacts.
////////////////////////////////////////////////////////////////////
class EXPCL_PANDAODE OdeContactCollection {
PUBLISHED:
  OdeContactCollection();
  OdeContactCollection(const OdeContactCollection &copy);
  void operator = (const OdeContactCollection &copy);
  INLINE ~OdeContactCollection();

  void add_contact(PT(OdeContact) contact);
  bool remove_contact(PT(OdeContact) contact);
  void add_contacts_from(const OdeContactCollection &other);
  void remove_contacts_from(const OdeContactCollection &other);
  void remove_duplicate_contacts();
  bool has_contact(PT(OdeContact) contact) const;
  INLINE void clear();

  INLINE bool is_empty() const;
  INLINE int get_num_contacts() const;
  PT(OdeContact) get_contact(int index) const;
  MAKE_SEQ(get_contacts, get_num_contacts, get_contact);
  PT(OdeContact) operator [] (int index) const;
  INLINE int size() const;
  INLINE void operator += (const OdeContactCollection &other);
  INLINE OdeContactCollection operator + (const OdeContactCollection &other) const;

private:  
  typedef pvector<PT(OdeContact)> Contacts;
  Contacts _contacts;
};

#include "odeContactCollection.I"
#endif

