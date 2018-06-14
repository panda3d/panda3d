/**
 * PANDA 3D SOFTWARE
 * Copyright (c) Carnegie Mellon University.  All rights reserved.
 *
 * All use of this software is subject to the terms of the revised BSD
 * license.  You should have received a copy of this license along
 * with this source code in a file named "LICENSE."
 *
 * @file internalNameCollection.cxx
 * @author drose
 * @date 2002-03-16
 */

#include "internalNameCollection.h"

#include "indent.h"

/**
 *
 */
InternalNameCollection::
InternalNameCollection() {
}

/**
 *
 */
InternalNameCollection::
InternalNameCollection(const InternalNameCollection &copy) :
  _names(copy._names)
{
}

/**
 *
 */
void InternalNameCollection::
operator = (const InternalNameCollection &copy) {
  _names = copy._names;
}

/**
 * Adds a new InternalName to the collection.
 */
void InternalNameCollection::
add_name(const InternalName *name) {
  // If the pointer to our internal array is shared by any other
  // InternalNameCollections, we have to copy the array now so we won't
  // inadvertently modify any of our brethren InternalNameCollection objects.

  if (_names.get_ref_count() > 1) {
    InternalNames old_names = _names;
    _names = InternalNames::empty_array(0);
    _names.v() = old_names.v();
  }

  _names.push_back(name);
}

/**
 * Removes the indicated InternalName from the collection.  Returns true if
 * the name was removed, false if it was not a member of the collection.
 */
bool InternalNameCollection::
remove_name(const InternalName *name) {
  int name_index = -1;
  for (int i = 0; name_index == -1 && i < (int)_names.size(); i++) {
    if (_names[i] == name) {
      name_index = i;
    }
  }

  if (name_index == -1) {
    // The indicated name was not a member of the collection.
    return false;
  }

  // If the pointer to our internal array is shared by any other
  // InternalNameCollections, we have to copy the array now so we won't
  // inadvertently modify any of our brethren InternalNameCollection objects.

  if (_names.get_ref_count() > 1) {
    InternalNames old_names = _names;
    _names = InternalNames::empty_array(0);
    _names.v() = old_names.v();
  }

  _names.erase(_names.begin() + name_index);
  return true;
}

/**
 * Adds all the InternalNames indicated in the other collection to this name.
 * The other names are simply appended to the end of the names in this list;
 * duplicates are not automatically removed.
 */
void InternalNameCollection::
add_names_from(const InternalNameCollection &other) {
  int other_num_names = other.get_num_names();
  for (int i = 0; i < other_num_names; i++) {
    add_name(other.get_name(i));
  }
}


/**
 * Removes from this collection all of the InternalNames listed in the other
 * collection.
 */
void InternalNameCollection::
remove_names_from(const InternalNameCollection &other) {
  InternalNames new_names;
  int num_names = get_num_names();
  for (int i = 0; i < num_names; i++) {
    const InternalName *name = get_name(i);
    if (!other.has_name(name)) {
      new_names.push_back(name);
    }
  }
  _names = new_names;
}

/**
 * Removes any duplicate entries of the same InternalNames on this collection.
 * If a InternalName appears multiple times, the first appearance is retained;
 * subsequent appearances are removed.
 */
void InternalNameCollection::
remove_duplicate_names() {
  InternalNames new_names;

  int num_names = get_num_names();
  for (int i = 0; i < num_names; i++) {
    const InternalName *name = get_name(i);
    bool duplicated = false;

    for (int j = 0; j < i && !duplicated; j++) {
      duplicated = (name == get_name(j));
    }

    if (!duplicated) {
      new_names.push_back(name);
    }
  }

  _names = new_names;
}

/**
 * Returns true if the indicated InternalName appears in this collection,
 * false otherwise.
 */
bool InternalNameCollection::
has_name(const InternalName *name) const {
  for (int i = 0; i < get_num_names(); i++) {
    if (name == get_name(i)) {
      return true;
    }
  }
  return false;
}

/**
 * Removes all InternalNames from the collection.
 */
void InternalNameCollection::
clear() {
  _names.clear();
}

/**
 * Returns the number of InternalNames in the collection.
 */
int InternalNameCollection::
get_num_names() const {
  return _names.size();
}

/**
 * Returns the nth InternalName in the collection.
 */
const InternalName *InternalNameCollection::
get_name(int index) const {
  nassertr(index >= 0 && index < (int)_names.size(), nullptr);

  return _names[index];
}

/**
 * Returns the nth InternalName in the collection.  This is the same as
 * get_name(), but it may be a more convenient way to access it.
 */
const InternalName *InternalNameCollection::
operator [] (int index) const {
  nassertr(index >= 0 && index < (int)_names.size(), nullptr);

  return _names[index];
}

/**
 * Returns the number of names in the collection.  This is the same thing as
 * get_num_names().
 */
int InternalNameCollection::
size() const {
  return _names.size();
}

/**
 * Writes a brief one-line description of the InternalNameCollection to the
 * indicated output stream.
 */
void InternalNameCollection::
output(std::ostream &out) const {
  if (get_num_names() == 1) {
    out << "1 InternalName";
  } else {
    out << get_num_names() << " InternalNames";
  }
}

/**
 * Writes a complete multi-line description of the InternalNameCollection to
 * the indicated output stream.
 */
void InternalNameCollection::
write(std::ostream &out, int indent_level) const {
  for (int i = 0; i < get_num_names(); i++) {
    indent(out, indent_level) << *get_name(i) << "\n";
  }
}
