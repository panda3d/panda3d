/**
 * PANDA 3D SOFTWARE
 * Copyright (c) Carnegie Mellon University.  All rights reserved.
 *
 * All use of this software is subject to the terms of the revised BSD
 * license.  You should have received a copy of this license along
 * with this source code in a file named "LICENSE."
 *
 * @file eggMaterialCollection.h
 * @author drose
 * @date 2001-04-30
 */

#ifndef EGGMATERIALCOLLECTION_H
#define EGGMATERIALCOLLECTION_H

#include "pandabase.h"

#include "eggMaterial.h"
#include "eggGroupNode.h"
#include "vector_PT_EggMaterial.h"

#include "pmap.h"

/**
 * This is a collection of materials by MRef name.  It can extract the
 * materials from an egg file and sort them all together; it can also manage
 * the creation of unique materials and the assignment of unique MRef names.
 */
class EXPCL_PANDA_EGG EggMaterialCollection {

  // This is a bit of private interface stuff that must be here as a forward
  // reference.  This allows us to define the EggMaterialCollection as an STL
  // container.

private:
  typedef pmap<PT_EggMaterial, int> Materials;
  typedef vector_PT_EggMaterial OrderedMaterials;

public:
  typedef OrderedMaterials::const_iterator iterator;
  typedef iterator const_iterator;
  typedef OrderedMaterials::size_type size_type;

  typedef pmap<PT_EggMaterial,  PT_EggMaterial > MaterialReplacement;

  // Here begins the actual public interface to EggMaterialCollection.

PUBLISHED:
  EggMaterialCollection();
  EggMaterialCollection(const EggMaterialCollection &copy);
  EggMaterialCollection &operator = (const EggMaterialCollection &copy);
  ~EggMaterialCollection();

  void clear();

  int extract_materials(EggGroupNode *node);

public:
  EggGroupNode::iterator insert_materials(EggGroupNode *node);
  EggGroupNode::iterator insert_materials(EggGroupNode *node, EggGroupNode::iterator position);

PUBLISHED:
  int find_used_materials(EggNode *node);
  void remove_unused_materials(EggNode *node);

  int collapse_equivalent_materials(int eq, EggGroupNode *node);
  int collapse_equivalent_materials(int eq, MaterialReplacement &removed);
  static void replace_materials(EggGroupNode *node,
                               const MaterialReplacement &replace);

  void uniquify_mrefs();
  void sort_by_mref();

  // Can be used to traverse all the materials in the collection, in order as
  // last sorted.
public:
  INLINE iterator begin() const;
  INLINE iterator end() const;
  INLINE bool empty() const;
  INLINE size_type size() const;

PUBLISHED:
  bool add_material(EggMaterial *material);
  bool remove_material(EggMaterial *material);

  // create_unique_material() creates a new material if there is not already
  // one equivalent (according to eq, see EggMaterial::is_equivalent_to()) to
  // the indicated material, or returns the existing one if there is.
  EggMaterial *create_unique_material(const EggMaterial &copy, int eq);

  // Find a material with a particular MRef name.
  EggMaterial *find_mref(const std::string &mref_name) const;

private:
  Materials _materials;
  OrderedMaterials _ordered_materials;
};

#include "eggMaterialCollection.I"

#endif
