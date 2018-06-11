/**
 * PANDA 3D SOFTWARE
 * Copyright (c) Carnegie Mellon University.  All rights reserved.
 *
 * All use of this software is subject to the terms of the revised BSD
 * license.  You should have received a copy of this license along
 * with this source code in a file named "LICENSE."
 *
 * @file characterMaker.h
 * @author drose
 * @date 2002-03-06
 */

#ifndef CHARACTERMAKER_H
#define CHARACTERMAKER_H

#include "pandabase.h"

#include "vertexTransform.h"
#include "vertexSlider.h"
#include "character.h"
#include "vector_PartGroupStar.h"
#include "typedef.h"
#include "pmap.h"


class EggNode;
class EggGroup;
class EggGroupNode;
class EggPrimitive;
class EggBin;
class PartGroup;
class CharacterJointBundle;
class Character;
class GeomNode;
class CharacterSlider;
class MovingPartBase;
class EggLoader;
class PandaNode;

/**
 * Converts an EggGroup hierarchy, beginning with a group with <Dart> set, to
 * a character node with joints.
 */
class EXPCL_PANDA_EGG2PG CharacterMaker {
public:
  CharacterMaker(EggGroup *root, EggLoader &loader, bool structured = false);

  Character *make_node();

  std::string get_name() const;
  PartGroup *egg_to_part(EggNode *egg_node) const;
  VertexTransform *egg_to_transform(EggNode *egg_node);
  int egg_to_index(EggNode *egg_node) const;
  PandaNode *part_to_node(PartGroup *part, const std::string &name) const;

  int create_slider(const std::string &name);
  VertexSlider *egg_to_slider(const std::string &name);

private:
  CharacterJointBundle *make_bundle();
  void build_joint_hierarchy(EggNode *egg_node, PartGroup *part, int index);
  void parent_joint_nodes(PartGroup *part);

  void make_geometry(EggNode *egg_node);

  EggGroupNode *determine_primitive_home(EggPrimitive *egg_primitive);
  EggGroupNode *determine_bin_home(EggBin *egg_bin);
  VertexTransform *get_identity_transform();

  typedef pmap<EggNode *, int> NodeMap;
  NodeMap _node_map;

  typedef vector_PartGroupStar Parts;
  Parts _parts;

  typedef pmap<int, PT(VertexTransform) > VertexTransforms;
  VertexTransforms _vertex_transforms;
  PT(VertexTransform) _identity_transform;

  typedef pmap<std::string, PT(VertexSlider) > VertexSliders;
  VertexSliders _vertex_sliders;

  EggLoader &_loader;
  EggGroup *_egg_root;
  PT(Character) _character_node;
  CharacterJointBundle *_bundle;
  PartGroup *_morph_root;
  PartGroup *_skeleton_root;

  bool _structured;

};

#endif
