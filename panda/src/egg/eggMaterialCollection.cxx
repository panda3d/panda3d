/**
 * PANDA 3D SOFTWARE
 * Copyright (c) Carnegie Mellon University.  All rights reserved.
 *
 * All use of this software is subject to the terms of the revised BSD
 * license.  You should have received a copy of this license along
 * with this source code in a file named "LICENSE."
 *
 * @file eggMaterialCollection.cxx
 * @author drose
 * @date 2001-04-30
 */

#include "eggMaterialCollection.h"
#include "eggGroupNode.h"
#include "eggPrimitive.h"
#include "eggMaterial.h"

#include "nameUniquifier.h"
#include "dcast.h"

#include <algorithm>

/**
 *
 */
EggMaterialCollection::
EggMaterialCollection() {
}

/**
 *
 */
EggMaterialCollection::
EggMaterialCollection(const EggMaterialCollection &copy) :
  _materials(copy._materials),
  _ordered_materials(copy._ordered_materials)
{
}

/**
 *
 */
EggMaterialCollection &EggMaterialCollection::
operator = (const EggMaterialCollection &copy) {
  _materials = copy._materials;
  _ordered_materials = copy._ordered_materials;
  return *this;
}

/**
 *
 */
EggMaterialCollection::
~EggMaterialCollection() {
}

/**
 * Removes all materials from the collection.
 */
void EggMaterialCollection::
clear() {
  _materials.clear();
  _ordered_materials.clear();
}

/**
 * Walks the egg hierarchy beginning at the indicated node, and removes any
 * EggMaterials encountered in the hierarchy, adding them to the collection.
 * Returns the number of EggMaterials encountered.
 */
int EggMaterialCollection::
extract_materials(EggGroupNode *node) {
  // Since this traversal is destructive, we'll handle it within the
  // EggGroupNode code.
  return node->find_materials(this);
}

/**
 * Adds a series of EggMaterial nodes to the beginning of the indicated node
 * to reflect each of the materials in the collection.  Returns an iterator
 * representing the first position after the newly inserted materials.
 */
EggGroupNode::iterator EggMaterialCollection::
insert_materials(EggGroupNode *node) {
  return insert_materials(node, node->begin());
}

/**
 * Adds a series of EggMaterial nodes to the beginning of the indicated node
 * to reflect each of the materials in the collection.  Returns an iterator
 * representing the first position after the newly inserted materials.
 */
EggGroupNode::iterator EggMaterialCollection::
insert_materials(EggGroupNode *node, EggGroupNode::iterator position) {
  OrderedMaterials::iterator oti;
  for (oti = _ordered_materials.begin();
       oti != _ordered_materials.end();
       ++oti) {
    EggMaterial *material = (*oti);
    position = node->insert(position, material);
  }

  return position;
}

/**
 * Walks the egg hierarchy beginning at the indicated node, looking for
 * materials that are referenced by primitives but are not already members of
 * the collection, adding them to the collection.
 *
 * If this is called following extract_materials(), it can be used to pick up
 * any additional material references that appeared in the egg hierarchy (but
 * whose EggMaterial node was not actually part of the hierarchy).
 *
 * If this is called in lieu of extract_materials(), it will fill up the
 * collection with all of the referenced materials (and only the referenced
 * materials), without destructively removing the EggMaterials from the
 * hierarchy.
 *
 * This also has the side effect of incrementing the internal usage count for
 * a material in the collection each time a material reference is encountered.
 * This side effect is taken advantage of by remove_unused_materials().
 */
int EggMaterialCollection::
find_used_materials(EggNode *node) {
  int num_found = 0;

  if (node->is_of_type(EggPrimitive::get_class_type())) {
    EggPrimitive *primitive = DCAST(EggPrimitive, node);
    if (primitive->has_material()) {
      EggMaterial *tex = primitive->get_material();
      Materials::iterator ti = _materials.find(tex);
      if (ti == _materials.end()) {
        // Here's a new material!
        num_found++;
        _materials.insert(Materials::value_type(tex, 1));
        _ordered_materials.push_back(tex);
      } else {
        // Here's a material we'd already known about.  Increment its usage
        // count.
        (*ti).second++;
      }
    }

  } else if (node->is_of_type(EggGroupNode::get_class_type())) {
    EggGroupNode *group = DCAST(EggGroupNode, node);

    EggGroupNode::iterator ci;
    for (ci = group->begin(); ci != group->end(); ++ci) {
      EggNode *child = *ci;

      num_found += find_used_materials(child);
    }
  }

  return num_found;
}

/**
 * Removes any materials from the collection that aren't referenced by any
 * primitives in the indicated egg hierarchy.  This also, incidentally, adds
 * materials to the collection that had been referenced by primitives but had
 * not previously appeared in the collection.
 */
void EggMaterialCollection::
remove_unused_materials(EggNode *node) {
  // We'll do this the easy way: First, we'll remove *all* the materials from
  // the collection, and then we'll add back only those that appear in the
  // hierarchy.
  clear();
  find_used_materials(node);
}

/**
 * Walks through the collection and collapses together any separate materials
 * that are equivalent according to the indicated equivalence factor, eq (see
 * EggMaterial::is_equivalent_to()).  The return value is the number of
 * materials removed.
 *
 * This flavor of collapse_equivalent_materials() automatically adjusts all
 * the primitives in the egg hierarchy to refer to the new material pointers.
 */
int EggMaterialCollection::
collapse_equivalent_materials(int eq, EggGroupNode *node) {
  MaterialReplacement removed;
  int num_collapsed = collapse_equivalent_materials(eq, removed);

  // And now walk the egg hierarchy and replace any references to a removed
  // material with its replacement.
  replace_materials(node, removed);

  return num_collapsed;
}

/**
 * Walks through the collection and collapses together any separate materials
 * that are equivalent according to the indicated equivalence factor, eq (see
 * EggMaterial::is_equivalent_to()).  The return value is the number of
 * materials removed.
 *
 * This flavor of collapse_equivalent_materials() does not adjust any
 * primitives in the egg hierarchy; instead, it fills up the 'removed' map
 * with an entry for each removed material, mapping it back to the equivalent
 * retained material.  It's up to the user to then call replace_materials()
 * with this map, if desired, to apply these changes to the egg hierarchy.
 */
int EggMaterialCollection::
collapse_equivalent_materials(int eq, EggMaterialCollection::MaterialReplacement &removed) {
  int num_collapsed = 0;

  typedef pset<PT(EggMaterial), UniqueEggMaterials> Collapser;
  UniqueEggMaterials uet(eq);
  Collapser collapser(uet);

  // First, put all of the materials into the Collapser structure, to find out
  // the unique materials.
  OrderedMaterials::const_iterator oti;
  for (oti = _ordered_materials.begin();
       oti != _ordered_materials.end();
       ++oti) {
    EggMaterial *tex = (*oti);

    std::pair<Collapser::const_iterator, bool> result = collapser.insert(tex);
    if (!result.second) {
      // This material is non-unique; another one was already there.
      EggMaterial *first = *(result.first);
      removed.insert(MaterialReplacement::value_type(tex, first));
      num_collapsed++;
    }
  }

  // Now record all of the unique materials only.
  clear();
  Collapser::const_iterator ci;
  for (ci = collapser.begin(); ci != collapser.end(); ++ci) {
    add_material(*ci);
  }

  return num_collapsed;
}

/**
 * Walks the egg hierarchy, changing out any reference to a material appearing
 * on the left side of the map with its corresponding material on the right
 * side.  This is most often done following a call to
 * collapse_equivalent_materials().  It does not directly affect the
 * Collection.
 */
void EggMaterialCollection::
replace_materials(EggGroupNode *node,
                 const EggMaterialCollection::MaterialReplacement &replace) {
  EggGroupNode::iterator ci;
  for (ci = node->begin();
       ci != node->end();
       ++ci) {
    EggNode *child = *ci;
    if (child->is_of_type(EggPrimitive::get_class_type())) {
      EggPrimitive *primitive = DCAST(EggPrimitive, child);
      if (primitive->has_material()) {
        PT(EggMaterial) tex = primitive->get_material();
        MaterialReplacement::const_iterator ri;
        ri = replace.find(tex);
        if (ri != replace.end()) {
          // Here's a material we want to replace.
          primitive->set_material((*ri).second);
        }
      }

    } else if (child->is_of_type(EggGroupNode::get_class_type())) {
      EggGroupNode *group_child = DCAST(EggGroupNode, child);
      replace_materials(group_child, replace);
    }
  }
}

/**
 * Guarantees that each material in the collection has a unique MRef name.
 * This is essential before writing an egg file.
 */
void EggMaterialCollection::
uniquify_mrefs() {
  NameUniquifier nu(".mref", "mref");

  OrderedMaterials::const_iterator oti;
  for (oti = _ordered_materials.begin();
       oti != _ordered_materials.end();
       ++oti) {
    EggMaterial *tex = (*oti);

    tex->set_name(nu.add_name(tex->get_name()));
  }
}

/**
 * Sorts all the materials into alphabetical order by MRef name.  Subsequent
 * operations using begin()/end() will traverse in this sorted order.
 */
void EggMaterialCollection::
sort_by_mref() {
  sort(_ordered_materials.begin(), _ordered_materials.end(),
       NamableOrderByName());
}

/**
 * Explicitly adds a new material to the collection.  Returns true if the
 * material was added, false if it was already there or if there was some
 * error.
 */
bool EggMaterialCollection::
add_material(EggMaterial *material) {
  nassertr(_materials.size() == _ordered_materials.size(), false);

  PT(EggMaterial) new_tex = material;

  Materials::const_iterator ti;
  ti = _materials.find(new_tex);
  if (ti != _materials.end()) {
    // This material is already a member of the collection.
    return false;
  }

  _materials.insert(Materials::value_type(new_tex, 0));
  _ordered_materials.push_back(new_tex);

  nassertr(_materials.size() == _ordered_materials.size(), false);
  return true;
}

/**
 * Explicitly removes a material from the collection.  Returns true if the
 * material was removed, false if it wasn't there or if there was some error.
 */
bool EggMaterialCollection::
remove_material(EggMaterial *material) {
  nassertr(_materials.size() == _ordered_materials.size(), false);

  Materials::iterator ti;
  ti = _materials.find(material);
  if (ti == _materials.end()) {
    // This material is not a member of the collection.
    return false;
  }

  _materials.erase(ti);

  OrderedMaterials::iterator oti;
  PT(EggMaterial) ptex = material;
  oti = find(_ordered_materials.begin(), _ordered_materials.end(), ptex);
  nassertr(oti != _ordered_materials.end(), false);

  _ordered_materials.erase(oti);

  nassertr(_materials.size() == _ordered_materials.size(), false);
  return true;
}

/**
 * Creates a new material if there is not already one equivalent (according to
 * eq, see EggMaterial::is_equivalent_to()) to the indicated material, or
 * returns the existing one if there is.
 */
EggMaterial *EggMaterialCollection::
create_unique_material(const EggMaterial &copy, int eq) {
  // This requires a complete linear traversal, not terribly efficient.
  OrderedMaterials::const_iterator oti;
  for (oti = _ordered_materials.begin();
       oti != _ordered_materials.end();
       ++oti) {
    EggMaterial *tex = (*oti);
    if (copy.is_equivalent_to(*tex, eq)) {
      return tex;
    }
  }

  EggMaterial *new_material = new EggMaterial(copy);
  add_material(new_material);
  return new_material;
}

/**
 * Returns the material with the indicated MRef name, or NULL if no material
 * matches.
 */
EggMaterial *EggMaterialCollection::
find_mref(const std::string &mref_name) const {
  // This requires a complete linear traversal, not terribly efficient.
  OrderedMaterials::const_iterator oti;
  for (oti = _ordered_materials.begin();
       oti != _ordered_materials.end();
       ++oti) {
    EggMaterial *tex = (*oti);
    if (tex->get_name() == mref_name) {
      return tex;
    }
  }

  return nullptr;
}
