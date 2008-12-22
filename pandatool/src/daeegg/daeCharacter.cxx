// Filename: daeCharacter.cxx
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

#include "daeCharacter.h"
#include "config_daeegg.h"
#include "fcollada_utils.h"
#include "pt_EggVertex.h"
#include "eggXfmSAnim.h"

#include "FCDocument/FCDocument.h"
#include "FCDocument/FCDController.h"
#include "FCDocument/FCDSceneNodeTools.h"

TypeHandle DaeCharacter::_type_handle;

////////////////////////////////////////////////////////////////////
//     Function: DaeCharacter::Constructor
//       Access: Public
//  Description: 
////////////////////////////////////////////////////////////////////
DaeCharacter::
DaeCharacter(const string name, const FCDControllerInstance* controller_instance) {
  _controller_instance = (FCDControllerInstance*) controller_instance;
  _name = name;
  _frame_rate = 0;
  _skin_controller = NULL;
  // If it's a skin controller, add the controller joints.
  FCDController* controller = (FCDController*) controller_instance->GetEntity();
  if (controller == NULL) return;
  if (controller->IsSkin()) {
    _skin_controller = controller->GetSkinController();
    if (_skin_controller == NULL) return;
    for (size_t j = 0; j < _skin_controller->GetJointCount(); ++j) {
      _controller_joints[FROM_FSTRING(_skin_controller->GetJoint(j)->GetId())] = (FCDSkinControllerJoint*) _skin_controller->GetJoint(j);
    }
  }
}

////////////////////////////////////////////////////////////////////
//     Function: DaeCharacter::as_egg_bundle
//       Access: Public
//  Description: Returns the character as a <Bundle> element,
//               suited for the animation table.
////////////////////////////////////////////////////////////////////
PT(EggTable) DaeCharacter::
as_egg_bundle() {
  PT(EggTable) bundle = new EggTable(_name);
  bundle->set_table_type(EggTable::TT_bundle);
  PT(EggTable) skeleton = new EggTable("<skeleton>");
  skeleton->set_table_type(EggTable::TT_table);
  bundle->add_child(skeleton);
  // Loop through the joint hierarchy
#if FCOLLADA_VERSION < 0x00030005
  FCDSceneNodeList roots = _controller_instance->FindSkeletonNodes();
#else
  FCDSceneNodeList roots;
  _controller_instance->FindSkeletonNodes(roots);
#endif
  for (FCDSceneNodeList::iterator it = roots.begin(); it != roots.end(); ++it) {
    process_joint(skeleton, *it);
  }
  return bundle;
}

////////////////////////////////////////////////////////////////////
//     Function: DaeCharacter::process_joint
//       Access: Public
//  Description: Processes a joint node and its transforms.
////////////////////////////////////////////////////////////////////
void DaeCharacter::
process_joint(PT(EggTable) parent, FCDSceneNode* node) {
  nassertv(node != NULL);
  string node_id = FROM_FSTRING(node->GetDaeId());
  PT(EggTable) joint = new EggTable(node_id);
  joint->set_table_type(EggTable::TT_table);
  parent->add_child(joint);
  PT(EggXfmSAnim) xform = new EggXfmSAnim("xform");
  joint->add_child(xform);
  xform->set_fps(_frame_rate);
  // Generate the sampled animation and loop through the matrices
  FCDSceneNodeTools::GenerateSampledAnimation(node);
  FMMatrix44List matrices = FCDSceneNodeTools::GetSampledAnimationMatrices();
  for (FMMatrix44List::const_iterator it = matrices.begin(); it != matrices.end(); ++it) {
    LMatrix4d matr = DAEToEggConverter::convert_matrix(*it);
    assert(xform->add_data(matr));
  }
  // Loop through the children joints
  for (size_t ch = 0; ch < node->GetChildrenCount(); ++ch) {
    if (node->GetChild(ch)->IsJoint()) {
      process_joint(joint, node->GetChild(ch));
    }
  }
}
