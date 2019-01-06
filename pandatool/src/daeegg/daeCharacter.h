/**
 * PANDA 3D SOFTWARE
 * Copyright (c) Carnegie Mellon University.  All rights reserved.
 *
 * All use of this software is subject to the terms of the revised BSD
 * license.  You should have received a copy of this license along
 * with this source code in a file named "LICENSE."
 *
 * @file daeCharacter.h
 * @author rdb
 * @date 2008-11-24
 */

#ifndef DAECHARACTER_H
#define DAECHARACTER_H

#include "pandatoolbase.h"
#include "typedReferenceCount.h"
#include "typeHandle.h"
#include "eggTable.h"
#include "epvector.h"

#include "pre_fcollada_include.h"
#include <FCollada.h>
#include <FCDocument/FCDSceneNode.h>
#include <FCDocument/FCDControllerInstance.h>
#include <FCDocument/FCDSkinController.h>
#include <FCDocument/FCDGeometryMesh.h>

class DAEToEggConverter;

/**
 * Class representing an animated character.
 */
class DaeCharacter : public TypedReferenceCount {
public:
  DaeCharacter(EggGroup *node_group, const FCDControllerInstance* controller_instance);

  struct Joint {
    INLINE Joint(EggGroup *group, const FCDSceneNode *scene_node) :
      _group(group),
      _scene_node(scene_node),
      _character(nullptr),
      _bind_pose(LMatrix4d::ident_mat()) {}

    LMatrix4d _bind_pose;
    PT(EggGroup) _group;
    const FCDSceneNode *_scene_node;
    DaeCharacter *_character;
  };
  typedef epvector<Joint> Joints;
  typedef pmap<std::string, Joint> JointMap;

  void bind_joints(JointMap &joint_map);
  void adjust_joints(FCDSceneNode *node, const JointMap &joint_map,
                     const LMatrix4d &transform = LMatrix4d::ident_mat());

  void influence_vertex(int index, EggVertex *vertex);

  void collect_keys(pset<float> &keys);
  void r_collect_keys(FCDSceneNode *node, pset<float> &keys);

  void build_table(EggTable *parent, FCDSceneNode* node, const pset<float> &keys);

public:
  PT(EggGroup) _node_group;
  const FCDGeometryMesh *_skin_mesh;
  const FCDControllerInstance *_instance;
  LMatrix4d _bind_shape_mat;

private:
  std::string _name;
  const FCDSkinController *_skin_controller;
  Joints _joints;
  JointMap _bound_joints;

public:
  virtual TypeHandle get_type() const {
    return get_class_type();
  }
  virtual TypeHandle force_init_type() {init_type(); return get_class_type();}
  static TypeHandle get_class_type() {
    return _type_handle;
  }
  static void init_type() {
    TypedReferenceCount::init_type();
    register_type(_type_handle, "DaeCharacter",
                  TypedReferenceCount::get_class_type());
  }

private:
  static TypeHandle _type_handle;
};

#endif
