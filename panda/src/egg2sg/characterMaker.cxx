// Filename: characterMaker.cxx
// Created by:  drose (23Feb99)
// 
////////////////////////////////////////////////////////////////////

#include "characterMaker.h"
#include "eggLoader.h"
#include "config_egg2sg.h"

#include <computedVertices.h>
#include <eggGroup.h>
#include <eggPrimitive.h>
#include <partGroup.h>
#include <characterJoint.h>
#include <characterJointBundle.h>
#include <characterSlider.h>
#include <character.h>
#include <renderRelation.h>
#include <transformTransition.h>

////////////////////////////////////////////////////////////////////
//     Function: CharacterMaker::Construtor
//       Access: Public
//  Description: 
////////////////////////////////////////////////////////////////////
CharacterMaker::
CharacterMaker(EggGroup *root, EggLoader &loader)
  : _egg_root(root), _loader(loader) {

  _character_node = new Character(_egg_root->get_name());
  _bundle = _character_node->get_bundle();

  _morph_root = new PartGroup(_bundle, "morph");
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
  nassertr(index < _parts.size(), NULL);
  return _parts[index];
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
NamedNode *CharacterMaker::
part_to_node(PartGroup *part) const {
  if (part->is_of_type(CharacterJoint::get_class_type())) {
    CharacterJoint *joint = DCAST(CharacterJoint, part);
    if (joint->_geom_node != (NamedNode *)NULL) {
      return joint->_geom_node;
    }
  }

  return _character_node;
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
  CharacterSlider *slider = new CharacterSlider(_morph_root, name);
  int index = _parts.size();
  _parts.push_back(slider);
  return index;
}


////////////////////////////////////////////////////////////////////
//     Function: CharacterMaker::make_bundle
//       Access: Private
//  Description: 
////////////////////////////////////////////////////////////////////
CharacterJointBundle *CharacterMaker::
make_bundle() {
  build_joint_hierarchy(_egg_root, _skeleton_root);
  _bundle->sort_descendants();

  parent_joint_nodes(_skeleton_root);
  make_geometry(_egg_root);

  _character_node->_computed_vertices =
    _comp_verts_maker.make_computed_vertices(_character_node, *this);

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

      LMatrix4f matf(matd(0,0), matd(0,1), matd(0,2), matd(0,3),
		     matd(1,0), matd(1,1), matd(1,2), matd(1,3),
		     matd(2,0), matd(2,1), matd(2,2), matd(2,3),
		     matd(3,0), matd(3,1), matd(3,2), matd(3,3));

      CharacterJoint *joint =
	new CharacterJoint(part, egg_group->get_name(), matf);
      index = _parts.size();
      _parts.push_back(joint);

      PartGroup *pgroup = (PartGroup *)joint;

      if (egg_group->get_dcs_flag()) {
	// If the joint requested an explicit DCS, create a node for
	// it.
	joint->_geom_node = new NamedNode(egg_group->get_name());
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
    NamedNode *joint_node = joint->_geom_node;
    if (joint_node != NULL) {
      RenderRelation *arc = new RenderRelation(_character_node, joint_node);
      joint->add_net_transform(arc);
      arc->set_transition(new TransformTransition(joint->_net_transform));
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

      if (prim_home == NULL) {
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
//     Function: CharacterMaker::make_static_primitive
//       Access: Private
//  Description: 
////////////////////////////////////////////////////////////////////
void CharacterMaker::
make_static_primitive(EggPrimitive *egg_primitive, EggGroupNode *prim_home) {
  NamedNode *node = part_to_node(egg_to_part(prim_home));
  
  // We need this funny transform to convert from the coordinate
  // space of the original vertices to that of the new joint node.
  LMatrix4d transform = 
    egg_primitive->get_vertex_frame() *
    prim_home->get_node_frame_inv();
  
  _loader.make_nonindexed_primitive(egg_primitive, node, &transform);
}

////////////////////////////////////////////////////////////////////
//     Function: CharacterMaker::make_dynamic_primitive
//       Access: Private
//  Description: 
////////////////////////////////////////////////////////////////////
void CharacterMaker::
make_dynamic_primitive(EggPrimitive *egg_primitive, EggGroupNode *prim_home) {
  NamedNode *node = part_to_node(egg_to_part(prim_home));
  
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
  // the polygon belongs entirely to one joint.  If that joint happens
  // to have a <DCS> flag, and will therefore be an explicit animated
  // node, we might as well put the polygon under that joint.
  // Otherwise, we'll leave the polygon under the character node and
  // animate it there explicitly.

  if (home->is_of_type(EggGroup::get_class_type())) {
    EggGroup *egg_group = DCAST(EggGroup, home);
    if (egg_group->get_dcs_flag()) {
      return home;
    }
  }

  return NULL;
}
