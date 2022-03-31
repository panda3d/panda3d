/**
 * PANDA 3D SOFTWARE
 * Copyright (c) Carnegie Mellon University.  All rights reserved.
 *
 * All use of this software is subject to the terms of the revised BSD
 * license.  You should have received a copy of this license along
 * with this source code in a file named "LICENSE."
 *
 * @file daeCharacter.cxx
 * @author rdb
 * @date 2008-11-24
 */

#include "daeCharacter.h"
#include "config_daeegg.h"
#include "fcollada_utils.h"
#include "pt_EggVertex.h"
#include "eggXfmSAnim.h"
#include "daeToEggConverter.h"
#include "daeMaterials.h"

#include "eggExternalReference.h"

#include <FCDocument/FCDocument.h>
#include <FCDocument/FCDController.h>
#include <FCDocument/FCDGeometry.h>
#include <FCDocument/FCDSceneNodeTools.h>

#include <FCDocument/FCDSceneNode.h>
#include <FCDocument/FCDTransform.h>
#include <FCDocument/FCDAnimated.h>
#include <FCDocument/FCDAnimationCurve.h>
#include <FCDocument/FCDAnimationKey.h>

TypeHandle DaeCharacter::_type_handle;

/**
 *
 */
DaeCharacter::
DaeCharacter(EggGroup *node_group, const FCDControllerInstance *instance) :
  _node_group(node_group),
  _name(node_group->get_name()),
  _instance(instance),
  _skin_controller(nullptr),
  _skin_mesh(nullptr) {

  _bind_shape_mat = LMatrix4d::ident_mat();

  // If it's a skin controller, add the controller joints.
  const FCDController *controller = (const FCDController *)instance->GetEntity();
  if (controller == nullptr) {
    return;
  }
  _skin_mesh = controller->GetBaseGeometry()->GetMesh();

  if (controller->IsSkin()) {
    _skin_controller = controller->GetSkinController();
    _bind_shape_mat = DAEToEggConverter::convert_matrix(_skin_controller->GetBindShapeTransform());
  }
}

/**
 * Binds the joints to the character.  This means changing them to the bind
 * pose.  It is necessary to call this before process_skin_geometry.
 *
 * Returns the root group.
 */
void DaeCharacter::
bind_joints(JointMap &joint_map) {
  _joints.clear();

  size_t num_joints = _skin_controller->GetJointCount();
  _joints.reserve(num_joints);

  // Record the bind pose for each joint.
  for (size_t j = 0; j < num_joints; ++j) {
    const FCDSkinControllerJoint *skin_joint = _skin_controller->GetJoint(j);
    std::string sid = FROM_FSTRING(skin_joint->GetId());
    LMatrix4d bind_pose;
    bind_pose.invert_from(DAEToEggConverter::convert_matrix(
                          skin_joint->GetBindPoseInverse()));

    // Check that we already encountered this joint during traversal.
    JointMap::iterator ji = joint_map.find(sid);
    if (ji != joint_map.end()) {
      Joint &joint = ji->second;

      if (joint._character != nullptr) {
        // In some cases, though, multiple controllers share the same joints.
        // We can't support this without duplicating the joint structure, so
        // we check if the bind poses are the same.
        if (!joint._bind_pose.almost_equal(bind_pose, 0.0001)) {
          // Ugh.  What else could we do?
          daeegg_cat.error()
            << "Multiple controllers share joint with sid " << sid
            << ", with different bind poses.\n";
        }
      } else {
        // Mark the joint as being controlled by this character.
        joint._bind_pose = bind_pose;
        joint._character = this;
      }

      _joints.push_back(joint);
    } else {
      daeegg_cat.warning()
        << "Unknown joint sid being referenced: '" << sid << "'\n";

      // We still have to add a dummy joint or the index will be off.
      _joints.push_back(Joint(nullptr, nullptr));
    }
  }
}

/**
 * Traverses through the character hierarchy in order to bind the mesh to the
 * character.  This involves reorienting the joints to match the bind pose.
 *
 * It is important that this is called only once.
 */
void DaeCharacter::
adjust_joints(FCDSceneNode *node, const JointMap &joint_map,
              const LMatrix4d &transform) {

  LMatrix4d this_transform = transform;

  if (node->IsJoint()) {
    std::string sid = FROM_FSTRING(node->GetSubId());

    JointMap::const_iterator ji = joint_map.find(sid);
    if (ji != joint_map.end()) {
      const Joint &joint = ji->second;

      // Panda needs the joints to be in bind pose.  Not fun!  We copy the
      // joint transform to the default pose, though, so that Panda will
      // restore the joint transformation after binding.

      if (joint._character == this) {
        LMatrix4d bind_pose = joint._bind_pose * _bind_shape_mat *
                        invert(transform);
        // LMatrix4d bind_pose = joint._bind_pose * _bind_shape_mat *
        // joint._group->get_parent()->get_node_frame_inv();

        this_transform = bind_pose * this_transform;
        joint._group->set_default_pose(*joint._group);
        joint._group->set_transform3d(bind_pose);

        /*
        PT(EggGroup) sphere = new EggGroup;
        sphere->add_uniform_scale(0.1);
        sphere->set_group_type(EggGroup::GT_instance);
        sphere->add_child(new EggExternalReference("", "jack.egg"));
        joint._group->add_child(sphere);
        */
      }
    }
  } else {
    // this_transform = DAEToEggConverter::convert_matrix(node->ToMatrix());
  }

  // Loop through the children joints
  for (size_t ch = 0; ch < node->GetChildrenCount(); ++ch) {
    // if (node->GetChild(ch)->IsJoint()) {
    adjust_joints(node->GetChild(ch), joint_map, this_transform);
    // }
  }
}

/**
 * Adds the influences for the given vertex.
 */
void DaeCharacter::
influence_vertex(int index, EggVertex *vertex) {
  const FCDSkinControllerVertex *influence = _skin_controller->GetVertexInfluence(index);

  for (size_t pa = 0; pa < influence->GetPairCount(); ++pa) {
    const FCDJointWeightPair* jwpair = influence->GetPair(pa);

    if (jwpair->jointIndex >= 0 && jwpair->jointIndex < (int)_joints.size()) {
      EggGroup *joint = _joints[jwpair->jointIndex]._group.p();
      if (joint != nullptr) {
        joint->ref_vertex(vertex, jwpair->weight);
      }
    } else {
      daeegg_cat.error()
        << "Invalid joint index: " << jwpair->jointIndex << "\n";
    }
  }
}

/**
 * Collects all animation keys of animations applied to this character.
 */
void DaeCharacter::
collect_keys(pset<float> &keys) {
#if FCOLLADA_VERSION < 0x00030005
  FCDSceneNodeList roots = _instance->FindSkeletonNodes();
#else
  FCDSceneNodeList roots;
  _instance->FindSkeletonNodes(roots);
#endif

  for (FCDSceneNodeList::iterator it = roots.begin(); it != roots.end(); ++it) {
    r_collect_keys(*it, keys);
  }
}

/**
 * Collects all animation keys found for the given node tree.
 */
void DaeCharacter::
r_collect_keys(FCDSceneNode* node, pset<float> &keys) {
  FCDAnimatedList animateds;

  // Collect all the animation curves
  for (size_t t = 0; t < node->GetTransformCount(); ++t) {
    FCDTransform *transform = node->GetTransform(t);
    FCDAnimated *animated = transform->GetAnimated();

    if (animated != nullptr) {
      const FCDAnimationCurveListList &all_curves = animated->GetCurves();

      for (size_t ci = 0; ci < all_curves.size(); ++ci) {
        const FCDAnimationCurveTrackList &curves = all_curves[ci];
        if (curves.empty()) {
          continue;
        }

        size_t num_keys = curves.front()->GetKeyCount();
        const FCDAnimationKey **curve_keys = curves.front()->GetKeys();

        for (size_t c = 0; c < num_keys; ++c) {
          keys.insert(curve_keys[c]->input);
        }
      }
    }
  }
}

/**
 * Processes a joint node and its transforms.
 */
void DaeCharacter::
build_table(EggTable *parent, FCDSceneNode* node, const pset<float> &keys) {
  nassertv(node != nullptr);

  if (!node->IsJoint()) {
    for (size_t ch = 0; ch < node->GetChildrenCount(); ++ch) {
      build_table(parent, node->GetChild(ch), keys);
    }
    return;
  }

  std::string node_id = FROM_FSTRING(node->GetDaeId());
  PT(EggTable) table = new EggTable(node_id);
  table->set_table_type(EggTable::TT_table);
  parent->add_child(table);

  PT(EggXfmSAnim) xform = new EggXfmSAnim("xform");
  table->add_child(xform);

  // Generate the sampled animation and loop through the matrices
  FCDAnimatedList animateds;

  // Collect all the animation curves
  for (size_t t = 0; t < node->GetTransformCount(); ++t) {
    FCDTransform *transform = node->GetTransform(t);
    FCDAnimated *animated = transform->GetAnimated();
    if (animated != nullptr) {
      if (animated->HasCurve()) {
        animateds.push_back(animated);
      }
    }
  }

  // Sample the scene node transform
  float last_key;
  float timing_total = 0;
  pset<float>::const_iterator ki;
  for (ki = keys.begin(); ki != keys.end(); ++ki) {
    for (FCDAnimatedList::iterator it = animateds.begin(); it != animateds.end(); ++it) {
      // Sample each animated, which changes the transform values directly
      (*it)->Evaluate(*ki);
    }

    if (ki != keys.begin()) {
      timing_total += (*ki - last_key);
    }
    last_key = *ki;

    // Retrieve the new transform matrix for the COLLADA scene node
    FMMatrix44 fmat = node->ToMatrix();

    // Work around issue in buggy exporters (like ColladaMax)
    if (IS_NEARLY_ZERO(fmat[3][3])) {
      fmat[3][3] = 1;
    }

    xform->add_data(DAEToEggConverter::convert_matrix(fmat));
  }

  // Quantize the FPS, otherwise Panda complains about FPS mismatches.
  float fps = cfloor(((keys.size() - 1) / timing_total) * 100 + 0.5f) * 0.01f;
  xform->set_fps(fps);

  // Loop through the children joints
  for (size_t ch = 0; ch < node->GetChildrenCount(); ++ch) {
    // if (node->GetChild(ch)->IsJoint()) {
      build_table(table, node->GetChild(ch), keys);
    // }
  }
}
