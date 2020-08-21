/**
 * PANDA 3D SOFTWARE
 * Copyright (c) Carnegie Mellon University.  All rights reserved.
 *
 * All use of this software is subject to the terms of the revised BSD
 * license.  You should have received a copy of this license along
 * with this source code in a file named "LICENSE."
 *
 * @file partGroup.h
 * @author drose
 * @date 1999-02-22
 */

#ifndef PARTGROUP_H
#define PARTGROUP_H

#include "pandabase.h"

#include "typedWritableReferenceCount.h"
#include "pointerTo.h"
#include "namable.h"
#include "typedef.h"
#include "thread.h"
#include "plist.h"
#include "luse.h"

class AnimControl;
class AnimGroup;
class PartBundle;
class PartSubset;
class BamReader;
class FactoryParams;
class BitArray;
class CycleData;
class TransformState;
class PandaNode;
class AnimChannelBase;

/**
 * This is the base class for PartRoot and MovingPart.  It defines a hierarchy
 * of MovingParts.
 */
class EXPCL_PANDA_CHAN PartGroup : public TypedWritableReferenceCount, public Namable {
PUBLISHED:
  // This enum defines bits which may be passed into check_hierarchy() and
  // PartBundle::bind_anim() to allow an inexact match of channel hierarchies.
  // This specifies conditions that we don't care about enforcing.
  enum HierarchyMatchFlags {
    HMF_ok_part_extra          = 0x01,
    HMF_ok_anim_extra          = 0x02,
    HMF_ok_wrong_root_name     = 0x04,
  };

protected:
  // The default constructor is protected: don't try to create a PartGroup
  // without a parent.  To create a PartGroup hierarchy, you must first create
  // a PartBundle, and use that as the parent of any subsequent children.
  INLINE PartGroup(const std::string &name = "");
  INLINE PartGroup(const PartGroup &copy);

PUBLISHED:
  // This is the normal PartGroup constructor.
  explicit PartGroup(PartGroup *parent, const std::string &name);
  virtual ~PartGroup();
  virtual bool is_character_joint() const;

  virtual PartGroup *make_copy() const;
  PartGroup *copy_subgraph() const;

  int get_num_children() const;
  PartGroup *get_child(int n) const;
  MAKE_SEQ(get_children, get_num_children, get_child);

  PartGroup *get_child_named(const std::string &name) const;
  PartGroup *find_child(const std::string &name) const;
  void sort_descendants();

  MAKE_SEQ_PROPERTY(children, get_num_children, get_child);

  bool apply_freeze(const TransformState *transform);
  virtual bool apply_freeze_matrix(const LVecBase3 &pos, const LVecBase3 &hpr, const LVecBase3 &scale);
  virtual bool apply_freeze_scalar(PN_stdfloat value);
  virtual bool apply_control(PandaNode *node);
  virtual bool clear_forced_channel();
  virtual AnimChannelBase *get_forced_channel() const;

  virtual void write(std::ostream &out, int indent_level) const;
  virtual void write_with_value(std::ostream &out, int indent_level) const;

public:
  virtual TypeHandle get_value_type() const;

  bool check_hierarchy(const AnimGroup *anim,
                       const PartGroup *parent,
                       int hierarchy_match_flags = 0) const;

  virtual bool do_update(PartBundle *root, const CycleData *root_cdata,
                         PartGroup *parent, bool parent_changed,
                         bool anim_changed, Thread *current_thread);
  virtual void do_xform(const LMatrix4 &mat, const LMatrix4 &inv_mat);
  virtual void determine_effective_channels(const CycleData *root_cdata);

protected:
  void write_descendants(std::ostream &out, int indent_level) const;
  void write_descendants_with_value(std::ostream &out, int indent_level) const;

  virtual void pick_channel_index(plist<int> &holes, int &next) const;
  virtual void bind_hierarchy(AnimGroup *anim, int channel_index,
                              int &joint_index, bool is_included,
                              BitArray &bound_joints,
                              const PartSubset &subset);
  virtual void find_bound_joints(int &joint_index, bool is_included,
                                 BitArray &bound_joints,
                                 const PartSubset &subset);

  typedef pvector< PT(PartGroup) > Children;
  Children _children;

public:
  static void register_with_read_factory();
  virtual void write_datagram(BamWriter* manager, Datagram &me);
  virtual int complete_pointers(TypedWritable **p_list,
                                BamReader *manager);

  static TypedWritable *make_PartGroup(const FactoryParams &params);

protected:
  void fillin(DatagramIterator& scan, BamReader* manager);

public:
  virtual TypeHandle get_type() const {
    return get_class_type();
  }
  virtual TypeHandle force_init_type() {init_type(); return get_class_type();}
  static TypeHandle get_class_type() {
    return _type_handle;
  }

public:
  static void init_type() {
    TypedWritableReferenceCount::init_type();
    register_type(_type_handle, "PartGroup",
                  TypedWritableReferenceCount::get_class_type());
  }

private:
  static TypeHandle _type_handle;

  friend class Character;
  friend class CharacterJointBundle;
  friend class PartBundle;
};

#include "partGroup.I"

#endif
