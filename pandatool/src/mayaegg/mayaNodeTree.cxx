// Filename: mayaNodeTree.cxx
// Created by:  drose (06Jun03)
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

#include "mayaNodeTree.h"
#include "mayaEggGroupUserData.h"
#include "config_mayaegg.h"
#include "maya_funcs.h"
#include "eggGroup.h"
#include "eggTable.h"
#include "eggXfmSAnim.h"
#include "eggData.h"

#include "pre_maya_include.h"
#include <maya/MString.h>
#include <maya/MItDag.h>
#include <maya/MSelectionList.h>
#include <maya/MGlobal.h>
#include "post_maya_include.h"

////////////////////////////////////////////////////////////////////
//     Function: MayaNodeTree::Constructor
//       Access: Public
//  Description: 
////////////////////////////////////////////////////////////////////
MayaNodeTree::
MayaNodeTree() {
  _root = new MayaNodeDesc;
  _fps = 0.0;
  _egg_data = (EggData *)NULL;
  _egg_root = (EggGroupNode *)NULL;
  _skeleton_node = (EggGroupNode *)NULL;
}

////////////////////////////////////////////////////////////////////
//     Function: MayaNodeTree::build_node
//       Access: Public
//  Description: Returns a pointer to the node corresponding to the
//               indicated dag_path object, creating it first if
//               necessary.
////////////////////////////////////////////////////////////////////
MayaNodeDesc *MayaNodeTree::
build_node(const MDagPath &dag_path) {
  MayaNodeDesc *node_desc = r_build_node(dag_path.fullPathName().asChar());
  node_desc->from_dag_path(dag_path);
  return node_desc;
}

////////////////////////////////////////////////////////////////////
//     Function: MayaNodeTree::build_complete_hierarchy
//       Access: Public
//  Description: Walks through the complete Maya hierarchy and builds
//               up the corresponding tree.
////////////////////////////////////////////////////////////////////
bool MayaNodeTree::
build_complete_hierarchy() {
  MStatus status;

  MItDag dag_iterator(MItDag::kDepthFirst, MFn::kTransform, &status);
  if (!status) {
    status.perror("MItDag constructor");
    return false;
  }

  // Get the entire Maya scene.
    
  // This while loop walks through the entire Maya hierarchy, one
  // node at a time.  Maya's MItDag object automatically performs a
  // depth-first traversal of its scene graph.
  
  bool all_ok = true;
  while (!dag_iterator.isDone()) {
    MDagPath dag_path;
    status = dag_iterator.getPath(dag_path);
    if (!status) {
      status.perror("MItDag::getPath");
    } else {
      build_node(dag_path);
    }
    
    dag_iterator.next();
  }

  if (all_ok) {
    _root->check_pseudo_joints(false);
  }
  
  return all_ok;
}

////////////////////////////////////////////////////////////////////
//     Function: MayaNodeTree::build_selected_hierarchy
//       Access: Public
//  Description: Walks through the selected subset of the Maya
//               hierarchy (or the complete hierarchy, if nothing is
//               selected) and builds up the corresponding tree.
////////////////////////////////////////////////////////////////////
bool MayaNodeTree::
build_selected_hierarchy() {
  MStatus status;

  MItDag dag_iterator(MItDag::kDepthFirst, MFn::kTransform, &status);
  if (!status) {
    status.perror("MItDag constructor");
    return false;
  }

  // Get only the selected geometry.
  MSelectionList selection;
  status = MGlobal::getActiveSelectionList(selection);
  if (!status) {
    status.perror("MGlobal::getActiveSelectionList");
    return false;
  }
  
  // Get the selected geometry only if the selection is nonempty;
  // otherwise, get the whole scene anyway.
  if (selection.isEmpty()) {
    mayaegg_cat.info()
      << "Selection list is empty.\n";
    return build_complete_hierarchy();
  }

  bool all_ok = true;
  unsigned int length = selection.length();
  for (unsigned int i = 0; i < length; i++) {
    MDagPath root_path;
    status = selection.getDagPath(i, root_path);
    if (!status) {
      status.perror("MSelectionList::getDagPath");
    } else {
      // Now traverse through the selected dag path and all nested
      // dag paths.
      dag_iterator.reset(root_path);
      while (!dag_iterator.isDone()) {
        MDagPath dag_path;
        status = dag_iterator.getPath(dag_path);
        if (!status) {
          status.perror("MItDag::getPath");
        } else {
          build_node(dag_path);
        }
        
        dag_iterator.next();
      }
    }
  }

  if (all_ok) {
    _root->check_pseudo_joints(false);
  }

  return all_ok;
}

////////////////////////////////////////////////////////////////////
//     Function: MayaNodeTree::get_num_nodes
//       Access: Public
//  Description: Returns the total number of nodes in the hierarchy,
//               not counting the root node.
////////////////////////////////////////////////////////////////////
int MayaNodeTree::
get_num_nodes() const {
  return _nodes.size();
}

////////////////////////////////////////////////////////////////////
//     Function: MayaNodeTree::get_node
//       Access: Public
//  Description: Returns the nth node in the hierarchy, in an
//               arbitrary ordering.
////////////////////////////////////////////////////////////////////
MayaNodeDesc *MayaNodeTree::
get_node(int n) const {
  nassertr(n >= 0 && n < (int)_nodes.size(), NULL);
  return _nodes[n];
}

////////////////////////////////////////////////////////////////////
//     Function: MayaNodeTree::clear_egg
//       Access: Public
//  Description: Removes all of the references to generated egg
//               structures from the tree, and prepares the tree for
//               generating new egg structures.
////////////////////////////////////////////////////////////////////
void MayaNodeTree::
clear_egg(EggData *egg_data, EggGroupNode *egg_root, 
          EggGroupNode *skeleton_node) {
  _root->clear_egg();
  _egg_data = egg_data;
  _egg_root = egg_root;
  _skeleton_node = skeleton_node;
}

////////////////////////////////////////////////////////////////////
//     Function: MayaNodeTree::get_egg_group
//       Access: Public
//  Description: Returns the EggGroupNode corresponding to the group
//               or joint for the indicated node.  Creates the group
//               node if it has not already been created.
////////////////////////////////////////////////////////////////////
EggGroup *MayaNodeTree::
get_egg_group(MayaNodeDesc *node_desc) {
  nassertr(_egg_root != (EggGroupNode *)NULL, NULL);

  if (node_desc->_egg_group == (EggGroup *)NULL) {
    // We need to make a new group node.
    EggGroup *egg_group;

    nassertr(node_desc->_parent != (MayaNodeDesc *)NULL, NULL);
    egg_group = new EggGroup(node_desc->get_name());
    if (node_desc->is_joint()) {
      egg_group->set_group_type(EggGroup::GT_joint);
    }

    if (node_desc->_parent == _root) {
      // The parent is the root.
      _egg_root->add_child(egg_group);

    } else {
      // The parent is another node.
      EggGroup *parent_egg_group = get_egg_group(node_desc->_parent);
      parent_egg_group->add_child(egg_group);
    }

    if (node_desc->has_dag_path()) {
      // Check for an object type setting, from Oliver's plug-in.
      MObject dag_object = node_desc->get_dag_path().node();
      string object_type;
      if (get_enum_attribute(dag_object, "eggObjectTypes1", object_type)) {
        egg_group->add_object_type(object_type);
      }
      if (get_enum_attribute(dag_object, "eggObjectTypes2", object_type)) {
        egg_group->add_object_type(object_type);
      }
      if (get_enum_attribute(dag_object, "eggObjectTypes3", object_type)) {
        egg_group->add_object_type(object_type);
      }

      // We treat the object type "billboard" as a special case: we
      // apply this one right away and also flag the group as an
      // instance.
      if (egg_group->has_object_type("billboard")) {    
        egg_group->remove_object_type("billboard");
        egg_group->set_group_type(EggGroup::GT_instance);
        egg_group->set_billboard_type(EggGroup::BT_axis);
        
      } else if (egg_group->has_object_type("billboard-point")) {    
        egg_group->remove_object_type("billboard-point");
        egg_group->set_group_type(EggGroup::GT_instance);
        egg_group->set_billboard_type(EggGroup::BT_point_camera_relative);
      }
      
      // We also treat the object type "dcs" and "model" as a special
      // case, so we can test for these flags later.
      if (egg_group->has_object_type("dcs")) {
        egg_group->remove_object_type("dcs");
        egg_group->set_dcs_type(EggGroup::DC_default);
      }
      if (egg_group->has_object_type("model")) {
        egg_group->remove_object_type("model");
        egg_group->set_model_flag(true);
      }
      
      // And "vertex-color" has meaning only to this converter.
      if (egg_group->has_object_type("vertex-color")) {
        egg_group->remove_object_type("vertex-color");
        MayaEggGroupUserData *user_data = new MayaEggGroupUserData;
        user_data->_vertex_color = true;
        egg_group->set_user_data(user_data);
      }
    }

    node_desc->_egg_group = egg_group;
  }

  return node_desc->_egg_group;
}

////////////////////////////////////////////////////////////////////
//     Function: MayaNodeTree::get_egg_table
//       Access: Public
//  Description: Returns the EggTable corresponding to the joint
//               for the indicated node.  Creates the table node if it
//               has not already been created.
////////////////////////////////////////////////////////////////////
EggTable *MayaNodeTree::
get_egg_table(MayaNodeDesc *node_desc) {
  nassertr(_skeleton_node != (EggGroupNode *)NULL, NULL);
  nassertr(node_desc->is_joint(), NULL);

  if (node_desc->_egg_table == (EggTable *)NULL) {
    // We need to make a new table node.
    nassertr(node_desc->_parent != (MayaNodeDesc *)NULL, NULL);

    EggTable *egg_table = new EggTable(node_desc->get_name());
    node_desc->_anim = new EggXfmSAnim("xform", _egg_data->get_coordinate_system());
    node_desc->_anim->set_fps(_fps);
    egg_table->add_child(node_desc->_anim);

    if (!node_desc->_parent->is_joint()) {
      // The parent is not a joint; put it at the top.
      _skeleton_node->add_child(egg_table);

    } else {
      // The parent is another joint.
      EggTable *parent_egg_table = get_egg_table(node_desc->_parent);
      parent_egg_table->add_child(egg_table);
    }

    node_desc->_egg_table = egg_table;
  }

  return node_desc->_egg_table;
}

////////////////////////////////////////////////////////////////////
//     Function: MayaNodeTree::get_egg_anim
//       Access: Public
//  Description: Returns the anim table corresponding to the joint
//               for the indicated node.  Creates the table node if it
//               has not already been created.
////////////////////////////////////////////////////////////////////
EggXfmSAnim *MayaNodeTree::
get_egg_anim(MayaNodeDesc *node_desc) {
  get_egg_table(node_desc);
  return node_desc->_anim;
}


////////////////////////////////////////////////////////////////////
//     Function: MayaNodeTree::r_build_node
//       Access: Private
//  Description: The recursive implementation of build_node().
////////////////////////////////////////////////////////////////////
MayaNodeDesc *MayaNodeTree::
r_build_node(const string &path) {
  // If we have already encountered this pathname, return the
  // corresponding MayaNodeDesc immediately.
  NodesByPath::const_iterator ni = _nodes_by_path.find(path);
  if (ni != _nodes_by_path.end()) {
    return (*ni).second;
  }

  // Otherwise, we have to create it.  Do this recursively, so we
  // create each node along the path.
  MayaNodeDesc *node_desc;

  if (path.empty()) {
    // This is the top.
    node_desc = _root;

  } else {
    // Maya uses vertical bars to separate path components.  Remove
    // everything from the rightmost bar on; this will give us the
    // parent's path name.
    size_t bar = path.rfind("|");
    string parent_path, local_name;
    if (bar != string::npos) {
      parent_path = path.substr(0, bar);
      local_name = path.substr(bar + 1);
    } else {
      local_name = path;
    }

    MayaNodeDesc *parent_node_desc = r_build_node(parent_path);
    node_desc = new MayaNodeDesc(parent_node_desc, local_name);
    _nodes.push_back(node_desc);
  }

  _nodes_by_path.insert(NodesByPath::value_type(path, node_desc));
  return node_desc;
}
