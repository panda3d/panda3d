// Filename: characterJoint.h
// Created by:  drose (23Feb99)
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

#ifndef CHARACTERJOINT_H
#define CHARACTERJOINT_H

#include "pandabase.h"

#include "movingPartMatrix.h"
#include "pandaNode.h"

////////////////////////////////////////////////////////////////////
//       Class : CharacterJoint
// Description : This represents one joint of the character's
//               animation, containing an animating transform matrix.
////////////////////////////////////////////////////////////////////
class EXPCL_PANDA CharacterJoint : public MovingPartMatrix {
protected:
  CharacterJoint();
  CharacterJoint(const CharacterJoint &copy);

public:
  CharacterJoint(PartGroup *parent, const string &name,
                 const LMatrix4f &initial_value);

  virtual PartGroup *make_copy() const;

  virtual bool update_internals(PartGroup *parent, bool self_changed,
                                bool parent_changed);

PUBLISHED:
  bool add_net_transform(PandaNode *node);
  bool remove_net_transform(PandaNode *node);
  bool has_net_transform(PandaNode *node) const;
  void clear_net_transforms();

  bool add_local_transform(PandaNode *node);
  bool remove_local_transform(PandaNode *node);
  bool has_local_transform(PandaNode *node) const;
  void clear_local_transforms();

  void get_transform(LMatrix4f &transform) const;

private:
  typedef pset< PT(PandaNode) > NodeList;
  NodeList _net_transform_nodes;
  NodeList _local_transform_nodes;

public:
  static void register_with_read_factory(void);
  virtual void write_datagram(BamWriter* manager, Datagram &me);
  virtual int complete_pointers(TypedWritable **p_list,
                                BamReader *manager);

  static TypedWritable *make_CharacterJoint(const FactoryParams &params);

protected:
  void fillin(DatagramIterator& scan, BamReader* manager);

private:
  int _num_net_nodes, _num_local_nodes;

public:
  // The _geom_node member just holds a temporary pointer to a node
  // for the CharacterMaker's convenenience while creating the
  // character.  It does not store any meaningful value after
  // creation is complete.
  PT(PandaNode) _geom_node;

  // These are filled in as the joint animates.
  LMatrix4f _net_transform;
  LMatrix4f _initial_net_transform_inverse;

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
};

#endif


