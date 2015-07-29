// Filename: eggSaver.cxx
// Created by:  drose (19Dec12)
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
#include "cullFaceAttrib.h"
#include "transparencyAttrib.h"
#include "depthWriteAttrib.h"
#include "lodNode.h"
#include "switchNode.h"
#include "sequenceNode.h"
#include "collisionNode.h"
#include "collisionPolygon.h"
#include "collisionPlane.h"
#include "collisionSphere.h"
#include "collisionBox.h"
#include "collisionInvSphere.h"
#include "collisionTube.h"
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

////////////////////////////////////////////////////////////////////
//     Function: EggSaver::Constructor
//       Access: Published
//  Description:
////////////////////////////////////////////////////////////////////
EggSaver::
EggSaver(EggData *data) :
  _data(data)
{
  if (_data == NULL) {
    _data = new EggData;
  }
}

////////////////////////////////////////////////////////////////////
//     Function: EggSaver::add_node
//       Access: Published
//  Description: Adds the scene graph rooted at the indicated node to
//               the accumulated egg data within this object.  Call
//               get_egg_data() to retrieve the result.
////////////////////////////////////////////////////////////////////
void EggSaver::
add_node(PandaNode *node) {
  _vpool = new EggVertexPool(node->get_name());
  _data->add_child(_vpool);

  NodePath root(node);
  convert_node(WorkingNodePath(root), _data, false);

  // Remove the vertex pool if it has no vertices.
  if (_vpool->empty()) {
    _data->remove_child(_vpool);
  }
  _vpool = NULL;
}

////////////////////////////////////////////////////////////////////
//     Function: EggSaver::convert_node
//       Access: Private
//  Description: Converts the indicated node to the corresponding Egg
//               constructs, by first determining what kind of node it
//               is.
////////////////////////////////////////////////////////////////////
void EggSaver::
convert_node(const WorkingNodePath &node_path, EggGroupNode *egg_parent,
             bool has_decal) {
  PandaNode *node = node_path.node();
  if (node->is_geom_node()) {
    convert_geom_node(DCAST(GeomNode, node), node_path, egg_parent, has_decal);

  } else if (node->is_of_type(LODNode::get_class_type())) {
    convert_lod_node(DCAST(LODNode, node), node_path, egg_parent, has_decal);

  } else if (node->is_of_type(SequenceNode::get_class_type())) {
    convert_sequence_node(DCAST(SequenceNode, node), node_path, egg_parent, has_decal);

  } else if (node->is_of_type(SwitchNode::get_class_type())) {
    convert_switch_node(DCAST(SwitchNode, node), node_path, egg_parent, has_decal);

  } else if (node->is_of_type(CollisionNode::get_class_type())) {
    convert_collision_node(DCAST(CollisionNode, node), node_path, egg_parent, has_decal);

  } else if (node->is_of_type(AnimBundleNode::get_class_type())) {
    convert_anim_node(DCAST(AnimBundleNode, node), node_path, egg_parent, has_decal);

  } else if (node->is_of_type(Character::get_class_type())) {
    convert_character_node(DCAST(Character, node), node_path, egg_parent, has_decal);

  } else {
    // Just a generic node.
    EggGroup *egg_group = new EggGroup(node->get_name());
    egg_parent->add_child(egg_group);
    apply_node_properties(egg_group, node);
    
    recurse_nodes(node_path, egg_group, has_decal);
  }
}

////////////////////////////////////////////////////////////////////
//     Function: EggSaver::convert_lod_node
//       Access: Private
//  Description: Converts the indicated LODNode to the corresponding
//               Egg constructs.
////////////////////////////////////////////////////////////////////
void EggSaver::
convert_lod_node(LODNode *node, const WorkingNodePath &node_path,
                 EggGroupNode *egg_parent, bool has_decal) {
  // An LOD node gets converted to an ordinary EggGroup, but we apply
  // the appropriate switch conditions to each of our children.
  EggGroup *egg_group = new EggGroup(node->get_name());
  egg_parent->add_child(egg_group);
  apply_node_properties(egg_group, node);

  int num_children = node->get_num_children();
  int num_switches = node->get_num_switches();

  num_children = min(num_children, num_switches);

  for (int i = 0; i < num_children; i++) {
    PandaNode *child = node->get_child(i);

    // Convert just this one node to an EggGroup.
    PT(EggGroup) next_group = new EggGroup;
    convert_node(WorkingNodePath(node_path, child), next_group, has_decal);

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

////////////////////////////////////////////////////////////////////
//     Function: EggSaver::convert_sequence_node
//       Access: Private
//  Description: Converts the indicated SequenceNode to the corresponding
//               Egg constructs.
////////////////////////////////////////////////////////////////////
void EggSaver::
convert_sequence_node(SequenceNode *node, const WorkingNodePath &node_path,
                      EggGroupNode *egg_parent, bool has_decal) {
  // A sequence node gets converted to an ordinary EggGroup, we only apply
  // the appropriate switch attributes to turn it into a sequence
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
    convert_node(WorkingNodePath(node_path, child), next_group, has_decal);

    egg_group->add_child(next_group.p());
  }

}

////////////////////////////////////////////////////////////////////
//     Function: EggSaver::convert_switch_node
//       Access: Private
//  Description: Converts the indicated SwitchNode to the corresponding
//               Egg constructs.
////////////////////////////////////////////////////////////////////
void EggSaver::
convert_switch_node(SwitchNode *node, const WorkingNodePath &node_path,
                    EggGroupNode *egg_parent, bool has_decal) {
  // A sequence node gets converted to an ordinary EggGroup, we only apply
  // the appropriate switch attributes to turn it into a sequence
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
    convert_node(WorkingNodePath(node_path, child), next_group, has_decal);

    egg_group->add_child(next_group.p());
  }
}

////////////////////////////////////////////////////////////////////
//     Function: EggSaver::convert_animGroup_node
//       Access: Private
//  Description: Converts the indicated AnimationGroupNodes to the corresponding
//               Egg constructs.
////////////////////////////////////////////////////////////////////
EggGroupNode * EggSaver::convert_animGroup_node(AnimGroup *animGroup, double fps ) {
  int num_children = animGroup->get_num_children();

  EggGroupNode *eggNode = NULL;
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
    if (eggChildNode!=NULL) {
      nassertr(eggNode!=NULL, NULL);
      eggNode->add_child(eggChildNode);
    }
  } 
  return eggNode;
}

////////////////////////////////////////////////////////////////////
//     Function: EggSaver::convert_anim_node
//       Access: Private
//  Description: Converts the indicated AnimNode to the corresponding
//               Egg constructs.
////////////////////////////////////////////////////////////////////
void EggSaver::
convert_anim_node(AnimBundleNode *node, const WorkingNodePath &node_path,
                    EggGroupNode *egg_parent, bool has_decal) {
  
  // A sequence node gets converted to an ordinary EggGroup, we only apply
  // the appropriate switch attributes to turn it into a sequence
  EggTable *eggTable = new EggTable();
  //egg_parent->add_child(eggTable);
  _data->add_child(eggTable);
 
  AnimBundle *animBundle = node->get_bundle();
  // turn it into a switch..
  //egg_group->set_switch_flag(true);

  EggGroupNode *eggAnimation = convert_animGroup_node(animBundle, animBundle->get_base_frame_rate());
  eggTable->add_child(eggAnimation);
}

////////////////////////////////////////////////////////////////////
//     Function: EggSaver::convert_character_bundle
//       Access: Private
//  Description: Converts the indicated Character Bundle to the corresponding
//               Egg joints structure.
////////////////////////////////////////////////////////////////////
void EggSaver::
convert_character_bundle(PartGroup *bundleNode, EggGroupNode *egg_parent, CharacterJointMap *jointMap) {
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
    if (jointMap!=NULL) {
      CharacterJointMap::iterator mi = jointMap->find(character_joint);
      if (mi != jointMap->end()) {
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
    convert_character_bundle(partGroup, joint_group, jointMap);
  }

}

////////////////////////////////////////////////////////////////////
//     Function: EggSaver::convert_character_node
//       Access: Private
//  Description: Converts the indicated Character to the corresponding
//               Egg constructs.
////////////////////////////////////////////////////////////////////
void EggSaver::
convert_character_node(Character *node, const WorkingNodePath &node_path,
                    EggGroupNode *egg_parent, bool has_decal) {
  
  // A sequence node gets converted to an ordinary EggGroup, we only apply
  // the appropriate switch attributes to turn it into a sequence
  EggGroup *egg_group = new EggGroup(node->get_name());
  egg_group->set_dart_type(EggGroup::DT_default);
  egg_parent->add_child(egg_group);
  apply_node_properties(egg_group, node);

  CharacterJointMap jointMap;
  
  // turn it into a switch..
  //egg_group->set_switch_flag(true);

  int num_children = node->get_num_children();
  int num_bundles = node->get_num_bundles();

  for (int i = 0; i < num_children; i++) {
    PandaNode *child = node->get_child(i);

    if (child->is_geom_node()) {
      convert_geom_node(DCAST(GeomNode, child), WorkingNodePath(node_path, child), egg_group, has_decal, &jointMap);
    }
  }

  for (int i = 0; i < num_bundles ; i++) {
    PartBundle *bundle= node->get_bundle(i);
    convert_character_bundle(bundle, egg_group, &jointMap);
  }

}


////////////////////////////////////////////////////////////////////
//     Function: EggSaver::convert_collision_node
//       Access: Private
//  Description: Converts the indicated CollisionNode to the corresponding
//               Egg constructs.
////////////////////////////////////////////////////////////////////
void EggSaver::
convert_collision_node(CollisionNode *node, const WorkingNodePath &node_path,
                       EggGroupNode *egg_parent, bool has_decal) {
  // A sequence node gets converted to an ordinary EggGroup, we only apply
  // the appropriate switch attributes to turn it into a sequence
  EggGroup *egg_group = new EggGroup(node->get_name());
  egg_parent->add_child(egg_group);
  apply_node_properties(egg_group, node, false);

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
      if (child->is_of_type(CollisionPolygon::get_class_type())) {
        egg_group->set_cs_type(EggGroup::CST_polyset);

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
        LVector3 offset(sphere->get_radius(), 0, 0);

        EggGroup *egg_sphere;
        if (num_solids == 1) {
          egg_sphere = egg_group;
        } else {
          egg_sphere = new EggGroup;
          egg_sphere->set_collide_flags(EggGroup::CF_descend);
          egg_group->add_child(egg_sphere);
        }
        egg_sphere->set_cs_type(EggGroup::CST_sphere);

        EggVertex ev1, ev2;
        ev1.set_pos(LCAST(double, (center + offset) * net_mat));
        ev2.set_pos(LCAST(double, (center - offset) * net_mat));

        EggPolygon *egg_poly = new EggPolygon;
        egg_sphere->add_child(egg_poly);

        egg_poly->add_vertex(cvpool->create_unique_vertex(ev1));
        egg_poly->add_vertex(cvpool->create_unique_vertex(ev2));

      } else if (child->is_of_type(CollisionPlane::get_class_type())) {
        LPlane plane = DCAST(CollisionPlane, child)->get_plane();
        LPoint3 origin = plane.get_point();
        LVector3 normal = plane.get_normal();

        // Get an arbitrary vector on the plane by taking the cross product
        // with any vector, as long as it is different.
        LVector3 vec1;
        if (abs(normal[2]) > abs(normal[1])) {
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
          egg_plane->set_collide_flags(EggGroup::CF_descend);
          egg_group->add_child(egg_plane);
        }
        egg_plane->set_cs_type(EggGroup::CST_plane);

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
        nout << "Encountered unhandled collsion type: CollisionBox" << "\n";
      } else if (child->is_of_type(CollisionInvSphere::get_class_type())) {
        nout << "Encountered unhandled collsion type: CollisionInvSphere" << "\n";
      } else if (child->is_of_type(CollisionTube::get_class_type())) {
        nout << "Encountered unhandled collsion type: CollisionTube" << "\n";
      } else {
        nout << "Encountered unknown CollisionSolid" << "\n";
      }
    }
  }

  // recurse over children - hm. do I need to do this?
  recurse_nodes(node_path, egg_group, has_decal);
}

////////////////////////////////////////////////////////////////////
//     Function: EggSaver::convert_geom_node
//       Access: Private
//  Description: Converts a GeomNode to the corresponding egg
//               structures.
////////////////////////////////////////////////////////////////////
void EggSaver::
convert_geom_node(GeomNode *node, const WorkingNodePath &node_path, 
                  EggGroupNode *egg_parent, bool has_decal, CharacterJointMap *jointMap) {
  PT(EggGroup) egg_group = new EggGroup(node->get_name());
  bool fancy_attributes = apply_node_properties(egg_group, node);

  if (node->get_effects()->has_decal()) {
    has_decal = true;
  }

  if (has_decal) {
    egg_group->set_decal_flag(true);
  }

  if (fancy_attributes || has_decal || !node->get_name().empty()) {
    // If we have any fancy attributes on the node, or if we're making
    // decal geometry, we have to make a special node to hold the
    // geometry (normally it would just appear within its parent).
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
    CPT(RenderState) geom_state = net_state->compose(node->get_geom_state(i));

    const Geom *geom = node->get_geom(i);
    int num_primitives = geom->get_num_primitives();
    for (int j = 0; j < num_primitives; ++j) {
      const GeomPrimitive *primitive = geom->get_primitive(j);
      CPT(GeomPrimitive) simple = primitive->decompose();
      CPT(GeomVertexData) vdata = geom->get_vertex_data();
      //        vdata = vdata->animate_vertices(true, Thread::get_current_thread());
      convert_primitive(vdata, simple, geom_state,
                        net_mat, egg_parent, jointMap);
    }
  }
  
  recurse_nodes(node_path, egg_parent, has_decal);
}

////////////////////////////////////////////////////////////////////
//     Function: EggSaver::convert_primitive
//       Access: Private
//  Description: 
////////////////////////////////////////////////////////////////////
void EggSaver::
convert_primitive(const GeomVertexData *vertex_data,
                  const GeomPrimitive *primitive,
                  const RenderState *net_state, 
                  const LMatrix4 &net_mat, EggGroupNode *egg_parent,
                  CharacterJointMap *jointMap) {
  GeomVertexReader reader(vertex_data);

  // Check for a color scale.
  LVecBase4 color_scale(1.0f, 1.0f, 1.0f, 1.0f);
  const ColorScaleAttrib *csa = DCAST(ColorScaleAttrib, net_state->get_attrib(ColorScaleAttrib::get_class_type()));
  if (csa != (const ColorScaleAttrib *)NULL) {
    color_scale = csa->get_scale();
  }

  // Check for a color override.
  bool has_color_override = false;
  bool has_color_off = false;
  LColor color_override;
  const ColorAttrib *ca = DCAST(ColorAttrib, net_state->get_attrib(ColorAttrib::get_class_type()));
  if (ca != (const ColorAttrib *)NULL) {
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
  EggMaterial *egg_mat = (EggMaterial *)NULL;
  const MaterialAttrib *ma = DCAST(MaterialAttrib, net_state->get_attrib(MaterialAttrib::get_class_type()));
  if (ma != (const MaterialAttrib *)NULL) {
    egg_mat = get_egg_material(ma->get_material());
  }

  // Check for a texture.
  EggTexture *egg_tex = (EggTexture *)NULL;
  const TextureAttrib *ta = DCAST(TextureAttrib, net_state->get_attrib(TextureAttrib::get_class_type()));
  if (ta != (const TextureAttrib *)NULL) {
    egg_tex = get_egg_texture(ta->get_texture());
  }

  // Check the texture environment
  if ((ta != (const TextureAttrib *)NULL) && (egg_tex != (const EggTexture *)NULL)) {
    TextureStage* tex_stage = ta->get_on_stage(0);
    if (tex_stage != (const TextureStage *)NULL) {
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
  }

  // Check the backface flag.
  bool bface = false;
  const RenderAttrib *cf_attrib = net_state->get_attrib(CullFaceAttrib::get_class_type());
  if (cf_attrib != (const RenderAttrib *)NULL) {
    const CullFaceAttrib *cfa = DCAST(CullFaceAttrib, cf_attrib);
    if (cfa->get_effective_mode() == CullFaceAttrib::M_cull_none) {
      bface = true;
    }
  }

  // Check the depth write flag - only needed for AM_blend_no_occlude
  bool has_depthwrite = false;
  DepthWriteAttrib::Mode depthwrite = DepthWriteAttrib::M_on;
  const RenderAttrib *dw_attrib = net_state->get_attrib(DepthWriteAttrib::get_class_type());
  if (dw_attrib != (const RenderAttrib *)NULL) {
    const DepthWriteAttrib *dwa = DCAST(DepthWriteAttrib, dw_attrib);
    depthwrite = dwa->get_mode();
    has_depthwrite = true;
  }

  // Check the transparency flag.
  bool has_transparency = false;
  TransparencyAttrib::Mode transparency = TransparencyAttrib::M_none;
  const RenderAttrib *tr_attrib = net_state->get_attrib(TransparencyAttrib::get_class_type());
  if (tr_attrib != (const RenderAttrib *)NULL) {
    const TransparencyAttrib *tra = DCAST(TransparencyAttrib, tr_attrib);
    transparency = tra->get_mode();
    has_transparency = true;
  }
  if (has_transparency && (egg_tex != (EggTexture *)NULL)) {
    EggRenderMode::AlphaMode tex_trans = EggRenderMode::AM_unspecified;
    switch (transparency) {
      case TransparencyAttrib::M_none:
        tex_trans = EggRenderMode::AM_off;
        break;
      case TransparencyAttrib::M_alpha:
        if (has_depthwrite && (depthwrite == DepthWriteAttrib::M_off)) {
            tex_trans = EggRenderMode::AM_blend_no_occlude;
                has_depthwrite = false;
        } else {
          tex_trans = EggRenderMode::AM_blend;
        }
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
      case TransparencyAttrib::M_notused:
        break;
    }
    if (tex_trans != EggRenderMode::AM_unspecified) {
      egg_tex->set_alpha_mode(tex_trans);
    }
  }


  LNormal normal;
  LColor color;
  CPT(TransformBlendTable) transformBlendTable = vertex_data->get_transform_blend_table();

  int num_primitives = primitive->get_num_primitives();
  int num_vertices = primitive->get_num_vertices_per_primitive();

  EggPrimitive *(*make_func)(void);

  if (primitive->is_of_type(GeomTriangles::get_class_type())) {
    make_func = make_egg_polygon;
  } else if (primitive->is_of_type(GeomPatches::get_class_type())) {
    make_func = make_egg_patch;
  } else if (primitive->is_of_type(GeomPoints::get_class_type())) {
    make_func = make_egg_point;
  } else if (primitive->is_of_type(GeomLines::get_class_type())) {
    make_func = make_egg_line;
  } else {
    // Huh, an unknown geometry type.
    return;
  }

  for (int i = 0; i < num_primitives; ++i) {
    PT(EggPrimitive) egg_prim = (*make_func)();

    egg_parent->add_child(egg_prim);

    if (egg_mat != (EggMaterial *)NULL) {
      egg_prim->set_material(egg_mat);
    }
    if (egg_tex != (EggTexture *)NULL) {
      egg_prim->set_texture(egg_tex);
    }
    
    if (bface) {
      egg_prim->set_bface_flag(true);
    }

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
        
      if ((vertex_data->has_column(InternalName::get_transform_blend())) && 
          (jointMap!=NULL) && (transformBlendTable!=NULL)) {
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

              CharacterJointMap::iterator mi = jointMap->find(joint_vertex_transform->get_joint());
              if (mi == jointMap->end()) {
                mi = jointMap->insert(CharacterJointMap::value_type(joint_vertex_transform->get_joint(), pvector<pair<EggVertex*,PN_stdfloat> >())).first;
              }
              pvector<pair<EggVertex*,PN_stdfloat> > &joint_vertices = (*mi).second;
              joint_vertices.push_back(pair<EggVertex*,PN_stdfloat>(new_egg_vert, weight));
            }
          }
        }
      }

      egg_prim->add_vertex(new_egg_vert);
    }
  }
}

////////////////////////////////////////////////////////////////////
//     Function: EggSaver::recurse_nodes
//       Access: Private
//  Description: Converts all the children of the indicated node.
////////////////////////////////////////////////////////////////////
void EggSaver::
recurse_nodes(const WorkingNodePath &node_path, EggGroupNode *egg_parent,
              bool has_decal) {
  PandaNode *node = node_path.node();
  int num_children = node->get_num_children();
  
  for (int i = 0; i < num_children; i++) {
    PandaNode *child = node->get_child(i);
    convert_node(WorkingNodePath(node_path, child), egg_parent, has_decal);
  }
}

////////////////////////////////////////////////////////////////////
//     Function: EggSaver::apply_node_properties
//       Access: Private
//  Description: Applies any special properties that might be stored
//               on the node, like billboarding.  Returns true if any
//               were applied, false otherwise.
////////////////////////////////////////////////////////////////////
bool EggSaver::
apply_node_properties(EggGroup *egg_group, PandaNode *node, bool allow_backstage) {
  bool any_applied = false;

  if (node->is_overall_hidden() && allow_backstage) {
    // This node is hidden.  We'll go ahead and convert it, but we'll
    // put in the "backstage" flag to mean it's not real geometry.
    // unless the caller wants to keep it (by setting allow_backstage to false)
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

  const RenderEffects *effects = node->get_effects();
  const RenderEffect *effect = effects->get_effect(BillboardEffect::get_class_type());
  if (effect != (RenderEffect *)NULL) {
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
      // If the transform can be represented componentwise, we prefer
      // storing it that way in the egg file.
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

  return any_applied;
}

////////////////////////////////////////////////////////////////////
//     Function: EggSaver::apply_tags
//       Access: Private
//  Description: Applies string tags to the egg file.  Returns true if
//               any were applied, false otherwise.
////////////////////////////////////////////////////////////////////
bool EggSaver::
apply_tags(EggGroup *egg_group, PandaNode *node) {
  ostringstream strm;
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

////////////////////////////////////////////////////////////////////
//     Function: EggSaver::apply_tag
//       Access: Private
//  Description: Applies the named string tags to the egg file.
////////////////////////////////////////////////////////////////////
bool EggSaver::
apply_tag(EggGroup *egg_group, PandaNode *node, const string &tag) {
  if (!node->has_tag(tag)) {
    return false;
  }

  string value = node->get_tag(tag);
  egg_group->set_tag(tag, value);
  return true;
}

////////////////////////////////////////////////////////////////////
//     Function: EggSaver::get_egg_material
//       Access: Private
//  Description: Returns an EggMaterial pointer that corresponds to
//               the indicated Material.
////////////////////////////////////////////////////////////////////
EggMaterial *EggSaver::
get_egg_material(Material *mat) {
  if (mat != (Material *)NULL) {
    EggMaterial temp(mat->get_name());
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

    temp.set_shininess(mat->get_shininess());
    temp.set_local(mat->get_local());

    return _materials.create_unique_material(temp, ~EggMaterial::E_mref_name);
  }

  return NULL;
}

////////////////////////////////////////////////////////////////////
//     Function: EggSaver::get_egg_texture
//       Access: Private
//  Description: Returns an EggTexture pointer that corresponds to the
//               indicated Texture.
////////////////////////////////////////////////////////////////////
EggTexture *EggSaver::
get_egg_texture(Texture *tex) {
  if (tex != (Texture *)NULL) {
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

  return NULL;
}

////////////////////////////////////////////////////////////////////
//     Function: EggSaver::make_egg_polygon
//       Access: Private, Static
//  Description: A factory function to make a new EggPolygon instance.
////////////////////////////////////////////////////////////////////
EggPrimitive *EggSaver::
make_egg_polygon() {
  return new EggPolygon;
}

////////////////////////////////////////////////////////////////////
//     Function: EggSaver::make_egg_patch
//       Access: Private, Static
//  Description: A factory function to make a new EggPatch instance.
////////////////////////////////////////////////////////////////////
EggPrimitive *EggSaver::
make_egg_patch() {
  return new EggPatch;
}

////////////////////////////////////////////////////////////////////
//     Function: EggSaver::make_egg_point
//       Access: Private, Static
//  Description: A factory function to make a new EggPoint instance.
////////////////////////////////////////////////////////////////////
EggPrimitive *EggSaver::
make_egg_point() {
  return new EggPoint;
}

////////////////////////////////////////////////////////////////////
//     Function: EggSaver::make_egg_line
//       Access: Private, Static
//  Description: A factory function to make a new EggLine instance.
////////////////////////////////////////////////////////////////////
EggPrimitive *EggSaver::
make_egg_line() {
  return new EggLine;
}
