// Filename: characterMaker.h
// Created by:  drose (06Mar02)
//
////////////////////////////////////////////////////////////////////
//
// PANDA 3D SOFTWARE
// Copyright (c) 2001 - 2004, Disney Enterprises, Inc.  All rights reserved
//
// All use of this software is subject to the terms of the Panda 3d
// Software license.  You should have received a copy of this license
// along with this source code; you will also find a current copy of
// the license at http://etc.cmu.edu/panda3d/docs/license/ .
//
// To contact the maintainers of this program write to
// panda3d-general@lists.sourceforge.net .
//
////////////////////////////////////////////////////////////////////

#ifndef CHARACTERMAKER_H
#define CHARACTERMAKER_H

#include "pandabase.h"

#include "computedVerticesMaker.h"

#include "vector_PartGroupStar.h"
#include "typedef.h"
#include "pmap.h"

class EggNode;
class EggGroup;
class EggGroupNode;
class EggPrimitive;
class PartGroup;
class CharacterJointBundle;
class Character;
class CharacterSlider;
class MovingPartBase;
class EggLoader;
class PandaNode;

///////////////////////////////////////////////////////////////////
//       Class : CharacterMaker
// Description : Converts an EggGroup hierarchy, beginning with a
//               group with <Dart> set, to a character node with
//               joints.
////////////////////////////////////////////////////////////////////
class EXPCL_PANDAEGG CharacterMaker {
public:
  CharacterMaker(EggGroup *root, EggLoader &loader);

  Character *make_node();

  PartGroup *egg_to_part(EggNode *egg_node) const;
  int egg_to_index(EggNode *egg_node) const;
  PandaNode *part_to_node(PartGroup *part) const;

  int create_slider(const string &name);

private:
  CharacterJointBundle *make_bundle();
  void build_joint_hierarchy(EggNode *egg_node, PartGroup *part);
  void parent_joint_nodes(PartGroup *part);

  void make_geometry(EggNode *egg_node);

  void make_static_primitive(EggPrimitive *egg_primitive,
                             EggGroupNode *prim_home);
  void make_dynamic_primitive(EggPrimitive *egg_primitive,
                              EggGroupNode *prim_home);
  EggGroupNode *determine_primitive_home(EggPrimitive *egg_primitive);

  typedef pmap<EggNode *, int> NodeMap;
  NodeMap _node_map;

  typedef vector_PartGroupStar Parts;
  Parts _parts;

  EggLoader &_loader;
  EggGroup *_egg_root;
  Character *_character_node;
  CharacterJointBundle *_bundle;
  ComputedVerticesMaker _comp_verts_maker;
  PartGroup *_morph_root;
  PartGroup *_skeleton_root;
};

#endif
