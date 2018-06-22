/**
 * PANDA 3D SOFTWARE
 * Copyright (c) Carnegie Mellon University.  All rights reserved.
 *
 * All use of this software is subject to the terms of the revised BSD
 * license.  You should have received a copy of this license along
 * with this source code in a file named "LICENSE."
 *
 * @file physicsObjectCollection.cxx
 * @author joswilso
 * @date 2006-07-12
 */

#include "physicsObjectCollection.h"

#include "indent.h"

/**
 *
 */
PhysicsObjectCollection::
PhysicsObjectCollection() {
}

/**
 *
 */
PhysicsObjectCollection::
PhysicsObjectCollection(const PhysicsObjectCollection &copy) :
  _physics_objects(copy._physics_objects)
{
}

/**
 *
 */
void PhysicsObjectCollection::
operator = (const PhysicsObjectCollection &copy) {
  _physics_objects = copy._physics_objects;
}

/**
 * Adds a new PhysicsObject to the collection.
 */
void PhysicsObjectCollection::
add_physics_object(PT(PhysicsObject) physics_object) {
  // If the pointer to our internal array is shared by any other
  // PhysicsObjectCollections, we have to copy the array now so we won't
  // inadvertently modify any of our brethren PhysicsObjectCollection objects.

  if (_physics_objects.get_ref_count() > 1) {
    PhysicsObjects old_physics_objects = _physics_objects;
    _physics_objects = PhysicsObjects::empty_array(0);
    _physics_objects.v() = old_physics_objects.v();
  }

  _physics_objects.push_back(physics_object);
}

/**
 * Removes the indicated PhysicsObject from the collection.  Returns true if
 * the physics_object was removed, false if it was not a member of the
 * collection.
 */
bool PhysicsObjectCollection::
remove_physics_object(PT(PhysicsObject) physics_object) {
  int object_index = -1;
  for (int i = 0; object_index == -1 && i < (int)_physics_objects.size(); i++) {
    if (_physics_objects[i] == physics_object) {
      object_index = i;
    }
  }

  if (object_index == -1) {
    // The indicated physics_object was not a member of the collection.
    return false;
  }

  // If the pointer to our internal array is shared by any other
  // PhysicsObjectCollections, we have to copy the array now so we won't
  // inadvertently modify any of our brethren PhysicsObjectCollection objects.

  if (_physics_objects.get_ref_count() > 1) {
    PhysicsObjects old_physics_objects = _physics_objects;
    _physics_objects = PhysicsObjects::empty_array(0);
    _physics_objects.v() = old_physics_objects.v();
  }

  _physics_objects.erase(_physics_objects.begin() + object_index);
  return true;
}

/**
 * Adds all the PhysicsObjects indicated in the other collection to this
 * collection.  The other physics_objects are simply appended to the end of
 * the physics_objects in this list; duplicates are not automatically removed.
 */
void PhysicsObjectCollection::
add_physics_objects_from(const PhysicsObjectCollection &other) {
  int other_num_physics_objects = other.get_num_physics_objects();
  for (int i = 0; i < other_num_physics_objects; i++) {
    add_physics_object(other.get_physics_object(i));
  }
}


/**
 * Removes from this collection all of the PhysicsObjects listed in the other
 * collection.
 */
void PhysicsObjectCollection::
remove_physics_objects_from(const PhysicsObjectCollection &other) {
  PhysicsObjects new_physics_objects;
  int num_physics_objects = get_num_physics_objects();
  for (int i = 0; i < num_physics_objects; i++) {
    PT(PhysicsObject) physics_object = get_physics_object(i);
    if (!other.has_physics_object(physics_object)) {
      new_physics_objects.push_back(physics_object);
    }
  }
  _physics_objects = new_physics_objects;
}

/**
 * Removes any duplicate entries of the same PhysicsObjects on this
 * collection.  If a PhysicsObject appears multiple times, the first
 * appearance is retained; subsequent appearances are removed.
 */
void PhysicsObjectCollection::
remove_duplicate_physics_objects() {
  PhysicsObjects new_physics_objects;

  int num_physics_objects = get_num_physics_objects();
  for (int i = 0; i < num_physics_objects; i++) {
    PT(PhysicsObject) physics_object = get_physics_object(i);
    bool duplicated = false;

    for (int j = 0; j < i && !duplicated; j++) {
      duplicated = (physics_object == get_physics_object(j));
    }

    if (!duplicated) {
      new_physics_objects.push_back(physics_object);
    }
  }

  _physics_objects = new_physics_objects;
}

/**
 * Returns true if the indicated PhysicsObject appears in this collection,
 * false otherwise.
 */
bool PhysicsObjectCollection::
has_physics_object(PT(PhysicsObject) physics_object) const {
  for (int i = 0; i < get_num_physics_objects(); i++) {
    if (physics_object == get_physics_object(i)) {
      return true;
    }
  }
  return false;
}

/**
 * Removes all PhysicsObjects from the collection.
 */
void PhysicsObjectCollection::
clear() {
  _physics_objects.clear();
}

/**
 * Returns true if there are no PhysicsObjects in the collection, false
 * otherwise.
 */
bool PhysicsObjectCollection::
is_empty() const {
  return _physics_objects.empty();
}

/**
 * Returns the number of PhysicsObjects in the collection.
 */
int PhysicsObjectCollection::
get_num_physics_objects() const {
  return _physics_objects.size();
}

/**
 * Returns the nth PhysicsObject in the collection.
 */
PT(PhysicsObject) PhysicsObjectCollection::
get_physics_object(int index) const {
  nassertr(index >= 0 && index < (int)_physics_objects.size(), PT(PhysicsObject)());

  return _physics_objects[index];
}

/**
 * Returns the nth PhysicsObject in the collection.  This is the same as
 * get_physics_object(), but it may be a more convenient way to access it.
 */
PT(PhysicsObject) PhysicsObjectCollection::
operator [] (int index) const {
  nassertr(index >= 0 && index < (int)_physics_objects.size(), PT(PhysicsObject)());

  return _physics_objects[index];
}

/**
 * Returns the number of physics objects in the collection.  This is the same
 * thing as get_num_physics_objects().
 */
int PhysicsObjectCollection::
size() const {
  return _physics_objects.size();
}

/**
 * Writes a brief one-line description of the PhysicsObjectCollection to the
 * indicated output stream.
 */
void PhysicsObjectCollection::
output(std::ostream &out) const {
  if (get_num_physics_objects() == 1) {
    out << "1 PhysicsObject";
  } else {
    out << get_num_physics_objects() << " PhysicsObjects";
  }
}

/**
 * Writes a complete multi-line description of the PhysicsObjectCollection to
 * the indicated output stream.
 */
void PhysicsObjectCollection::
write(std::ostream &out, int indent_level) const {
  for (int i = 0; i < get_num_physics_objects(); i++) {
    indent(out, indent_level) << get_physics_object(i) << "\n";
  }
}
