/**
 * PANDA 3D SOFTWARE
 * Copyright (c) Carnegie Mellon University.  All rights reserved.
 *
 * All use of this software is subject to the terms of the revised BSD
 * license.  You should have received a copy of this license along
 * with this source code in a file named "LICENSE."
 *
 * @file materialCollection.cxx
 * @author drose
 * @date 2002-03-16
 */

#include "materialCollection.h"

#include "indent.h"

/**
 *
 */
MaterialCollection::
MaterialCollection() {
}

/**
 *
 */
MaterialCollection::
MaterialCollection(const MaterialCollection &copy) :
  _materials(copy._materials)
{
}

/**
 *
 */
void MaterialCollection::
operator = (const MaterialCollection &copy) {
  _materials = copy._materials;
}

/**
 * Adds a new Material to the collection.
 */
void MaterialCollection::
add_material(Material *node_material) {
  // If the pointer to our internal array is shared by any other
  // MaterialCollections, we have to copy the array now so we won't
  // inadvertently modify any of our brethren MaterialCollection objects.

  if (_materials.get_ref_count() > 1) {
    Materials old_materials = _materials;
    _materials = Materials::empty_array(0);
    _materials.v() = old_materials.v();
  }

  _materials.push_back(node_material);
}

/**
 * Removes the indicated Material from the collection.  Returns true if the
 * material was removed, false if it was not a member of the collection.
 */
bool MaterialCollection::
remove_material(Material *node_material) {
  int material_index = -1;
  for (int i = 0; material_index == -1 && i < (int)_materials.size(); i++) {
    if (_materials[i] == node_material) {
      material_index = i;
    }
  }

  if (material_index == -1) {
    // The indicated material was not a member of the collection.
    return false;
  }

  // If the pointer to our internal array is shared by any other
  // MaterialCollections, we have to copy the array now so we won't
  // inadvertently modify any of our brethren MaterialCollection objects.

  if (_materials.get_ref_count() > 1) {
    Materials old_materials = _materials;
    _materials = Materials::empty_array(0);
    _materials.v() = old_materials.v();
  }

  _materials.erase(_materials.begin() + material_index);
  return true;
}

/**
 * Adds all the Materials indicated in the other collection to this material.
 * The other materials are simply appended to the end of the materials in this
 * list; duplicates are not automatically removed.
 */
void MaterialCollection::
add_materials_from(const MaterialCollection &other) {
  int other_num_materials = other.get_num_materials();
  for (int i = 0; i < other_num_materials; i++) {
    add_material(other.get_material(i));
  }
}


/**
 * Removes from this collection all of the Materials listed in the other
 * collection.
 */
void MaterialCollection::
remove_materials_from(const MaterialCollection &other) {
  Materials new_materials;
  int num_materials = get_num_materials();
  for (int i = 0; i < num_materials; i++) {
    PT(Material) material = get_material(i);
    if (!other.has_material(material)) {
      new_materials.push_back(material);
    }
  }
  _materials = new_materials;
}

/**
 * Removes any duplicate entries of the same Materials on this collection.  If
 * a Material appears multiple times, the first appearance is retained;
 * subsequent appearances are removed.
 */
void MaterialCollection::
remove_duplicate_materials() {
  Materials new_materials;

  int num_materials = get_num_materials();
  for (int i = 0; i < num_materials; i++) {
    PT(Material) material = get_material(i);
    bool duplicated = false;

    for (int j = 0; j < i && !duplicated; j++) {
      duplicated = (material == get_material(j));
    }

    if (!duplicated) {
      new_materials.push_back(material);
    }
  }

  _materials = new_materials;
}

/**
 * Returns true if the indicated Material appears in this collection, false
 * otherwise.
 */
bool MaterialCollection::
has_material(Material *material) const {
  for (int i = 0; i < get_num_materials(); i++) {
    if (material == get_material(i)) {
      return true;
    }
  }
  return false;
}

/**
 * Removes all Materials from the collection.
 */
void MaterialCollection::
clear() {
  _materials.clear();
}

/**
 * Returns the material in the collection with the indicated name, if any, or
 * NULL if no material has that name.
 */
Material *MaterialCollection::
find_material(const std::string &name) const {
  int num_materials = get_num_materials();
  for (int i = 0; i < num_materials; i++) {
    Material *material = get_material(i);
    if (material->get_name() == name) {
      return material;
    }
  }
  return nullptr;
}

/**
 * Returns the number of Materials in the collection.
 */
int MaterialCollection::
get_num_materials() const {
  return _materials.size();
}

/**
 * Returns the nth Material in the collection.
 */
Material *MaterialCollection::
get_material(int index) const {
  nassertr(index >= 0 && index < (int)_materials.size(), nullptr);

  return _materials[index];
}

/**
 * Returns the nth Material in the collection.  This is the same as
 * get_material(), but it may be a more convenient way to access it.
 */
Material *MaterialCollection::
operator [] (int index) const {
  nassertr(index >= 0 && index < (int)_materials.size(), nullptr);

  return _materials[index];
}

/**
 * Returns the number of materials in the collection.  This is the same thing
 * as get_num_materials().
 */
int MaterialCollection::
size() const {
  return _materials.size();
}

/**
 * Writes a brief one-line description of the MaterialCollection to the
 * indicated output stream.
 */
void MaterialCollection::
output(std::ostream &out) const {
  if (get_num_materials() == 1) {
    out << "1 Material";
  } else {
    out << get_num_materials() << " Materials";
  }
}

/**
 * Writes a complete multi-line description of the MaterialCollection to the
 * indicated output stream.
 */
void MaterialCollection::
write(std::ostream &out, int indent_level) const {
  for (int i = 0; i < get_num_materials(); i++) {
    indent(out, indent_level) << *get_material(i) << "\n";
  }
}
