/**
 * PANDA 3D SOFTWARE
 * Copyright (c) Carnegie Mellon University.  All rights reserved.
 *
 * All use of this software is subject to the terms of the revised BSD
 * license.  You should have received a copy of this license along
 * with this source code in a file named "LICENSE."
 *
 * @file characterJoint.h
 * @author drose
 * @date 1999-02-23
 */

#ifndef CHARACTERJOINT_H
#define CHARACTERJOINT_H

#include "pandabase.h"
#include "transformState.h"

#include "movingPartMatrix.h"
#include "pandaNode.h"
#include "nodePathCollection.h"
#include "ordered_vector.h"

class JointVertexTransform;
class Character;

/**
 * This represents one joint of the character's animation, containing an
 * animating transform matrix.
 */
class EXPCL_PANDA_CHAR CharacterJoint : public MovingPartMatrix {
protected:
  CharacterJoint();
  CharacterJoint(const CharacterJoint &copy);

PUBLISHED:
  explicit CharacterJoint(Character *character, PartBundle *root,
                          PartGroup *parent, const std::string &name,
                          const LMatrix4 &default_value);
  virtual ~CharacterJoint();

public:
  virtual bool is_character_joint() const;
  virtual PartGroup *make_copy() const;

  virtual bool update_internals(PartBundle *root, PartGroup *parent,
                                bool self_changed, bool parent_changed,
                                Thread *current_thread);
  virtual void do_xform(const LMatrix4 &mat, const LMatrix4 &inv_mat);

PUBLISHED:
  bool add_net_transform(PandaNode *node);
  bool remove_net_transform(PandaNode *node);
  bool has_net_transform(PandaNode *node) const;
  void clear_net_transforms();
  NodePathCollection get_net_transforms();

  bool add_local_transform(PandaNode *node);
  bool remove_local_transform(PandaNode *node);
  bool has_local_transform(PandaNode *node) const;
  void clear_local_transforms();
  NodePathCollection get_local_transforms();

  void get_transform(LMatrix4 &transform) const;
  INLINE const LMatrix4 &get_transform() const;
  CPT(TransformState) get_transform_state() const;

  void get_net_transform(LMatrix4 &transform) const;

  Character *get_character() const;

private:
  void set_character(Character *character);

private:
  // Not a reference-counted pointer.
  Character *_character;

  typedef ov_set< PT(PandaNode) > NodeList;
  NodeList _net_transform_nodes;
  NodeList _local_transform_nodes;

  typedef ov_set<JointVertexTransform *> VertexTransforms;
  VertexTransforms _vertex_transforms;

public:
  static void register_with_read_factory();
  virtual void write_datagram(BamWriter* manager, Datagram &me);
  virtual int complete_pointers(TypedWritable **p_list,
                                BamReader *manager);

  static TypedWritable *make_CharacterJoint(const FactoryParams &params);

protected:
  void fillin(DatagramIterator& scan, BamReader* manager);

private:
  int _num_net_nodes, _num_local_nodes;

public:
  // The _geom_node member just holds a temporary pointer to a node for the
  // CharacterMaker's convenenience while creating the character.  It does not
  // store any meaningful value after creation is complete.
  PT(PandaNode) _geom_node;

  // These are filled in as the joint animates.
  LMatrix4 _net_transform;
  LMatrix4 _initial_net_transform_inverse;

  // This is the product of the above; the matrix that gets applied to a
  // vertex (whose coordinates are in the coordinate space of the character
  // in its neutral pose) to transform it from its neutral position to its
  // animated position.
  LMatrix4 _skinning_matrix;

public:
  virtual TypeHandle get_type() const {
    return get_class_type();
  }
  virtual TypeHandle force_init_type() {init_type(); return get_class_type();}
  static TypeHandle get_class_type() {
    return _type_handle;
  }
  static void init_type() {
    MovingPartMatrix::init_type();
    register_type(_type_handle, "CharacterJoint",
                  MovingPartMatrix::get_class_type());
  }

private:
  static TypeHandle _type_handle;

  friend class Character;
  friend class CharacterJointBundle;
  friend class JointVertexTransform;
};

#include "characterJoint.I"

#endif
