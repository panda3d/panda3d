// Filename: softNodeDesc.h
// Created by:  masad (03Oct03)
//
////////////////////////////////////////////////////////////////////
//
// PANDA 3D SOFTWARE
// Copyright (c) 2001, Disney Enterprises, Inc.  All rights reserved
//
// All use of this software is subject to the terms of the Panda 3d
// Software license.  You should have received a copy of this license
// along with this source code; you will also find a current copy of
// the license at http://www.panda3d.org/license.txt .
//
// To contact the maintainers of this program write to
// panda3d@yahoogroups.com .
//
////////////////////////////////////////////////////////////////////

#ifndef SOFTNODEDESC_H
#define SOFTNODEDESC_H

#include "pandatoolbase.h"

#include "referenceCount.h"
#include "pointerTo.h"
#include "namable.h"

#include <SAA.h>

class EggGroup;
class EggTable;
class EggXfmSAnim;

////////////////////////////////////////////////////////////////////
//       Class : SoftNodeDesc
// Description : Describes a single instance of a node aka element in the Soft
//               scene graph, relating it to the corresponding egg
//               structures (e.g. node, group, or table entry) that
//               will be created.
////////////////////////////////////////////////////////////////////
class SoftNodeDesc : public ReferenceCount, public Namable {
public:
  SoftNodeDesc(const string &name = string());
  ~SoftNodeDesc();

  void set_model(SAA_Elem *model);
  bool has_model() const;
  SAA_Elem *get_model() const;

  bool is_joint() const;
  //  bool is_joint_parent() const;

  //  SoftNodeDesc *_parent;
  //  typedef pvector< PT(SoftNodeDesc) > Children;
  //  Children _children;
  
private:
  void clear_egg();
  //  void mark_joint_parent();
  //  void check_pseudo_joints(bool joint_above);

  SAA_Elem *_model;

  EggGroup *_egg_group;
  EggTable *_egg_table;
  EggXfmSAnim *_anim;

  enum JointType {
    JT_none,         // Not a joint.
    JT_joint,        // An actual joint in Soft.
    JT_pseudo_joint, // Not a joint in Soft, but treated just like a
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
    register_type(_type_handle, "SoftNodeDesc",
                  ReferenceCount::get_class_type(),
                  Namable::get_class_type());
  }

private:
  static TypeHandle _type_handle;

  friend class SoftNodeTree;
};

#endif
