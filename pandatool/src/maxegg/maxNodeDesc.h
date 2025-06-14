/**
 * PANDA 3D SOFTWARE
 * Copyright (c) Carnegie Mellon University.  All rights reserved.
 *
 * All use of this software is subject to the terms of the revised BSD
 * license.  You should have received a copy of this license along
 * with this source code in a file named "LICENSE."
 *
 * @file maxNodeDesc.h
 * @author crevilla
 * from mayaNodeDesc.h created by:  drose (06Jun03)
 */

#ifndef MAXNODEDESC_H
#define MAXNODEDESC_H

/**
 * Describes a single instance of a node in the Max scene graph, relating it
 * to the corresponding egg structures (e.g.  node, group, or table entry)
 * that will be created.
 */
class MaxNodeDesc : public ReferenceCount, public Namable {
 public:
  MaxNodeDesc(MaxNodeDesc *parent = nullptr, INode *max_node = nullptr);
  ~MaxNodeDesc();

  void from_INode(INode *max_node);
  bool has_max_node() const;
  INode *get_max_node() const;

  void set_joint(bool onoff);
  bool is_joint() const;
  bool is_joint_parent() const;
  bool is_node_joint() const;

  MaxNodeDesc *_parent;
  MaxNodeDesc *_joint_entry;
  typedef pvector< MaxNodeDesc* > Children;
  Children _children;

 private:
  void clear_egg();
  void mark_joint_parent();
  void check_pseudo_joints(bool joint_above);

  INode *_max_node;

  EggGroup *_egg_group;
  EggTable *_egg_table;
  EggXfmSAnim *_anim;

  enum JointType {
    JT_none,         // Not a joint.
    JT_node_joint,    // Node that represents a joint in the geometry
                                         // but not the actual joint itself
    JT_joint,        // An actual joint in Max.
    JT_pseudo_joint, // Not a joint in Max, but treated just like a
                     // joint for the purposes of the converter.
    JT_joint_parent, // A parent or ancestor of a joint or pseudo joint.
  };
  JointType _joint_type;


 public:
  static TypeHandle get_class_type() {
    return _type_handle;
  }
  static void init_type() {
    ReferenceCount::init_type();
    Namable::init_type();
    register_type(_type_handle, "MaxNodeDesc",
                  ReferenceCount::get_class_type(),
                  Namable::get_class_type());
  }

 private:
  static TypeHandle _type_handle;

  friend class MaxNodeTree;
};

#endif
