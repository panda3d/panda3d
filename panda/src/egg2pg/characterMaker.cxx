// Filename: characterMaker.cxx
// Created by:  drose (06Mar02)
//
////////////////////////////////////////////////////////////////////
//
// PANDA 3D SOFTWARE
// Copyright (c) 2001 - 2004, Disney Enterprises, Inc.  All rights reserved
//
// All use of this software is subject to the terms of the Panda 3d
// Software license.  You should have received a copy of this license
// along with this source code; you will also find a current copy of
// the license at http://etc.cmu.edu/panda3d/docs/license/ .
//
// To contact the maintainers of this program write to
// panda3d-general@lists.sourceforge.net .
//
////////////////////////////////////////////////////////////////////

#include "characterMaker.h"
#include "eggLoader.h"
#include "config_egg2pg.h"
#include "eggBinner.h"
#include "computedVertices.h"
#include "eggGroup.h"
#include "eggPrimitive.h"
#include "eggBin.h"
#include "partGroup.h"
#include "characterJoint.h"
#include "characterJointBundle.h"
#include "characterSlider.h"
#include "character.h"
#include "geomNode.h"
#include "transformState.h"
#include "eggSurface.h"
#include "eggCurve.h"
#include "modelNode.h"
#include "characterVertexSlider.h"
#include "jointVertexTransform.h"
#include "userVertexTransform.h"

////////////////////////////////////////////////////////////////////
//     Function: CharacterMaker::Construtor
//       Access: Public
//  Description:
////////////////////////////////////////////////////////////////////
CharacterMaker::
CharacterMaker(EggGroup *root, EggLoader &loader)
  : _loader(loader), _egg_root(root) {

  _character_node = new Character(_egg_root->get_name());
  _bundle = _character_node->get_bundle();

  _morph_root = (PartGroup *)NULL;
  _skeleton_root = new PartGroup(_bundle, "<skeleton>");
}

////////////////////////////////////////////////////////////////////
//     Function: CharacterMaker::make_node
//       Access: Public
//  Description:
////////////////////////////////////////////////////////////////////
Character *CharacterMaker::
make_node() {
  make_bundle();
  _character_node->_parts = _parts;
  return _character_node;
}

////////////////////////////////////////////////////////////////////
//     Function: CharacterMaker::get_name
//       Access: Public
//  Description: Returns the name of the character.
////////////////////////////////////////////////////////////////////
string CharacterMaker::
get_name() const {
  return _egg_root->get_name();
}

////////////////////////////////////////////////////////////////////
//     Function: CharacterMaker::egg_to_part
//       Access: Public
//  Description: Returns the PartGroup node associated with the given
//               egg node.  If the egg node is not a node in the
//               character's hierarchy, returns the top of the
//               character's hierarchy.
////////////////////////////////////////////////////////////////////
PartGroup *CharacterMaker::
egg_to_part(EggNode *egg_node) const {
  int index = egg_to_index(egg_node);
  if (index < 0) {
    // If there's a reference to the geometry outside of the
    // character, just return the root of the character.
    return _bundle;
  }
  nassertr(index < (int)_parts.size(), NULL);
  return _parts[index];
}

////////////////////////////////////////////////////////////////////
//     Function: CharacterMaker::egg_to_transform
//       Access: Public
//  Description: Returns a JointVertexTransform suitable for
//               applying the animation associated with the given
//               egg node (which should be a joint).  Returns an
//               identity transform if the egg node is not a joint in
//               the character's hierarchy.
////////////////////////////////////////////////////////////////////
VertexTransform *CharacterMaker::
egg_to_transform(EggNode *egg_node) {
  int index = egg_to_index(egg_node);
  if (index < 0) {
    // Not a joint in the hierarchy.
    return get_identity_transform();
  }

  VertexTransforms::iterator vi = _vertex_transforms.find(index);
  if (vi != _vertex_transforms.end()) {
    return (*vi).second;
  }

  PartGroup *part = _parts[index];
  CharacterJoint *joint;
  DCAST_INTO_R(joint, part, get_identity_transform());

  PT(VertexTransform) vt = new JointVertexTransform(joint);
  _vertex_transforms[index] = vt;

  return vt;
}

////////////////////////////////////////////////////////////////////
//     Function: CharacterMaker::egg_to_index
//       Access: Public
//  Description: Returns the index number associated with the
//               PartGroup node for the given egg node, or -1.
////////////////////////////////////////////////////////////////////
int CharacterMaker::
egg_to_index(EggNode *egg_node) const {
  NodeMap::const_iterator nmi = _node_map.find(egg_node);
  if (nmi == _node_map.end()) {
    return -1;
  }
  return (*nmi).second;
}

////////////////////////////////////////////////////////////////////
//     Function: CharacterMaker::part_to_node
//       Access: Public
//  Description: Returns the scene graph node associated with the
//               given PartGroup node, if there is one.  If the
//               PartGroup does not have an associated node, returns
//               the character's top node.
////////////////////////////////////////////////////////////////////
PandaNode *CharacterMaker::
part_to_node(PartGroup *part, const string &name) const {
  PandaNode *node = _character_node;

  if (part->is_of_type(CharacterJoint::get_class_type())) {
    CharacterJoint *joint = DCAST(CharacterJoint, part);
    if (joint->_geom_node != (PandaNode *)NULL) {
      node = joint->_geom_node;
    }
  }

  if (use_qpgeom) {
    // We should always return a GeomNode, so that all polysets
    // created at the same level will get added into the same
    // GeomNode.  Look for a child of this node.  If it doesn't have a
    // child yet, add a GeomNode and return it.  Otherwise, if it
    // already has a child, return that.
    if (node->is_geom_node() && node->get_name() == name) {
      return node;
    }
    for (int i = 0; i < node->get_num_children(); i++) {
      PandaNode *child = node->get_child(i);
      if (child->is_geom_node() && child->get_name() == name) {
        return child;
      }
    }
    PT(GeomNode) geom_node = new GeomNode(name);
    node->add_child(geom_node);
    return geom_node;

  } else {
    // In the original Geom implementation, a node is a node.
    return node;
  }
}


////////////////////////////////////////////////////////////////////
//     Function: CharacterMaker::create_slider
//       Access: Public
//  Description: Creates a new morph slider of the given name, and
//               returns its index.  This is actually called by
//               ComputedVerticesMaker, which is responsible for
//               identifying all the unique morph target names.
////////////////////////////////////////////////////////////////////
int CharacterMaker::
create_slider(const string &name) {
  if (_morph_root == (PartGroup *)NULL) {
    _morph_root = new PartGroup(_bundle, "morph");
  }
  CharacterSlider *slider = new CharacterSlider(_morph_root, name);
  int index = _parts.size();
  _parts.push_back(slider);
  return index;
}

////////////////////////////////////////////////////////////////////
//     Function: CharacterMaker::egg_to_slider
//       Access: Public
//  Description: Returns the VertexSlider corresponding to the
//               indicated egg slider name.
////////////////////////////////////////////////////////////////////
VertexSlider *CharacterMaker::
egg_to_slider(const string &name) {
  VertexSliders::iterator vi = _vertex_sliders.find(name);
  if (vi != _vertex_sliders.end()) {
    return (*vi).second;
  }

  int index = create_slider(name);
  PT(VertexSlider) slider = 
    new CharacterVertexSlider(DCAST(CharacterSlider, _parts[index]));
  _vertex_sliders[name] = slider;
  return slider;
}


////////////////////////////////////////////////////////////////////
//     Function: CharacterMaker::make_bundle
//       Access: Private
//  Description:
////////////////////////////////////////////////////////////////////
CharacterJointBundle *CharacterMaker::
make_bundle() {
  build_joint_hierarchy(_egg_root, _skeleton_root);

  if (use_qpgeom) {
    // The new, experimental Geom system.
    make_qpgeometry(_egg_root);

  } else {
    // The old Geom system.
    make_geometry(_egg_root);
    
    _character_node->_computed_vertices =
      _comp_verts_maker.make_computed_vertices(_character_node, *this);
  }
  _bundle->sort_descendants();
  parent_joint_nodes(_skeleton_root);

  return _bundle;
}

////////////////////////////////////////////////////////////////////
//     Function: CharacterMaker::build_hierarchy
//       Access: Private
//  Description:
////////////////////////////////////////////////////////////////////
void CharacterMaker::
build_joint_hierarchy(EggNode *egg_node, PartGroup *part) {
  int index = -1;

  if (egg_node->is_of_type(EggGroup::get_class_type())) {
    EggGroup *egg_group = DCAST(EggGroup, egg_node);

    // Each joint we come across is significant, and gets added to the
    // hierarchy.  Non-joints we encounter are ignored.
    if (egg_group->get_group_type() == EggGroup::GT_joint) {
      // We need to get the transform of the joint, and then convert
      // it to single-precision.
      LMatrix4d matd;
      if (egg_group->has_transform()) {
        matd = egg_group->get_transform();
      } else {
        matd = LMatrix4d::ident_mat();
      }

      LMatrix4f matf = LCAST(float, matd);

      CharacterJoint *joint =
        new CharacterJoint(part, egg_group->get_name(), matf);
      index = _parts.size();
      _parts.push_back(joint);

      if (egg_group->has_dcs_type()) {
        // If the joint requested an explicit DCS, create a node for
        // it.
        PT(ModelNode) geom_node = new ModelNode(egg_group->get_name());
        geom_node->set_preserve_transform(ModelNode::PT_local);
        joint->_geom_node = geom_node.p();
      }

      part = joint;
    }

    EggGroup::const_iterator ci;
    for (ci = egg_group->begin(); ci != egg_group->end(); ++ci) {
      build_joint_hierarchy((*ci), part);
    }
  }

  _node_map[egg_node] = index;
}

////////////////////////////////////////////////////////////////////
//     Function: CharacterMaker::parent_joint_nodes
//       Access: Private
//  Description: Walks the joint hierarchy, and parents any explicit
//               nodes created for the joints under the character
//               node.
////////////////////////////////////////////////////////////////////
void CharacterMaker::
parent_joint_nodes(PartGroup *part) {
  if (part->is_of_type(CharacterJoint::get_class_type())) {
    CharacterJoint *joint = DCAST(CharacterJoint, part);
    PandaNode *joint_node = joint->_geom_node;
    if (joint_node != NULL) {
      _character_node->add_child(joint_node);
      joint->add_net_transform(joint_node);
      joint_node->set_transform(TransformState::make_mat(joint->_net_transform));
    }
  }

  for (int i = 0; i < part->get_num_children(); i++) {
    parent_joint_nodes(part->get_child(i));
  }
}

////////////////////////////////////////////////////////////////////
//     Function: CharacterMaker::make_geometry
//       Access: Private
//  Description:
////////////////////////////////////////////////////////////////////
void CharacterMaker::
make_geometry(EggNode *egg_node) {
  if (egg_node->is_of_type(EggPrimitive::get_class_type())) {
    EggPrimitive *egg_primitive = DCAST(EggPrimitive, egg_node);
    if (!egg_primitive->empty()) {
      EggGroupNode *prim_home = determine_primitive_home(egg_primitive);

      if (prim_home == (EggGroupNode *)NULL &&
          (egg_primitive->is_of_type(EggSurface::get_class_type()) ||
           egg_primitive->is_of_type(EggCurve::get_class_type()))) {
        // If the primitive would be dynamic but is a parametric
        // primitive, we can't animate it anyway, so just put the
        // whole thing under the primitive's parent node.
        prim_home = egg_primitive->get_parent();
      }

      if (prim_home == (EggGroupNode *)NULL) {
        // This is a totally dynamic primitive that lives under the
        // character's node.
        make_dynamic_primitive(egg_primitive, _egg_root);

      } else {
        // This is a static primitive that lives under a particular
        // node.
        make_static_primitive(egg_primitive, prim_home);
      }
    }
  }

  if (egg_node->is_of_type(EggGroupNode::get_class_type())) {
    EggGroupNode *egg_group = DCAST(EggGroupNode, egg_node);

    EggGroupNode::const_iterator ci;
    for (ci = egg_group->begin(); ci != egg_group->end(); ++ci) {
      make_geometry(*ci);
    }
  }
}

////////////////////////////////////////////////////////////////////
//     Function: CharacterMaker::make_qpgeometry
//       Access: Private
//  Description: Walks the hierarchy, looking for bins that represent
//               polysets, which are to be animated with the
//               character.  Invokes the egg loader to create the
//               animated geometry.
//
//               This is part of the experimental Geom rewrite.
////////////////////////////////////////////////////////////////////
void CharacterMaker::
make_qpgeometry(EggNode *egg_node) {
  if (egg_node->is_of_type(EggBin::get_class_type())) {
    EggBin *egg_bin = DCAST(EggBin, egg_node);

    if (!egg_bin->empty() && 
        egg_bin->get_bin_number() == EggBinner::BN_polyset) {
      EggGroupNode *bin_home = determine_bin_home(egg_bin);

      bool is_dynamic;
      if (bin_home == (EggGroupNode *)NULL) {
        // This is a dynamic polyset that lives under the character's
        // root node.
        bin_home = _egg_root;
        is_dynamic = true;
      } else {
        // This is a totally static polyset that is parented under
        // some animated joint node.
        is_dynamic = false;
      }

      PandaNode *parent = part_to_node(egg_to_part(bin_home), egg_bin->get_name());
      LMatrix4d transform =
        egg_bin->get_vertex_frame() *
        bin_home->get_node_frame_inv();
      
      _loader.make_polyset(egg_bin, parent, &transform, is_dynamic,
                           this);
    }
  }

  if (egg_node->is_of_type(EggGroupNode::get_class_type())) {
    EggGroupNode *egg_group = DCAST(EggGroupNode, egg_node);

    EggGroupNode::const_iterator ci;
    for (ci = egg_group->begin(); ci != egg_group->end(); ++ci) {
      make_qpgeometry(*ci);
    }
  }
}

////////////////////////////////////////////////////////////////////
//     Function: CharacterMaker::make_static_primitive
//       Access: Private
//  Description:
////////////////////////////////////////////////////////////////////
void CharacterMaker::
make_static_primitive(EggPrimitive *egg_primitive, EggGroupNode *prim_home) {
  PandaNode *node = part_to_node(egg_to_part(prim_home), string());

  // We need this funny transform to convert from the coordinate
  // space of the original vertices to that of the new joint node.
  LMatrix4d transform =
    egg_primitive->get_vertex_frame() *
    prim_home->get_node_frame_inv();

  _loader.make_nonindexed_primitive(egg_primitive, node, &transform,
                                    _comp_verts_maker);
}

////////////////////////////////////////////////////////////////////
//     Function: CharacterMaker::make_dynamic_primitive
//       Access: Private
//  Description:
////////////////////////////////////////////////////////////////////
void CharacterMaker::
make_dynamic_primitive(EggPrimitive *egg_primitive, EggGroupNode *prim_home) {
  PandaNode *node = part_to_node(egg_to_part(prim_home), string());

  LMatrix4d transform =
    egg_primitive->get_vertex_frame() *
    prim_home->get_node_frame_inv();

  _loader.make_indexed_primitive(egg_primitive, node, &transform,
                                 _comp_verts_maker);
}

////////////////////////////////////////////////////////////////////
//     Function: CharacterMaker::determine_primitive_home
//       Access: Private
//  Description:
////////////////////////////////////////////////////////////////////
EggGroupNode *CharacterMaker::
determine_primitive_home(EggPrimitive *egg_primitive) {
  // A primitive's vertices may be referenced by any joint in the
  // character.  Or, the primitive itself may be explicitly placed
  // under a joint.

  // If any of the vertices are referenced by multiple joints, or if
  // any two vertices are referenced by different joints, then the
  // entire primitive must be considered dynamic.  (We'll indicate a
  // dynamic primitive by returning NULL.)

  // We need to keep track of the one joint we've encountered so far,
  // to see if all the vertices are referenced by the same joint.
  EggGroupNode *home = NULL;

  EggPrimitive::const_iterator vi;
  for (vi = egg_primitive->begin();
       vi != egg_primitive->end();
       ++vi) {
    EggVertex *vertex = (*vi);
    if (vertex->gref_size() > 1) {
      // This vertex is referenced by multiple joints; the primitive
      // is dynamic.
      return NULL;
    }

    if (!vertex->_dxyzs.empty() ||
        !vertex->_dnormals.empty() ||
        !vertex->_drgbas.empty()) {
      // This vertex has some morph slider definitions; therefore, the
      // primitive is dynamic.
      return NULL;
    }
    EggVertex::const_uv_iterator uvi;
    for (uvi = vertex->uv_begin(); uvi != vertex->uv_end(); ++uvi) {
      if (!(*uvi)->_duvs.empty()) {
        // Ditto: the vertex has some UV morphs; therefore the
        // primitive is dynamic.
        return NULL;
      }
    }

    EggGroupNode *vertex_home;

    if (vertex->gref_size() == 0) {
      // This vertex is not referenced at all, which means it belongs
      // right where it is.
      vertex_home = egg_primitive->get_parent();
    } else {
      nassertr(vertex->gref_size() == 1, NULL);
      // This vertex is referenced exactly once.
      vertex_home = *vertex->gref_begin();
    }

    if (home != NULL && home != vertex_home) {
      // Oops, two vertices are referenced by different joints!  The
      // primitive is dynamic.
      return NULL;
    }

    home = vertex_home;
  }

  // This shouldn't be possible, unless there are no vertices--but we
  // check for that before calling this function.
  nassertr(home != NULL, NULL);

  // So, all the vertices are assigned to the same group.  This means
  // the polygon belongs entirely to one joint.

  // If the group is not, in fact, a joint then we return the first
  // joint above the group.
  EggGroup *egg_group = (EggGroup *)NULL;
  if (home->is_of_type(EggGroup::get_class_type())) {
    egg_group = DCAST(EggGroup, home);
  }
  while (egg_group != (EggGroup *)NULL &&
         egg_group->get_group_type() != EggGroup::GT_joint &&
         egg_group->get_dart_type() == EggGroup::DT_none) {
    nassertr(egg_group->get_parent() != (EggGroupNode *)NULL, NULL);
    home = egg_group->get_parent();
    egg_group = (EggGroup *)NULL;
    if (home->is_of_type(EggGroup::get_class_type())) {
      egg_group = DCAST(EggGroup, home);
    }
  }

  if (egg_group != (EggGroup *)NULL &&
      egg_group->get_group_type() == EggGroup::GT_joint &&
      !egg_group->has_dcs_type()) {
    // If the home is a joint without a <DCS> flag--this is the normal
    // case--we'll move the polygon under the character node and
    // animate it from there explicitly.
    return NULL;
  }

  // Otherwise, if the joint *does* have a <DCS> flag, we'll create
  // static geometry that we parent directly to the joint node.
  // We'll also create static geometry for polygons that have no
  // explicit joint assignment.
  return home;
}

////////////////////////////////////////////////////////////////////
//     Function: CharacterMaker::determine_bin_home
//       Access: Private
//  Description: Examines the joint assignment of the vertices of all
//               of the primitives within this bin to determine which
//               parent node the bin's polyset should be created
//               under.
////////////////////////////////////////////////////////////////////
EggGroupNode *CharacterMaker::
determine_bin_home(EggBin *egg_bin) {
  // A primitive's vertices may be referenced by any joint in the
  // character.  Or, the primitive itself may be explicitly placed
  // under a joint.

  // If any of the vertices, in any primitive, are referenced by
  // multiple joints, or if any two vertices are referenced by
  // different joints, then the entire bin must be considered dynamic.
  // (We'll indicate a dynamic bin by returning NULL.)

  // We need to keep track of the one joint we've encountered so far,
  // to see if all the vertices are referenced by the same joint.
  EggGroupNode *home = NULL;

  EggGroupNode::const_iterator ci;
  for (ci = egg_bin->begin(); ci != egg_bin->end(); ++ci) {
    CPT(EggPrimitive) egg_primitive = DCAST(EggPrimitive, (*ci));

    EggPrimitive::const_iterator vi;
    for (vi = egg_primitive->begin();
         vi != egg_primitive->end();
         ++vi) {
      EggVertex *vertex = (*vi);
      if (vertex->gref_size() > 1) {
        // This vertex is referenced by multiple joints; the primitive
        // is dynamic.
        return NULL;
      }
      
      if (!vertex->_dxyzs.empty() ||
          !vertex->_dnormals.empty() ||
          !vertex->_drgbas.empty()) {
        // This vertex has some morph slider definitions; therefore, the
        // primitive is dynamic.
        return NULL;
      }
      EggVertex::const_uv_iterator uvi;
      for (uvi = vertex->uv_begin(); uvi != vertex->uv_end(); ++uvi) {
        if (!(*uvi)->_duvs.empty()) {
          // Ditto: the vertex has some UV morphs; therefore the
          // primitive is dynamic.
          return NULL;
        }
      }

      EggGroupNode *vertex_home;
      
      if (vertex->gref_size() == 0) {
        // This vertex is not referenced at all, which means it belongs
        // right where it is.
        vertex_home = egg_primitive->get_parent();
      } else {
        nassertr(vertex->gref_size() == 1, NULL);
        // This vertex is referenced exactly once.
        vertex_home = *vertex->gref_begin();
      }
      
      if (home != NULL && home != vertex_home) {
        // Oops, two vertices are referenced by different joints!  The
        // primitive is dynamic.
        return NULL;
      }

      home = vertex_home;
    }
  }

  // This shouldn't be possible, unless there are no vertices--but we
  // eliminate invalid primitives before we begin, so all primitives
  // should have vertices, and all bins should have primitives.
  nassertr(home != NULL, NULL);

  // So, all the vertices are assigned to the same group.  This means
  // all the primitives in the bin belong entirely to one joint.

  // If the group is not, in fact, a joint then we return the first
  // joint above the group.
  EggGroup *egg_group = (EggGroup *)NULL;
  if (home->is_of_type(EggGroup::get_class_type())) {
    egg_group = DCAST(EggGroup, home);
  }
  while (egg_group != (EggGroup *)NULL &&
         egg_group->get_group_type() != EggGroup::GT_joint &&
         egg_group->get_dart_type() == EggGroup::DT_none) {
    nassertr(egg_group->get_parent() != (EggGroupNode *)NULL, NULL);
    home = egg_group->get_parent();
    egg_group = (EggGroup *)NULL;
    if (home->is_of_type(EggGroup::get_class_type())) {
      egg_group = DCAST(EggGroup, home);
    }
  }

  if (egg_group != (EggGroup *)NULL &&
      egg_group->get_group_type() == EggGroup::GT_joint &&
      !egg_group->has_dcs_type()) {
    // If we have rigid geometry that is assigned to a joint without a
    // <DCS> flag, which means the joint didn't get created as its own
    // node, go ahead and make an implicit node for the joint.

    if (egg_group->get_dcs_type() == EggGroup::DC_none) {
      // Unless the user specifically forbade exposing the joint by
      // putting an explicit "<DCS> { none }" entry in the joint.  In
      // this case, we return NULL to treat the geometry as dynamic
      // (and animate it by animating its vertices), but display lists
      // and vertex buffers will perform better if as much geometry as
      // possible is rigid.
      return NULL;
    }

    CharacterJoint *joint;
    DCAST_INTO_R(joint, egg_to_part(egg_group), home);
    egg_group->set_dcs_type(EggGroup::DC_default);

    PT(ModelNode) geom_node = new ModelNode(egg_group->get_name());
    geom_node->set_preserve_transform(ModelNode::PT_local);
    joint->_geom_node = geom_node.p();
  }

  return home;
}

////////////////////////////////////////////////////////////////////
//     Function: CharacterMaker::get_identity_transform
//       Access: Private
//  Description: Returns a VertexTransform that represents the root of
//               the character--it never animates.
////////////////////////////////////////////////////////////////////
VertexTransform *CharacterMaker::
get_identity_transform() {
  if (_identity_transform == (VertexTransform *)NULL) {
    _identity_transform = new UserVertexTransform("root");
  }
  return _identity_transform;
}
