/**
 * PANDA 3D SOFTWARE
 * Copyright (c) Carnegie Mellon University.  All rights reserved.
 *
 * All use of this software is subject to the terms of the revised BSD
 * license.  You should have received a copy of this license along
 * with this source code in a file named "LICENSE."
 *
 * @file mayaNodeDesc.h
 * @author drose
 * @date 2003-06-06
 */

#ifndef MAYANODEDESC_H
#define MAYANODEDESC_H

#include "pandatoolbase.h"

#include "mayaBlendDesc.h"
#include "referenceCount.h"
#include "pointerTo.h"
#include "namable.h"

#include "pre_maya_include.h"
#include <maya/MDagPath.h>
#include <maya/MFnDagNode.h>
#include "post_maya_include.h"

class MayaToEggConverter;
class MayaNodeTree;
class EggGroup;
class EggTable;
class EggXfmSAnim;

/**
 * Describes a single instance of a node in the Maya scene graph, relating it
 * to the corresponding egg structures (e.g.  node, group, or table entry)
 * that will be created.
 */
class MayaNodeDesc : public ReferenceCount, public Namable {
public:
  MayaNodeDesc(MayaNodeTree *tree,
               MayaNodeDesc *parent = nullptr, const std::string &name = std::string());
  ~MayaNodeDesc();

  void from_dag_path(const MDagPath &dag_path, MayaToEggConverter *converter);
  bool has_dag_path() const;
  const MDagPath &get_dag_path() const;

  int get_num_blend_descs() const;
  MayaBlendDesc *get_blend_desc(int n) const;

  bool is_joint() const;
  bool is_joint_parent() const;

  bool is_tagged() const;
  bool is_joint_tagged() const;
  bool has_object_type(std::string object_type) const;

  MayaNodeTree *_tree;
  MayaNodeDesc *_parent;
  typedef pvector< PT(MayaNodeDesc) > Children;
  Children _children;

private:
  void tag();
  void untag();
  void tag_recursively();
  void untag_recursively();
  void tag_joint();
  void tag_joint_recursively();

  void clear_egg();
  void mark_joint_parent();
  void check_pseudo_joints(bool joint_above);
  void check_blend_shapes(const MFnDagNode &node,
                          const std::string &attrib_name);
  void check_lods();

  MDagPath *_dag_path;

  EggGroup *_egg_group;
  EggTable *_egg_table;
  EggXfmSAnim *_anim;

  typedef pvector< PT(MayaBlendDesc) > BlendDescs;
  BlendDescs _blend_descs;

  enum JointType {
    JT_none,         // Not a joint.
    JT_joint,        // An actual joint in Maya.
    JT_pseudo_joint, // Not a joint in Maya, but treated just like a
                     // joint for the purposes of the converter.
    JT_joint_parent, // A parent or ancestor of a joint or pseudo joint.
  };
  JointType _joint_type;

  bool _is_lod;
  double _switch_in, _switch_out;

  bool _tagged;
  bool _joint_tagged;

public:
  static TypeHandle get_class_type() {
    return _type_handle;
  }
  static void init_type() {
    ReferenceCount::init_type();
    Namable::init_type();
    register_type(_type_handle, "MayaNodeDesc",
                  ReferenceCount::get_class_type(),
                  Namable::get_class_type());
  }

private:
  static TypeHandle _type_handle;

  friend class MayaNodeTree;
};

#endif
