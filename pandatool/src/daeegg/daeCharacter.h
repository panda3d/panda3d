// Filename: daeCharacter.h
// Created by:  pro-rsoft (24Nov08)
//
////////////////////////////////////////////////////////////////////
//
// PANDA 3D SOFTWARE
// Copyright (c) Carnegie Mellon University.  All rights reserved.
//
// All use of this software is subject to the terms of the revised BSD
// license.  You should have received a copy of this license along
// with this source code in a file named "LICENSE."
//
////////////////////////////////////////////////////////////////////

#include "pandatoolbase.h"
#include "typedReferenceCount.h"
#include "typeHandle.h"
#include "eggTable.h"
#include "daeToEggConverter.h"

#include "pre_fcollada_include.h"
#include "FCollada.h"
#include "FCDocument/FCDSceneNode.h"
#include "FCDocument/FCDControllerInstance.h"
#include "FCDocument/FCDSkinController.h"

#ifndef DAECHARACTER_H
#define DAECHARACTER_H

////////////////////////////////////////////////////////////////////
//       Class : DaeCharacter
// Description : Class representing an animated character.
////////////////////////////////////////////////////////////////////
class DaeCharacter : public TypedReferenceCount {
public:
  DaeCharacter(const string name, const FCDControllerInstance* controller_instance);
  PT(EggTable) as_egg_bundle();
  void process_joint(PT(EggTable) parent, FCDSceneNode* node);
  
private:
  int _frame_rate;
  string _name;
  FCDControllerInstance* _controller_instance;
  FCDSkinController* _skin_controller;
  pmap<string, FCDSkinControllerJoint*> _controller_joints;
  
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
