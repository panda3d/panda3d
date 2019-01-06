/**
 * PANDA 3D SOFTWARE
 * Copyright (c) Carnegie Mellon University.  All rights reserved.
 *
 * All use of this software is subject to the terms of the revised BSD
 * license.  You should have received a copy of this license along
 * with this source code in a file named "LICENSE."
 *
 * @file eggSaver.cxx
 * @author drose
 * @date 2012-12-19
 */

#include "eggSaver.h"

#include "pandaNode.h"
#include "workingNodePath.h"
#include "nodePath.h"
#include "billboardEffect.h"
#include "renderEffects.h"
#include "transformState.h"
#include "colorScaleAttrib.h"
#include "colorAttrib.h"
#include "materialAttrib.h"
#include "textureAttrib.h"
#include "cullBinAttrib.h"
#include "cullFaceAttrib.h"
#include "transparencyAttrib.h"
#include "depthTestAttrib.h"
#include "depthOffsetAttrib.h"
#include "depthWriteAttrib.h"
#include "lodNode.h"
#include "switchNode.h"
#include "sequenceNode.h"
#include "uvScrollNode.h"
#include "collisionNode.h"
#include "collisionPolygon.h"
#include "collisionPlane.h"
#include "collisionSphere.h"
#include "collisionBox.h"
#include "collisionInvSphere.h"
#include "collisionCapsule.h"
#include "textureStage.h"
#include "geomNode.h"
#include "geom.h"
#include "geomTriangles.h"
#include "geomPatches.h"
#include "geomPoints.h"
#include "geomLines.h"
#include "geomVertexReader.h"
#include "transformTable.h"
#include "modelNode.h"
#include "animBundleNode.h"
#include "animChannelMatrixXfmTable.h"
#include "characterJoint.h"
#include "character.h"
#include "string_utils.h"
#include "bamFile.h"
#include "bamCacheRecord.h"
#include "eggSAnimData.h"
#include "eggXfmAnimData.h"
#include "eggXfmSAnim.h"
#include "eggGroup.h"
#include "eggVertexPool.h"
#include "eggVertex.h"
#include "eggPrimitive.h"
#include "eggPolygon.h"
#include "eggPatch.h"
#include "eggPoint.h"
#include "eggLine.h"
#include "eggTexture.h"
#include "eggMaterial.h"
#include "eggRenderMode.h"
#include "eggTable.h"
#include "dcast.h"

using std::pair;
using std::string;

/**
 *
 */
EggSaver::
EggSaver(EggData *data) :
  _data(data)
{
  if (_data == nullptr) {
    _data = new EggData;
  }
}

/**
 * Adds the scene graph rooted at the indicated node to the accumulated egg
 * data within this object.  Call get_egg_data() to retrieve the result.
 */
void EggSaver::
add_node(PandaNode *node) {
  _vpool = new EggVertexPool(node->get_name());
  _data->add_child(_vpool);

  NodePath root(node);
  convert_node(WorkingNodePath(root), _data, false, nullptr);

  // Remove the vertex pool if it has no vertices.
  if (_vpool->empty()) {
    _data->remove_child(_vpool);
  }
  _vpool = nullptr;
}

/**
 * Adds the scene graph rooted at the indicated node (but without the node
 * itself) to the accumulated egg data within this object.  Call
 * get_egg_data() to retrieve the result.
 */
void EggSaver::
add_subgraph(PandaNode *root) {
  _vpool = new EggVertexPool(root->get_name());
  _data->add_child(_vpool);

  NodePath root_path(root);
  recurse_nodes(root_path, _data, false, nullptr);

  // Remove the vertex pool if it has no vertices.
  if (_vpool->empty()) {
    _data->remove_child(_vpool);
  }
  _vpool = nullptr;
}

/**
 * Converts the indicated node to the corresponding Egg constructs, by first
 * determining what kind of node it is.
 */
void EggSaver::
convert_node(const WorkingNodePath &node_path, EggGroupNode *egg_parent,
             bool has_decal, CharacterJointMap *joint_map) {
  PandaNode *node = node_path.node();
  if (node->is_geom_node()) {
    convert_geom_node(DCAST(GeomNode, node), node_path, egg_parent, has_decal, joint_map);

  } else if (node->is_of_type(LODNode::get_class_type())) {
    convert_lod_node(DCAST(LODNode, node), node_path, egg_parent, has_decal, joint_map);

  } else if (node->is_of_type(SequenceNode::get_class_type())) {
    convert_sequence_node(DCAST(SequenceNode, node), node_path, egg_parent, has_decal, joint_map);

  } else if (node->is_of_type(SwitchNode::get_class_type())) {
    convert_switch_node(DCAST(SwitchNode, node), node_path, egg_parent, has_decal, joint_map);

  } else if (node->is_of_type(CollisionNode::get_class_type())) {
    convert_collision_node(DCAST(CollisionNode, node), node_path, egg_parent, has_decal, joint_map);

  } else if (node->is_of_type(AnimBundleNode::get_class_type())) {
    convert_anim_node(DCAST(AnimBundleNode, node), node_path, egg_parent, has_decal);

  } else if (node->is_of_type(Character::get_class_type())) {
    convert_character_node(DCAST(Character, node), node_path, egg_parent, has_decal);

  } else {
    // Just a generic node.
    EggGroup *egg_group = new EggGroup(node->get_name());
    egg_parent->add_child(egg_group);
    apply_node_properties(egg_group, node);

    recurse_nodes(node_path, egg_group, has_decal, joint_map);
  }
}

/**
 * Converts the indicated LODNode to the corresponding Egg constructs.
 */
void EggSaver::
convert_lod_node(LODNode *node, const WorkingNodePath &node_path,
                 EggGroupNode *egg_parent, bool has_decal,
                 CharacterJointMap *joint_map) {
  // An LOD node gets converted to an ordinary EggGroup, but we apply the
  // appropriate switch conditions to each of our children.
  EggGroup *egg_group = new EggGroup(node->get_name());
  egg_parent->add_child(egg_group);
  apply_node_properties(egg_group, node);

  int num_children = node->get_num_children();
  int num_switches = node->get_num_switches();

  num_children = std::min(num_children, num_switches);

  for (int i = 0; i < num_children; i++) {
    PandaNode *child = node->get_child(i);

    // Convert just this one node to an EggGroup.
    PT(EggGroup) next_group = new EggGroup;
    convert_node(WorkingNodePath(node_path, child), next_group, has_decal, joint_map);

    if (next_group->size() == 1) {
      // If we have exactly one child, and that child is an EggGroup,
      // collapse.
      EggNode *child_node = *next_group->begin();
      if (child_node->is_of_type(EggGroup::get_class_type())) {
        PT(EggGroup) child = DCAST(EggGroup, child_node);
        next_group->remove_child(child.p());
        next_group = child;
      }
    }

    // Now set up the switching properties appropriately.
    PN_stdfloat in = node->get_in(i);
    PN_stdfloat out = node->get_out(i);
    LPoint3 center = node->get_center();
    EggSwitchConditionDistance dist(in, out, LCAST(double, center));
    next_group->set_lod(dist);
    egg_group->add_child(next_group.p());
  }
}

/**
 * Converts the indicated SequenceNode to the corresponding Egg constructs.
 */
void EggSaver::
convert_sequence_node(SequenceNode *node, const WorkingNodePath &node_path,
                      EggGroupNode *egg_parent, bool has_decal,
                      CharacterJointMap *joint_map) {
  // A sequence node gets converted to an ordinary EggGroup, we only apply the
  // appropriate switch attributes to turn it into a sequence
  EggGroup *egg_group = new EggGroup(node->get_name());
  egg_parent->add_child(egg_group);
  apply_node_properties(egg_group, node);

  // turn it into a sequence with the right frame-rate
  egg_group->set_switch_flag(true);
  egg_group->set_switch_fps(node->get_frame_rate());

  int num_children = node->get_num_children();

  for (int i = 0; i < num_children; i++) {
    PandaNode *child = node->get_child(i);

    // Convert just this one node to an EggGroup.
    PT(EggGroup) next_group = new EggGroup;
    convert_node(WorkingNodePath(node_path, child), next_group, has_decal, joint_map);

    egg_group->add_child(next_group.p());
  }

}

/**
 * Converts the indicated SwitchNode to the corresponding Egg constructs.
 */
void EggSaver::
convert_switch_node(SwitchNode *node, const WorkingNodePath &node_path,
                    EggGroupNode *egg_parent, bool has_decal,
                    CharacterJointMap *joint_map) {
  // A sequence node gets converted to an ordinary EggGroup, we only apply the
  // appropriate switch attributes to turn it into a sequence
  EggGroup *egg_group = new EggGroup(node->get_name());
  egg_parent->add_child(egg_group);
  apply_node_properties(egg_group, node);

  // turn it into a switch..
  egg_group->set_switch_flag(true);

  int num_children = node->get_num_children();

  for (int i = 0; i < num_children; i++) {
    PandaNode *child = node->get_child(i);

    // Convert just this one node to an EggGroup.
    PT(EggGroup) next_group = new EggGroup;
    convert_node(WorkingNodePath(node_path, child), next_group, has_decal, joint_map);

    egg_group->add_child(next_group.p());
  }
}

/**
 * Converts the indicated AnimationGroupNodes to the corresponding Egg
 * constructs.
 */
EggGroupNode * EggSaver::convert_animGroup_node(AnimGroup *animGroup, double fps ) {
  int num_children = animGroup->get_num_children();

  EggGroupNode *eggNode = nullptr;
  if (animGroup->is_of_type(AnimBundle::get_class_type())) {
    EggTable *eggTable = new EggTable(animGroup->get_name());
    eggTable ->set_table_type(EggTable::TT_bundle);
    eggNode = eggTable;
  } else if (animGroup->is_of_type(AnimGroup::get_class_type())) {
    EggTable *eggTable = new EggTable(animGroup->get_name());
    eggTable ->set_table_type(EggTable::TT_table);
    eggNode = eggTable;
  }

  if (animGroup->is_of_type(AnimChannelMatrixXfmTable::get_class_type())) {
    AnimChannelMatrixXfmTable *xmfTable = DCAST(AnimChannelMatrixXfmTable, animGroup);
    EggXfmSAnim *egg_anim = new EggXfmSAnim("xform");
    egg_anim->set_fps(fps);
    for (int i = 0; i < num_matrix_components; i++) {
      string componentName(1, matrix_component_letters[i]);
      char table_id = matrix_component_letters[i];
      CPTA_stdfloat table = xmfTable->get_table(table_id);

      if (xmfTable->has_table(table_id)) {
        for (unsigned int j = 0; j < table.size(); j++) {
          egg_anim->add_component_data(componentName, table[(int)j]);
        }
      }
    }
    eggNode->add_child(egg_anim);
  }
  for (int i = 0; i < num_children; i++) {
    AnimGroup *animChild = animGroup->get_child(i);
    EggGroupNode *eggChildNode = convert_animGroup_node(animChild, fps);
    if (eggChildNode!=nullptr) {
      nassertr(eggNode!=nullptr, nullptr);
      eggNode->add_child(eggChildNode);
    }
  }
  return eggNode;
}

/**
 * Converts the indicated AnimNode to the corresponding Egg constructs.
 */
void EggSaver::
convert_anim_node(AnimBundleNode *node, const WorkingNodePath &node_path,
                    EggGroupNode *egg_parent, bool has_decal) {

  // A sequence node gets converted to an ordinary EggGroup, we only apply the
  // appropriate switch attributes to turn it into a sequence
  EggTable *eggTable = new EggTable();
  // egg_parent->add_child(eggTable);
  _data->add_child(eggTable);

  AnimBundle *animBundle = node->get_bundle();
  // turn it into a switch.. egg_group->set_switch_flag(true);

  EggGroupNode *eggAnimation = convert_animGroup_node(animBundle, animBundle->get_base_frame_rate());
  eggTable->add_child(eggAnimation);
}

/**
 * Converts the indicated Character Bundle to the corresponding Egg joints
 * structure.
 */
void EggSaver::
convert_character_bundle(PartGroup *bundleNode, EggGroupNode *egg_parent, CharacterJointMap *joint_map) {
  int num_children = bundleNode->get_num_children();

  EggGroupNode *joint_group = egg_parent;
  if (bundleNode->is_of_type(CharacterJoint::get_class_type())) {
    CharacterJoint *character_joint = DCAST(CharacterJoint, bundleNode);

    LMatrix4 transformf;
    character_joint->get_transform(transformf);
    LMatrix4d transformd(LCAST(double, transformf));
    EggGroup *joint = new EggGroup(bundleNode->get_name());
    joint->add_matrix4(transformd);
    joint->set_group_type(EggGroup::GT_joint);
    joint_group = joint;
    egg_parent->add_child(joint_group);
    if (joint_map != nullptr) {
      CharacterJointMap::iterator mi = joint_map->find(character_joint);
      if (mi != joint_map->end()) {
        pvector<pair<EggVertex*,PN_stdfloat> > &joint_vertices = (*mi).second;
        pvector<pair<EggVertex*,PN_stdfloat> >::const_iterator vi;
        for (vi = joint_vertices.begin(); vi != joint_vertices.end(); ++vi) {
          joint->set_vertex_membership((*vi).first, (*vi).second);
        }
      }
    }
  }

  for (int i = 0; i < num_children ; i++) {
    PartGroup *partGroup= bundleNode->get_child(i);
    convert_character_bundle(partGroup, joint_group, joint_map);
  }

}

/**
 * Converts the indicated Character to the corresponding Egg constructs.
 */
void EggSaver::
convert_character_node(Character *node, const WorkingNodePath &node_path,
                       EggGroupNode *egg_parent, bool has_decal) {

  // A sequence node gets converted to an ordinary EggGroup, we only apply the
  // appropriate switch attributes to turn it into a sequence.
  // We have to use DT_structured since it is the only mode that preserves the
  // node hierarchy, including LODNodes and CollisionNodes that may be under
  // this Character node.
  EggGroup *egg_group = new EggGroup(node->get_name());
  egg_group->set_dart_type(EggGroup::DT_structured);
  egg_parent->add_child(egg_group);
  apply_node_properties(egg_group, node);

  CharacterJointMap joint_map;
  recurse_nodes(node_path, egg_group, has_decal, &joint_map);

  // turn it into a switch.. egg_group->set_switch_flag(true);

  int num_bundles = node->get_num_bundles();
  for (int i = 0; i < num_bundles; ++i) {
    PartBundle *bundle = node->get_bundle(i);
    convert_character_bundle(bundle, egg_group, &joint_map);
  }
}


/**
 * Converts the indicated CollisionNode to the corresponding Egg constructs.
 */
void EggSaver::
convert_collision_node(CollisionNode *node, const WorkingNodePath &node_path,
                       EggGroupNode *egg_parent, bool has_decal,
                       CharacterJointMap *joint_map) {
  // A sequence node gets converted to an ordinary EggGroup, we only apply the
  // appropriate switch attributes to turn it into a sequence
  EggGroup *egg_group = new EggGroup(node->get_name());
  egg_parent->add_child(egg_group);
  apply_node_properties(egg_group, node, false);

  // Set the collision masks, if present.
  CollideMask from_mask = node->get_from_collide_mask();
  CollideMask into_mask = node->get_into_collide_mask();
  if (from_mask != CollisionNode::get_default_collide_mask() ||
      into_mask != CollisionNode::get_default_collide_mask()) {
    if (from_mask == into_mask) {
      egg_group->set_collide_mask(into_mask);
    } else {
      egg_group->set_from_collide_mask(from_mask);
      egg_group->set_into_collide_mask(into_mask);
    }
  }

  // turn it into a collision node
  egg_group->set_collide_flags(EggGroup::CF_descend);

  NodePath np = node_path.get_node_path();
  CPT(TransformState) net_transform = np.get_net_transform();
  LMatrix4 net_mat = net_transform->get_mat();
  LMatrix4 inv = LCAST(PN_stdfloat, egg_parent->get_vertex_frame_inv());
  net_mat = net_mat * inv;

  int num_solids = node->get_num_solids();

  if (num_solids > 0) {
    // create vertex pool for collisions
    EggVertexPool *cvpool = new EggVertexPool("vpool-collision");
    egg_group->add_child(cvpool);

    // traverse solids
    for (int i = 0; i < num_solids; i++) {
      CPT(CollisionSolid) child = node->get_solid(i);
      int flags = EggGroup::CF_descend;

      if (!child->is_tangible()) {
        flags |= EggGroup::CF_intangible;
      }

      if (child->has_effective_normal() &&
          child->get_effective_normal() == LVector3::up()) {
        flags |= EggGroup::CF_level;
      }

      if (child->is_of_type(CollisionPolygon::get_class_type())) {
        egg_group->set_cs_type(EggGroup::CST_polyset);
        egg_group->set_collide_flags(flags);

        EggPolygon *egg_poly = new EggPolygon;
        egg_group->add_child(egg_poly);

        CPT(CollisionPolygon) poly = DCAST(CollisionPolygon, child);
        int num_points = poly->get_num_points();
        for (int j = 0; j < num_points; j++) {
          EggVertex egg_vert;
          egg_vert.set_pos(LCAST(double, poly->get_point(j) * net_mat));
          egg_vert.set_normal(LCAST(double, poly->get_normal() * net_mat));

          EggVertex *new_egg_vert = cvpool->create_unique_vertex(egg_vert);
          egg_poly->add_vertex(new_egg_vert);
        }

      } else if (child->is_of_type(CollisionSphere::get_class_type())) {
        CPT(CollisionSphere) sphere = DCAST(CollisionSphere, child);
        LPoint3 center = sphere->get_center();
        PN_stdfloat radius = sphere->get_radius();

        EggGroup *egg_sphere;
        if (num_solids == 1) {
          egg_sphere = egg_group;
        } else {
          egg_sphere = new EggGroup;
          egg_group->add_child(egg_sphere);
        }

        if (child->is_of_type(CollisionInvSphere::get_class_type())) {
          egg_sphere->set_cs_type(EggGroup::CST_inv_sphere);
        } else {
          egg_sphere->set_cs_type(EggGroup::CST_sphere);
        }
        egg_sphere->set_collide_flags(flags);

        EggVertex ev1, ev2, ev3, ev4;
        ev1.set_pos(LCAST(double, (center + LVector3(radius, 0, 0)) * net_mat));
        ev2.set_pos(LCAST(double, (center + LVector3(0, radius, 0)) * net_mat));
        ev3.set_pos(LCAST(double, (center + LVector3(-radius, 0, 0)) * net_mat));
        ev4.set_pos(LCAST(double, (center + LVector3(0, -radius, 0)) * net_mat));

        EggPolygon *egg_poly = new EggPolygon;
        egg_sphere->add_child(egg_poly);

        egg_poly->add_vertex(cvpool->create_unique_vertex(ev1));
        egg_poly->add_vertex(cvpool->create_unique_vertex(ev2));
        egg_poly->add_vertex(cvpool->create_unique_vertex(ev3));
        egg_poly->add_vertex(cvpool->create_unique_vertex(ev4));

      } else if (child->is_of_type(CollisionPlane::get_class_type())) {
        LPlane plane = DCAST(CollisionPlane, child)->get_plane();
        LPoint3 origin = plane.get_point();
        LVector3 normal = plane.get_normal();

        // Get an arbitrary vector on the plane by taking the cross product
        // with any vector, as long as it is different.
        LVector3 vec1;
        if (std::fabs(normal[2]) > std::fabs(normal[1])) {
          vec1 = normal.cross(LVector3(0, 1, 0));
        } else {
          vec1 = normal.cross(LVector3(0, 0, 1));
        }

        // Find a second vector perpendicular to the two.
        LVector3 vec2 = normal.cross(vec1);

        EggGroup *egg_plane;
        if (num_solids == 1) {
          egg_plane = egg_group;
        } else {
          egg_plane = new EggGroup;
          egg_group->add_child(egg_plane);
        }
        egg_plane->set_cs_type(EggGroup::CST_plane);
        egg_plane->set_collide_flags(flags);

        EggVertex ev0, ev1, ev2;
        ev0.set_pos(LCAST(double, origin * net_mat));
        ev1.set_pos(LCAST(double, (origin + vec1) * net_mat));
        ev2.set_pos(LCAST(double, (origin + vec2) * net_mat));

        EggPolygon *egg_poly = new EggPolygon;
        egg_plane->add_child(egg_poly);

        egg_poly->add_vertex(cvpool->create_unique_vertex(ev0));
        egg_poly->add_vertex(cvpool->create_unique_vertex(ev1));
        egg_poly->add_vertex(cvpool->create_unique_vertex(ev2));

      } else if (child->is_of_type(CollisionBox::get_class_type())) {
        CPT(CollisionBox) box = DCAST(CollisionBox, child);
        LPoint3 min_point = box->get_min();
        LPoint3 max_point = box->get_max();

        EggGroup *egg_box;
        if (num_solids == 1) {
          egg_box = egg_group;
        } else {
          egg_box = new EggGroup;
          egg_group->add_child(egg_box);
        }
        egg_box->set_cs_type(EggGroup::CST_box);
        egg_box->set_collide_flags(flags);

        // Just add the min and max points.
        EggVertex ev0, ev1;
        ev0.set_pos(LCAST(double, min_point * net_mat));
        ev1.set_pos(LCAST(double, max_point * net_mat));

        EggLine *egg_poly = new EggLine;
        egg_box->add_child(egg_poly);

        egg_poly->add_vertex(cvpool->create_unique_vertex(ev0));
        egg_poly->add_vertex(cvpool->create_unique_vertex(ev1));

      } else if (child->is_of_type(CollisionCapsule::get_class_type())) {
        CPT(CollisionCapsule) capsule = DCAST(CollisionCapsule, child);
        LPoint3 point_a = capsule->get_point_a();
        LPoint3 point_b = capsule->get_point_b();
        LPoint3 centroid = (point_a + point_b) * 0.5f;

        // Also get an arbitrary vector perpendicular to the capsule.
        LVector3 axis = point_b - point_a;
        LVector3 sideways;
        if (std::fabs(axis[2]) > std::fabs(axis[1])) {
          sideways = axis.cross(LVector3(0, 1, 0));
        } else {
          sideways = axis.cross(LVector3(0, 0, 1));
        }
        sideways.normalize();
        sideways *= capsule->get_radius();
        LVector3 extend = axis.normalized() * capsule->get_radius();

        EggGroup *egg_capsule;
        if (num_solids == 1) {
          egg_capsule = egg_group;
        } else {
          egg_capsule = new EggGroup;
          egg_group->add_child(egg_capsule);
        }
        egg_capsule->set_cs_type(EggGroup::CST_tube);
        egg_capsule->set_collide_flags(flags);

        // Add two points for the endcaps, and then two points around the
        // centroid to indicate the radius.
        EggVertex ev0, ev1, ev2, ev3;
        ev0.set_pos(LCAST(double, (point_a - extend) * net_mat));
        ev1.set_pos(LCAST(double, (centroid + sideways) * net_mat));
        ev2.set_pos(LCAST(double, (point_b + extend) * net_mat));
        ev3.set_pos(LCAST(double, (centroid - sideways) * net_mat));

        EggPolygon *egg_poly = new EggPolygon;
        egg_capsule->add_child(egg_poly);

        egg_poly->add_vertex(cvpool->create_unique_vertex(ev0));
        egg_poly->add_vertex(cvpool->create_unique_vertex(ev1));
        egg_poly->add_vertex(cvpool->create_unique_vertex(ev2));
        egg_poly->add_vertex(cvpool->create_unique_vertex(ev3));

      } else {
        nout << "Encountered unknown collision solid type " << child->get_type() << "\n";
      }
    }
  }

  // recurse over children - hm.  do I need to do this?
  recurse_nodes(node_path, egg_group, has_decal, joint_map);
}

/**
 * Converts a GeomNode to the corresponding egg structures.
 */
void EggSaver::
convert_geom_node(GeomNode *node, const WorkingNodePath &node_path,
                  EggGroupNode *egg_parent, bool has_decal, CharacterJointMap *joint_map) {
  PT(EggGroup) egg_group = new EggGroup(node->get_name());
  bool fancy_attributes = apply_node_properties(egg_group, node);

  if (node->get_effects()->has_decal()) {
    has_decal = true;
  }

  if (has_decal) {
    egg_group->set_decal_flag(true);
  }

  if (fancy_attributes || has_decal || !node->get_name().empty()) {
    // If we have any fancy attributes on the node, or if we're making decal
    // geometry, we have to make a special node to hold the geometry (normally
    // it would just appear within its parent).
    egg_parent->add_child(egg_group.p());
    egg_parent = egg_group;
  }

  NodePath np = node_path.get_node_path();
  CPT(RenderState) net_state = np.get_net_state();
  CPT(TransformState) net_transform = np.get_net_transform();
  LMatrix4 net_mat = net_transform->get_mat();
  LMatrix4 inv = LCAST(PN_stdfloat, egg_parent->get_vertex_frame_inv());
  net_mat = net_mat * inv;

  // Now get out all the various kinds of geometry.
  int num_geoms = node->get_num_geoms();
  for (int i = 0; i < num_geoms; ++i) {
    CPT(RenderState) geom_state = node->get_geom_state(i);
    CPT(RenderState) geom_net_state = net_state->compose(geom_state);

    // If there is only one Geom, and the node has no state, apply the state
    // attributes from the Geom to the group instead, so that we don't end up
    // duplicating it for a lot of primitives.
    if (num_geoms == 1 && node->get_num_children() == 0 && egg_parent == egg_group &&
        !geom_state->is_empty() && node->get_state()->is_empty()) {
      apply_state_properties(egg_group, geom_state);
      geom_state = RenderState::make_empty();
    }

    const Geom *geom = node->get_geom(i);
    int num_primitives = geom->get_num_primitives();
    for (int j = 0; j < num_primitives; ++j) {
      const GeomPrimitive *primitive = geom->get_primitive(j);
      CPT(GeomPrimitive) simple = primitive->decompose();
      CPT(GeomVertexData) vdata = geom->get_vertex_data();
      // vdata = vdata->animate_vertices(true, Thread::get_current_thread());
      convert_primitive(vdata, simple, geom_state, geom_net_state,
                        net_mat, egg_parent, joint_map);
    }
  }

  recurse_nodes(node_path, egg_parent, has_decal, joint_map);
}

/**
 *
 */
void EggSaver::
convert_primitive(const GeomVertexData *vertex_data,
                  const GeomPrimitive *primitive,
                  const RenderState *geom_state, const RenderState *net_state,
                  const LMatrix4 &net_mat, EggGroupNode *egg_parent,
                  CharacterJointMap *joint_map) {
  GeomVertexReader reader(vertex_data);

  // Make a zygote that will be duplicated for each primitive.
  PT(EggPrimitive) egg_prim;
  if (primitive->is_of_type(GeomTriangles::get_class_type())) {
    egg_prim = new EggPolygon();
  } else if (primitive->is_of_type(GeomPatches::get_class_type())) {
    egg_prim = new EggPatch();
  } else if (primitive->is_of_type(GeomPoints::get_class_type())) {
    egg_prim = new EggPoint();
  } else if (primitive->is_of_type(GeomLines::get_class_type())) {
    egg_prim = new EggLine();
  } else {
    // Huh, an unknown geometry type.
    return;
  }

  // Apply render attributes.
  apply_state_properties(egg_prim, geom_state);

  // Check for a color scale.
  LVecBase4 color_scale(1.0f, 1.0f, 1.0f, 1.0f);
  const ColorScaleAttrib *csa;
  if (net_state->get_attrib(csa)) {
    color_scale = csa->get_scale();
  }

  // Check for a color override.
  bool has_color_override = false;
  bool has_color_off = false;
  LColor color_override;
  const ColorAttrib *ca;
  if (net_state->get_attrib(ca)) {
    if (ca->get_color_type() == ColorAttrib::T_flat) {
      has_color_override = true;
      color_override = ca->get_color();
      color_override.set(color_override[0] * color_scale[0],
                         color_override[1] * color_scale[1],
                         color_override[2] * color_scale[2],
                         color_override[3] * color_scale[3]);

    } else if (ca->get_color_type() == ColorAttrib::T_off) {
      has_color_off = true;
    }
  }

  // Check for a material.
  EggMaterial *egg_mat = nullptr;
  const MaterialAttrib *ma;
  if (net_state->get_attrib(ma)) {
    egg_mat = get_egg_material(ma->get_material());
    if (egg_mat != nullptr) {
      egg_prim->set_material(egg_mat);
    }
  }

  // Check for a texture.
  const TextureAttrib *ta;
  if (net_state->get_attrib(ta)) {
    EggTexture *egg_tex = get_egg_texture(ta->get_texture());

    if (egg_tex != nullptr) {
      TextureStage *tex_stage = ta->get_on_stage(0);
      if (tex_stage != nullptr) {
        switch (tex_stage->get_mode()) {
        case TextureStage::M_modulate:
          if (has_color_off == true) {
            egg_tex->set_env_type(EggTexture::ET_replace);
          } else {
            egg_tex->set_env_type(EggTexture::ET_modulate);
          }
          break;
        case TextureStage::M_decal:
          egg_tex->set_env_type(EggTexture::ET_decal);
          break;
        case TextureStage::M_blend:
          egg_tex->set_env_type(EggTexture::ET_blend);
          break;
        case TextureStage::M_replace:
          egg_tex->set_env_type(EggTexture::ET_replace);
          break;
        case TextureStage::M_add:
          egg_tex->set_env_type(EggTexture::ET_add);
          break;
        case TextureStage::M_blend_color_scale:
          egg_tex->set_env_type(EggTexture::ET_blend_color_scale);
          break;
        default:
          break;
        }
      }

      egg_prim->set_texture(egg_tex);
    }
  }

  // Check the backface flag.
  const CullFaceAttrib *cfa;
  if (net_state->get_attrib(cfa)) {
    if (cfa->get_effective_mode() == CullFaceAttrib::M_cull_none) {
      egg_prim->set_bface_flag(true);
    }
  }

  // Check for line thickness and such.
  const RenderModeAttrib *rma;
  if (net_state->get_attrib(rma)) {
    if (egg_prim->is_of_type(EggPoint::get_class_type())) {
      EggPoint *egg_point = (EggPoint *)egg_prim.p();
      egg_point->set_thick(rma->get_thickness());
      egg_point->set_perspective(rma->get_perspective());

    } else if (egg_prim->is_of_type(EggLine::get_class_type())) {
      EggLine *egg_line = (EggLine *)egg_prim.p();
      egg_line->set_thick(rma->get_thickness());
    }
  }

  CPT(TransformBlendTable) transformBlendTable = vertex_data->get_transform_blend_table();

  int num_primitives = primitive->get_num_primitives();
  int num_vertices = primitive->get_num_vertices_per_primitive();

  for (int i = 0; i < num_primitives; ++i) {
    PT(EggPrimitive) egg_child = egg_prim->make_copy();
    egg_parent->add_child(egg_child);

    for (int j = 0; j < num_vertices; j++) {
      EggVertex egg_vert;

      // Get per-vertex properties.
      reader.set_row(primitive->get_vertex(i * num_vertices + j));

      reader.set_column(InternalName::get_vertex());
      LVertex vertex = reader.get_data3();
      egg_vert.set_pos(LCAST(double, vertex * net_mat));

      if (vertex_data->has_column(InternalName::get_normal())) {
        reader.set_column(InternalName::get_normal());
        LNormal normal = reader.get_data3();
        egg_vert.set_normal(LCAST(double, normal * net_mat));
      }
      if (has_color_override) {
        egg_vert.set_color(color_override);

      } else if (!has_color_off) {
        LColor color(1.0f, 1.0f, 1.0f, 1.0f);
        if (vertex_data->has_column(InternalName::get_color())) {
          reader.set_column(InternalName::get_color());
          color = reader.get_data4();
        }
        egg_vert.set_color(LColor(color[0] * color_scale[0],
                                  color[1] * color_scale[1],
                                  color[2] * color_scale[2],
                                  color[3] * color_scale[3]));
      }

      if (vertex_data->has_column(InternalName::get_texcoord())) {
        reader.set_column(InternalName::get_texcoord());
        LTexCoord uv = reader.get_data2();
        egg_vert.set_uv(LCAST(double, uv));
      }

      EggVertex *new_egg_vert = _vpool->create_unique_vertex(egg_vert);

      if (vertex_data->has_column(InternalName::get_transform_blend()) &&
          joint_map != nullptr && transformBlendTable != nullptr) {
        reader.set_column(InternalName::get_transform_blend());
        int idx = reader.get_data1i();
        const TransformBlend &blend = transformBlendTable->get_blend(idx);
        int num_weights = blend.get_num_transforms();
        for (int k = 0; k < num_weights; ++k) {
          PN_stdfloat weight = blend.get_weight(k);
          if (weight!=0) {
            const VertexTransform *vertex_transform = blend.get_transform(k);
            if (vertex_transform->is_of_type(JointVertexTransform::get_class_type())) {
              const JointVertexTransform *joint_vertex_transform = DCAST(const JointVertexTransform, vertex_transform);

              CharacterJointMap::iterator mi = joint_map->find(joint_vertex_transform->get_joint());
              if (mi == joint_map->end()) {
                mi = joint_map->insert(CharacterJointMap::value_type(joint_vertex_transform->get_joint(), pvector<pair<EggVertex*,PN_stdfloat> >())).first;
              }
              pvector<pair<EggVertex*,PN_stdfloat> > &joint_vertices = (*mi).second;
              joint_vertices.push_back(pair<EggVertex*,PN_stdfloat>(new_egg_vert, weight));
            }
          }
        }
      }

      egg_child->add_vertex(new_egg_vert);
    }
  }
}

/**
 * Converts all the children of the indicated node.
 */
void EggSaver::
recurse_nodes(const WorkingNodePath &node_path, EggGroupNode *egg_parent,
              bool has_decal, CharacterJointMap *joint_map) {
  PandaNode *node = node_path.node();
  int num_children = node->get_num_children();

  for (int i = 0; i < num_children; i++) {
    PandaNode *child = node->get_child(i);
    convert_node(WorkingNodePath(node_path, child), egg_parent, has_decal, joint_map);
  }
}

/**
 * Applies any special properties that might be stored on the node, like
 * billboarding.  Returns true if any were applied, false otherwise.
 */
bool EggSaver::
apply_node_properties(EggGroup *egg_group, PandaNode *node, bool allow_backstage) {
  bool any_applied = false;

  if (node->is_overall_hidden() && allow_backstage) {
    // This node is hidden.  We'll go ahead and convert it, but we'll put in
    // the "backstage" flag to mean it's not real geometry.  unless the caller
    // wants to keep it (by setting allow_backstage to false)
    egg_group->add_object_type("backstage");
  }

  if (node->has_tags()) {
    if (apply_tags(egg_group, node)) {
      any_applied = true;
    }
  }

  if (node->is_of_type(ModelNode::get_class_type())) {
    ModelNode *model_node = DCAST(ModelNode, node);
    switch (model_node->get_preserve_transform()) {
    case ModelNode::PT_none:
      egg_group->set_model_flag(true);
      break;

    case ModelNode::PT_drop_node:
      break;

    case ModelNode::PT_net:
      egg_group->set_dcs_type(EggGroup::DC_net);
      break;

    case ModelNode::PT_local:
      egg_group->set_dcs_type(EggGroup::DC_local);
      break;

    case ModelNode::PT_no_touch:
      egg_group->set_dcs_type(EggGroup::DC_no_touch);
      break;
    }
  }

  if (node->is_of_type(UvScrollNode::get_class_type())) {
    const UvScrollNode *scroll_node = (const UvScrollNode *)node;
    egg_group->set_scroll_u(scroll_node->get_u_speed());
    egg_group->set_scroll_v(scroll_node->get_v_speed());
    egg_group->set_scroll_w(scroll_node->get_w_speed());
    egg_group->set_scroll_r(scroll_node->get_r_speed());
  }

  const RenderEffects *effects = node->get_effects();
  const RenderEffect *effect = effects->get_effect(BillboardEffect::get_class_type());
  if (effect != nullptr) {
    const BillboardEffect *bbe = DCAST(BillboardEffect, effect);
    if (bbe->get_axial_rotate()) {
      egg_group->set_billboard_type(EggGroup::BT_axis);
      any_applied = true;

    } else if (bbe->get_eye_relative()) {
      egg_group->set_billboard_type(EggGroup::BT_point_camera_relative);
      any_applied = true;

    } else {
      egg_group->set_billboard_type(EggGroup::BT_point_world_relative);
      any_applied = true;
    }
  }

  const TransformState *transform = node->get_transform();
  if (!transform->is_identity()) {
    if (transform->has_components()) {
      // If the transform can be represented componentwise, we prefer storing
      // it that way in the egg file.
      const LVecBase3 &scale = transform->get_scale();
      const LQuaternion &quat = transform->get_quat();
      const LVecBase3 &pos = transform->get_pos();
      if (!scale.almost_equal(LVecBase3(1.0f, 1.0f, 1.0f))) {
        egg_group->add_scale3d(LCAST(double, scale));
      }
      if (!quat.is_identity()) {
        egg_group->add_rotate3d(LCAST(double, quat));
      }
      if (!pos.almost_equal(LVecBase3::zero())) {
        egg_group->add_translate3d(LCAST(double, pos));
      }

    } else if (transform->has_mat()) {
      // Otherwise, we store the raw matrix.
      const LMatrix4 &mat = transform->get_mat();
      egg_group->set_transform3d(LCAST(double, mat));
    }
    any_applied = true;
  }

  const RenderState *state = node->get_state();
  if (apply_state_properties(egg_group, state)) {
    return true;
  }

  return any_applied;
}

/**
 * Applies any special render state settings on the primitive or group.
 * Returns true if any were applied, false otherwise.
 */
bool EggSaver::
apply_state_properties(EggRenderMode *egg_render_mode, const RenderState *state) {
  if (state->is_empty()) {
    return false;
  }

  bool any_applied = false;

  // Check the transparency mode.
  const TransparencyAttrib *tra;
  if (state->get_attrib(tra)) {
    EggRenderMode::AlphaMode tex_trans = EggRenderMode::AM_unspecified;
    switch (tra->get_mode()) {
    case TransparencyAttrib::M_none:
      tex_trans = EggRenderMode::AM_off;
      break;
    case TransparencyAttrib::M_alpha:
      tex_trans = EggRenderMode::AM_blend;
      break;
    case TransparencyAttrib::M_premultiplied_alpha:
      tex_trans = EggRenderMode::AM_premultiplied;
      break;
    case TransparencyAttrib::M_multisample:
      tex_trans = EggRenderMode::AM_ms;
      break;
    case TransparencyAttrib::M_multisample_mask:
      tex_trans = EggRenderMode::AM_ms_mask;
      break;
    case TransparencyAttrib::M_binary:
      tex_trans = EggRenderMode::AM_binary;
      break;
    case TransparencyAttrib::M_dual:
      tex_trans = EggRenderMode::AM_dual;
      break;
    default:  // intentional fall-through
      break;
    }
    egg_render_mode->set_alpha_mode(tex_trans);
  }

  const DepthWriteAttrib *dwa;
  if (state->get_attrib(dwa)) {
    if (dwa->get_mode() != DepthWriteAttrib::M_off) {
      egg_render_mode->set_depth_write_mode(EggRenderMode::DWM_on);

    } else if (egg_render_mode->get_alpha_mode() == EggRenderMode::AM_blend) {
      // AM_blend_no_occlude is like AM_blend but also implies DWM_off.
      egg_render_mode->set_alpha_mode(EggRenderMode::AM_blend_no_occlude);

    } else {
      egg_render_mode->set_depth_write_mode(EggRenderMode::DWM_off);
    }
    any_applied = true;
  }

  const DepthTestAttrib *dta;
  if (state->get_attrib(dta)) {
    RenderAttrib::PandaCompareFunc mode = dta->get_mode();
    if (mode == DepthTestAttrib::M_none || mode == DepthTestAttrib::M_always) {
      egg_render_mode->set_depth_test_mode(EggRenderMode::DTM_off);
    } else {
      egg_render_mode->set_depth_test_mode(EggRenderMode::DTM_on);
    }
    any_applied = true;
  }

  const DepthOffsetAttrib *doa;
  if (state->get_attrib(doa)) {
    egg_render_mode->set_depth_offset(doa->get_offset());
    any_applied = true;
  }

  const CullBinAttrib *cba;
  if (state->get_attrib(cba)) {
    egg_render_mode->set_bin(cba->get_bin_name());
    egg_render_mode->set_draw_order(cba->get_draw_order());
    any_applied = true;
  }

  return any_applied;
}

/**
 * Applies string tags to the egg file.  Returns true if any were applied,
 * false otherwise.
 */
bool EggSaver::
apply_tags(EggGroup *egg_group, PandaNode *node) {
  std::ostringstream strm;
  char delimiter = '\n';
  string delimiter_str(1, delimiter);
  node->list_tags(strm, delimiter_str);

  string data = strm.str();
  if (data.empty()) {
    return false;
  }

  bool any_applied = false;

  size_t p = 0;
  size_t q = data.find(delimiter);
  while (q != string::npos) {
    string tag = data.substr(p, q);
    if (apply_tag(egg_group, node, tag)) {
      any_applied = true;
    }
    p = q + 1;
    q = data.find(delimiter, p);
  }

  string tag = data.substr(p);
  if (apply_tag(egg_group, node, tag)) {
    any_applied = true;
  }

  return any_applied;
}

/**
 * Applies the named string tags to the egg file.
 */
bool EggSaver::
apply_tag(EggGroup *egg_group, PandaNode *node, const string &tag) {
  if (!node->has_tag(tag)) {
    return false;
  }

  string value = node->get_tag(tag);
  egg_group->set_tag(tag, value);
  return true;
}

/**
 * Returns an EggMaterial pointer that corresponds to the indicated Material.
 */
EggMaterial *EggSaver::
get_egg_material(Material *mat) {
  if (mat != nullptr) {
    EggMaterial temp(mat->get_name());
    if (mat->has_base_color()) {
      temp.set_base(mat->get_base_color());
    }

    if (mat->has_ambient()) {
      temp.set_amb(mat->get_ambient());
    }

    if (mat->has_diffuse()) {
      temp.set_diff(mat->get_diffuse());
    }

    if (mat->has_specular()) {
      temp.set_spec(mat->get_specular());
    }

    if (mat->has_emission()) {
      temp.set_emit(mat->get_emission());
    }

    if (mat->has_roughness()) {
      temp.set_roughness(mat->get_roughness());
    } else {
      temp.set_shininess(mat->get_shininess());
    }

    if (mat->has_metallic()) {
      temp.set_metallic(mat->get_metallic());
    }

    if (mat->has_refractive_index()) {
      temp.set_ior(mat->get_refractive_index());
    }

    temp.set_local(mat->get_local());

    return _materials.create_unique_material(temp, ~EggMaterial::E_mref_name);
  }

  return nullptr;
}

/**
 * Returns an EggTexture pointer that corresponds to the indicated Texture.
 */
EggTexture *EggSaver::
get_egg_texture(Texture *tex) {
  if (tex != nullptr) {
    if (tex->has_filename()) {
      Filename filename = tex->get_filename();
      EggTexture temp(filename.get_basename_wo_extension(), filename);
      if (tex->has_alpha_filename()) {
        Filename alpha = tex->get_alpha_filename();
        temp.set_alpha_filename(alpha);
      }

      switch (tex->get_minfilter()) {
      case SamplerState::FT_nearest:
        temp.set_minfilter(EggTexture::FT_nearest);
        break;
      case SamplerState::FT_linear:
        temp.set_minfilter(EggTexture::FT_linear);
        break;
      case SamplerState::FT_nearest_mipmap_nearest:
        temp.set_minfilter(EggTexture::FT_nearest_mipmap_nearest);
        break;
      case SamplerState::FT_linear_mipmap_nearest:
        temp.set_minfilter(EggTexture::FT_linear_mipmap_nearest);
        break;
      case SamplerState::FT_nearest_mipmap_linear:
        temp.set_minfilter(EggTexture::FT_nearest_mipmap_linear);
        break;
      case SamplerState::FT_linear_mipmap_linear:
        temp.set_minfilter(EggTexture::FT_linear_mipmap_linear);
        break;

      default:
        break;
      }

      switch (tex->get_magfilter()) {
      case SamplerState::FT_nearest:
        temp.set_magfilter(EggTexture::FT_nearest);
        break;
      case SamplerState::FT_linear:
        temp.set_magfilter(EggTexture::FT_linear);
        break;

      default:
        break;
      }

      switch (tex->get_wrap_u()) {
      case SamplerState::WM_clamp:
        temp.set_wrap_u(EggTexture::WM_clamp);
        break;
      case SamplerState::WM_repeat:
        temp.set_wrap_u(EggTexture::WM_repeat);
        break;

      default:
        // There are some new wrap options on Texture that aren't yet
        // supported in egg.
        break;
      }

      switch (tex->get_wrap_v()) {
      case SamplerState::WM_clamp:
        temp.set_wrap_v(EggTexture::WM_clamp);
        break;
      case SamplerState::WM_repeat:
        temp.set_wrap_v(EggTexture::WM_repeat);
        break;

      default:
        // There are some new wrap options on Texture that aren't yet
        // supported in egg.
        break;
      }

      switch (tex->get_format()) {
      case Texture::F_red:
        temp.set_format(EggTexture::F_red);
        break;
      case Texture::F_green:
        temp.set_format(EggTexture::F_green);
        break;
      case Texture::F_blue:
        temp.set_format(EggTexture::F_blue);
        break;
      case Texture::F_alpha:
        temp.set_format(EggTexture::F_alpha);
        break;
      case Texture::F_rgb:
        temp.set_format(EggTexture::F_rgb);
        break;
      case Texture::F_rgb5:
        temp.set_format(EggTexture::F_rgb5);
        break;
      case Texture::F_rgb8:
        temp.set_format(EggTexture::F_rgb8);
        break;
      case Texture::F_rgb12:
        temp.set_format(EggTexture::F_rgb12);
        break;
      case Texture::F_rgb332:
        temp.set_format(EggTexture::F_rgb332);
        break;
      case Texture::F_rgba:
        temp.set_format(EggTexture::F_rgba);
        break;
      case Texture::F_rgbm:
        temp.set_format(EggTexture::F_rgbm);
        break;
      case Texture::F_rgba4:
        temp.set_format(EggTexture::F_rgba4);
        break;
      case Texture::F_rgba5:
        temp.set_format(EggTexture::F_rgba5);
        break;
      case Texture::F_rgba8:
        temp.set_format(EggTexture::F_rgba8);
        break;
      case Texture::F_rgba12:
        temp.set_format(EggTexture::F_rgba12);
        break;
      case Texture::F_luminance:
        temp.set_format(EggTexture::F_luminance);
        break;
      case Texture::F_luminance_alpha:
        temp.set_format(EggTexture::F_luminance_alpha);
        break;
      case Texture::F_luminance_alphamask:
        temp.set_format(EggTexture::F_luminance_alphamask);
        break;
      default:
        break;
      }

      return _textures.create_unique_texture(temp, ~EggTexture::E_tref_name);
    }
  }

  return nullptr;
}
