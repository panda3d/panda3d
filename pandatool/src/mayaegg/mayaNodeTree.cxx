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
#include "dcast.h"

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
//     Function: MayaNodeTree::build_hierarchy
//       Access: Public
//  Description: Walks through the complete Maya hierarchy and builds
//               up the corresponding tree, but does not tag any nodes
//               for conversion.
////////////////////////////////////////////////////////////////////
bool MayaNodeTree::
build_hierarchy() {
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
//     Function: MayaNodeTree::tag_all
//       Access: Public
//  Description: Tags the entire hierarchy for conversion.  This is
//               the normal behavior.
////////////////////////////////////////////////////////////////////
void MayaNodeTree::
tag_all() {
  _root->tag_recursively();
}

////////////////////////////////////////////////////////////////////
//     Function: MayaNodeTree::tag_selected
//       Access: Public
//  Description: Tags the just the selected hierarchy for conversion,
//               or the entire hierarchy if nothing is selected.
//               Returns true on success, false on failure.
////////////////////////////////////////////////////////////////////
bool MayaNodeTree::
tag_selected() {
  MStatus status;

  MItDag dag_iterator(MItDag::kDepthFirst, MFn::kTransform, &status);
  if (!status) {
    status.perror("MItDag constructor");
    return false;
  }

  MSelectionList selection;
  status = MGlobal::getActiveSelectionList(selection);
  if (!status) {
    status.perror("MGlobal::getActiveSelectionList");
    return false;
  }
  
  if (selection.isEmpty()) {
    mayaegg_cat.info()
      << "Selection list is empty.\n";
    tag_all();
    return true;
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
          build_node(dag_path)->tag();
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
//     Function: MayaNodeTree::tag_named
//       Access: Public
//  Description: Tags nodes matching the indicated glob (and all of
//               their children) for conversion.  Returns true on
//               success, false otherwise (e.g. the named node does
//               not exist).
////////////////////////////////////////////////////////////////////
bool MayaNodeTree::
tag_named(const GlobPattern &glob) {
  // There might be multiple nodes matching the name; search for all
  // of them.
  bool found_any = false;

  Nodes::iterator ni;
  for (ni = _nodes.begin(); ni != _nodes.end(); ++ni) {
    MayaNodeDesc *node = (*ni);
    if (glob.matches(node->get_name())) {
      node->tag_recursively();
      found_any = true;
    }
  }

  return found_any;
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
//     Function: MayaNodeTree::clear
//       Access: Public
//  Description: Resets the entire tree in preparation for
//               repopulating with a new scene.
////////////////////////////////////////////////////////////////////
void MayaNodeTree::
clear() {
  _root = new MayaNodeDesc;
  _fps = 0.0;
  _egg_data = (EggData *)NULL;
  _egg_root = (EggGroupNode *)NULL;
  _skeleton_node = (EggGroupNode *)NULL;
  _nodes_by_path.clear();
  _nodes.clear();
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

    MayaEggGroupUserData *parent_user_data = NULL;

    if (node_desc->_parent == _root) {
      // The parent is the root.
      _egg_root->add_child(egg_group);

    } else {
      // The parent is another node.
      EggGroup *parent_egg_group = get_egg_group(node_desc->_parent);
      parent_egg_group->add_child(egg_group);

      if (parent_egg_group->has_user_data()) {
        DCAST_INTO_R(parent_user_data, parent_egg_group->get_user_data(), NULL);
      }
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

      // Is the node flagged to be invisible?  If it is, it is tagged
      // with the "hidden" visibility flag, so it won't get converted
      // in the normal case (unless it represents a collision solid or
      // something).
      bool visible = true;
      get_bool_attribute(dag_object, "visibility", visible);
      if (!visible && egg_group->get_num_object_types() == 0) {
        egg_group->set_visibility_mode(EggGroup::VM_hidden);
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
        
      } else if (egg_group->has_object_type("bbpoint")) {
        egg_group->remove_object_type("bbpoint");
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
      
      // And "vertex-color" and "double-sided" have meaning only to
      // this converter.
      MayaEggGroupUserData *user_data;
      if (parent_user_data == (MayaEggGroupUserData *)NULL) {
        user_data = new MayaEggGroupUserData;
      } else {
        // Inherit the flags from above.
        user_data = new MayaEggGroupUserData(*parent_user_data);
      }

      if (egg_group->has_object_type("vertex-color")) {
        egg_group->remove_object_type("vertex-color");
        user_data->_vertex_color = true;
      }
      if (egg_group->has_object_type("double-sided")) {
        egg_group->remove_object_type("double-sided");
        user_data->_double_sided = true;
      }
      egg_group->set_user_data(user_data);
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
