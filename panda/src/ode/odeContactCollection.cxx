// Filename: odeContactCollection.cxx
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

#include "odeContactCollection.h"

OdeContactCollection::
OdeContactCollection() {
}

OdeContactCollection::
OdeContactCollection(const OdeContactCollection &copy) :
  _contacts(copy._contacts) {
}

void OdeContactCollection::
operator = (const OdeContactCollection &copy) {
  _contacts = copy._contacts;
}

void OdeContactCollection::
add_contact(PT(OdeContact) contact) {
  _contacts.push_back(contact);
}

bool OdeContactCollection::
remove_contact(PT(OdeContact) contact) {
  int contact_index = -1;
  for (int i = 0; contact_index == -1 && i < (int)_contacts.size(); i++) {
    if (_contacts[i] == contact) {
      contact_index = i;
    }
  }

  if (contact_index == -1) {
    // The indicated contact was not a member of the collection.
    return false;
  }

  _contacts.erase(_contacts.begin() + contact_index);
  return true;
}

void OdeContactCollection::
add_contacts_from(const OdeContactCollection &other) {
  int other_num_contacts = other.get_num_contacts();
  for (int i = 0; i < other_num_contacts; i++) {
    add_contact(other.get_contact(i));
  }
}

void OdeContactCollection::
remove_contacts_from(const OdeContactCollection &other) {
  Contacts new_contacts;
  int num_contacts = get_num_contacts();
  for (int i = 0; i < num_contacts; i++) {
    PT(OdeContact) contact = get_contact(i);
    if (!other.has_contact(contact)) {
      new_contacts.push_back(contact);
    }
  }
  _contacts = new_contacts;
}

void OdeContactCollection::
remove_duplicate_contacts() {
  Contacts new_contacts;

  int num_contacts = get_num_contacts();
  for (int i = 0; i < num_contacts; i++) {
    PT(OdeContact) contact = get_contact(i);
    bool duplicated = false;

    for (int j = 0; j < i && !duplicated; j++) {
      duplicated = (contact == get_contact(j));
    }

    if (!duplicated) {
      new_contacts.push_back(contact);
    }
  }

  _contacts = new_contacts;
}

bool OdeContactCollection::
has_contact(PT(OdeContact) contact) const {
  for (int i = 0; i < get_num_contacts(); i++) {
    if (contact == get_contact(i)) {
      return true;
    }
  }
  return false;
}

PT(OdeContact) OdeContactCollection::
get_contact(int index) const {
  nassertr(index >= 0 && index < (int)_contacts.size(), NULL);
  return _contacts[index];
}

PT(OdeContact) OdeContactCollection::
operator [] (int index) const {
  return get_contact(index);
}

