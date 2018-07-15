/**
 * PANDA 3D SOFTWARE
 * Copyright (c) Carnegie Mellon University.  All rights reserved.
 *
 * All use of this software is subject to the terms of the revised BSD
 * license.  You should have received a copy of this license along
 * with this source code in a file named "LICENSE."
 *
 * @file maxNodeTree.cxx
 * @author crevilla
 * from mayaNodeTree.cxx created by:  drose (06Jun03)
 */

#include "maxEgg.h"

/**
 *
 */
MaxNodeTree::
MaxNodeTree() {
  _root = new MaxNodeDesc;
  _fps = 0.0;
  _export_mesh = false;
  _egg_data = nullptr;
  _egg_root = nullptr;
  _skeleton_node = nullptr;
}

/**
 * Returns a pointer to the node corresponding to the indicated INode object,
 * creating it first if necessary.
 */
MaxNodeDesc *MaxNodeTree::
build_node(INode *max_node) {
  MaxNodeDesc *node_desc = r_build_node(max_node);
  node_desc->from_INode(max_node);

  if (node_desc->is_node_joint()) {
    node_desc->_joint_entry = build_joint(max_node, node_desc);
  }
  return node_desc;
}

/**
 * Returns a pointer to the node corresponding to the indicated INode object,
 * creating it first if necessary.
 */
MaxNodeDesc *MaxNodeTree::
build_joint(INode *max_node, MaxNodeDesc *node_joint) {
  MaxNodeDesc *node_desc = r_build_joint(node_joint, max_node);
  node_desc->from_INode(max_node);
  node_desc->set_joint(true);
  return node_desc;
}

bool MaxNodeTree::node_in_list(ULONG handle, ULONG *list, int len) {
  if (!list) return true;
  for (int i = 0; i < len; i++)
    if (list[i] == handle) return true;
  return false;
}

bool MaxNodeTree::is_joint(INode *node) {
  Control *c = node->GetTMController();
  return (node->GetBoneNodeOnOff() ||                    //joints
         (c &&                                           //bipeds
         ((c->ClassID() == BIPSLAVE_CONTROL_CLASS_ID) ||
         (c->ClassID() == BIPBODY_CONTROL_CLASS_ID) ||
         (c->ClassID() == FOOTPRINT_CLASS_ID))));
}

bool MaxNodeTree::
r_build_hierarchy(INode *root, ULONG *selection_list, int len) {
  if (node_in_list(root->GetHandle(), selection_list, len))
    build_node(root);
  // Export children
  for ( int i = 0; i < root->NumberOfChildren(); i++ ) {
    // *** Should probably be checking the return value of the following line
    r_build_hierarchy(root->GetChildNode(i), selection_list, len);
  }
  return true;
}
/**
 * Walks through the complete Max hierarchy and builds up the corresponding
 * tree.
 */
bool MaxNodeTree::
build_complete_hierarchy(INode *root, ULONG *selection_list, int len) {

  // Get the entire Max scene.
  if (root == nullptr) {
    // *** Log an error
    return false;
  }

  bool all_ok = true;
  r_build_hierarchy(root, selection_list, len);

  if (all_ok) {
    _root->check_pseudo_joints(false);
  }

  return all_ok;
}

/**
 * Returns the total number of nodes in the hierarchy, not counting the root
 * node.
 */
int MaxNodeTree::
get_num_nodes() const {
  return _nodes.size();
}

/**
 * Returns the nth node in the hierarchy, in an arbitrary ordering.
 */
MaxNodeDesc *MaxNodeTree::
get_node(int n) const {
  nassertr(n >= 0 && n < (int)_nodes.size(), nullptr);
  return _nodes[n];
}

/**
 * Removes all of the references to generated egg structures from the tree,
 * and prepares the tree for generating new egg structures.
 */
void MaxNodeTree::
clear_egg(EggData *egg_data, EggGroupNode *egg_root,
          EggGroupNode *skeleton_node) {
  _root->clear_egg();
  _egg_data = egg_data;
  _egg_root = egg_root;
  _skeleton_node = skeleton_node;
}

/**
 * Returns the EggGroupNode corresponding to the group or joint for the
 * indicated node.  Creates the group node if it has not already been created.
 */
EggGroup *MaxNodeTree::
get_egg_group(MaxNodeDesc *node_desc) {
  nassertr(_egg_root != nullptr, nullptr);

  if (node_desc->_egg_group == nullptr) {
    // We need to make a new group node.
    EggGroup *egg_group;

    nassertr(node_desc->_parent != nullptr, nullptr);
    egg_group = new EggGroup(node_desc->get_name());
    if (node_desc->is_joint()) {
      egg_group->set_group_type(EggGroup::GT_joint);
    }
    if (node_desc->_parent == _root) {
      // The parent is the root.  Set collision properties for the root if it
      // has them:
      if(!_export_mesh)
      {
          set_collision_tags(node_desc, egg_group);
      }
      _egg_root->add_child(egg_group);

    } else {
      // The parent is another node.  if export mesh, the tag should be added
      // at the second level
      if(_export_mesh)
      {
        if(node_desc->_parent->_parent == _root)
        {
            set_collision_tags(node_desc, egg_group);
        }
      }
      EggGroup *parent_egg_group = get_egg_group(node_desc->_parent);
      parent_egg_group->add_child(egg_group);
    }

    node_desc->_egg_group = egg_group;
  }

  return node_desc->_egg_group;
}

/**
 * Returns the EggTable corresponding to the joint for the indicated node.
 * Creates the table node if it has not already been created.
 */
EggTable *MaxNodeTree::
get_egg_table(MaxNodeDesc *node_desc) {
  nassertr(_skeleton_node != nullptr, nullptr);
  nassertr(node_desc->is_joint(), nullptr);

  if (node_desc->_egg_table == nullptr) {
    // We need to make a new table node.
    nassertr(node_desc->_parent != nullptr, nullptr);

    EggTable *egg_table = new EggTable(node_desc->get_name());
    node_desc->_anim = new EggXfmSAnim("xform",
                                       _egg_data->get_coordinate_system());
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

/**
 * Returns the anim table corresponding to the joint for the indicated node.
 * Creates the table node if it has not already been created.
 */
EggXfmSAnim *MaxNodeTree::
get_egg_anim(MaxNodeDesc *node_desc) {
  get_egg_table(node_desc);
  return node_desc->_anim;
}

/**
 * The recursive implementation of build_node().
 */
MaxNodeDesc *MaxNodeTree::
r_build_node(INode* max_node) {
  // If we have already encountered this pathname, return the corresponding
  // MaxNodeDesc immediately.

  ULONG node_handle = 0;

  if (max_node) {
    node_handle = max_node->GetHandle();
  }

  NodesByPath::const_iterator ni = _nodes_by_path.find(node_handle);
  if (ni != _nodes_by_path.end()) {
    return (*ni).second;
  }

  // Otherwise, we have to create it.  Do this recursively, so we create each
  // node along the path.
  MaxNodeDesc *node_desc;

  if (!max_node) {
    // This is the top.
    node_desc = _root;

  } else {
    INode *parent_node;

    if (max_node->IsRootNode()) {
      parent_node = nullptr;
    } else {
      parent_node = max_node->GetParentNode();
    }

    MaxNodeDesc *parent_node_desc = r_build_node(parent_node);
    node_desc = new MaxNodeDesc(parent_node_desc, max_node);
    _nodes.push_back(node_desc);
  }

  _nodes_by_path.insert(NodesByPath::value_type(node_handle, node_desc));
  return node_desc;
}

/**
 * The recursive implementation of build_joint().
 */
MaxNodeDesc *MaxNodeTree::
r_build_joint(MaxNodeDesc *node_desc, INode *max_node)
{
  MaxNodeDesc *node_joint;
  if (node_desc == _root) {
    node_joint = new MaxNodeDesc(_root, max_node);
    _nodes.push_back(node_joint);
    return node_joint;
  } else if (node_desc->is_node_joint() && node_desc->_joint_entry) {
    node_joint = new MaxNodeDesc(node_desc->_joint_entry, max_node);
    _nodes.push_back(node_joint);
    return node_joint;
  } else {
    return r_build_joint(node_desc->_parent, max_node);
  }
}

/**
 * The recursive implementation of build_node().
 */
MaxNodeDesc *MaxNodeTree::
find_node(INode* max_node) {
  // If we have already encountered this pathname, return the corresponding
  // MaxNodeDesc immediately.

  ULONG node_handle = 0;

  if (max_node) {
    node_handle = max_node->GetHandle();
  }

  NodesByPath::const_iterator ni = _nodes_by_path.find(node_handle);
  if (ni != _nodes_by_path.end()) {
    return (*ni).second;
  }

  return nullptr;
}

/**
 * The recursive implementation of build_node().
 */
MaxNodeDesc *MaxNodeTree::
find_joint(INode* max_node)
{
  MaxNodeDesc *node = find_node(max_node);
  if (!node || (is_joint(max_node) && !node->is_node_joint()))
    node = build_node(max_node);
  return node->_joint_entry;
}

/**
 * Sets the corresponding collision tag to the egg_group based on the User
 * Defined Tab in the object properties panel
 */
void MaxNodeTree::set_collision_tags(MaxNodeDesc *node_desc, EggGroup *egg_group) {
    // Max has huge problems passing strings and bools to Get and SetUserProp
    // So instead we have to use Integers.  Now we have to check for every
    // collide type, then get its collide flags and do some number crunching
    // to get the actual flag into the group

    int check = 1; //is the value true. This could be anything really

      // We have to check each collision type in turn to see if it's true Ugly
      // but it works per object, not globaly
    if (node_desc->get_max_node()->GetUserPropInt(_T("polyset"), check)) {
        // we have a polyset.
      if (check == 1) {
        egg_group->set_collision_name(node_desc->get_name());
        egg_group->set_cs_type(EggGroup::CST_polyset);
      }
    }
    if (node_desc->get_max_node()->GetUserPropInt(_T("plane"), check)) {
      // plane
      if (check == 1) {
        egg_group->set_collision_name(node_desc->get_name());
        egg_group->set_cs_type(EggGroup::CST_plane);
      }
    }
    if (node_desc->get_max_node()->GetUserPropInt(_T("polygon"), check)) {
      // polygon
      if (check == 1) {
        egg_group->set_collision_name(node_desc->get_name());
        egg_group->set_cs_type(EggGroup::CST_polygon);
      }
    }
    if (node_desc->get_max_node()->GetUserPropInt(_T("sphere"), check)) {
      // sphere
      if (check == 1) {
        egg_group->set_collision_name(node_desc->get_name());
        egg_group->set_cs_type(EggGroup::CST_sphere);
      }
    }
    if (node_desc->get_max_node()->GetUserPropInt(_T("inv-sphere"), check)) {
      // invsphere
      if (check == 1) {
        egg_group->set_collision_name(node_desc->get_name());
        egg_group->set_cs_type(EggGroup::CST_inv_sphere);
      }
    }
    if (node_desc->get_max_node()->GetUserPropInt(_T("invsphere"), check)) {
      // invsphere (different spelling)
      if (check == 1) {
        egg_group->set_collision_name(node_desc->get_name());
        egg_group->set_cs_type(EggGroup::CST_inv_sphere);
      }
    }
    if (node_desc->get_max_node()->GetUserPropInt(_T("tube"), check)) {
      // tube
      if (check == 1) {
        egg_group->set_collision_name(node_desc->get_name());
        egg_group->set_cs_type(EggGroup::CST_tube);
      }
    }
    if (node_desc->get_max_node()->GetUserPropInt(_T("floor-mesh"), check)) {
      // floor-mesh
      if (check == 1) {
        egg_group->set_collision_name(node_desc->get_name());
        egg_group->set_cs_type(EggGroup::CST_floor_mesh);
      }
    }

    if (node_desc->get_max_node()->GetUserPropInt(_T("descend"), check)) {
      if (check == 1) {
      // we have the descend flag specified
      egg_group->set_collide_flags(EggGroup::CF_descend);
      }
    }
    if (node_desc->get_max_node()->GetUserPropInt(_T("event"), check)) {
      if (check == 1) {
      // we have the event flag specified
      egg_group->set_collide_flags(EggGroup::CF_event);
      }
    }
    if (node_desc->get_max_node()->GetUserPropInt(_T("keep"), check)) {
      if (check == 1) {
      // we have the keep flag specified
      egg_group->set_collide_flags(EggGroup::CF_keep);
      }
    }
    if (node_desc->get_max_node()->GetUserPropInt(_T("solid"), check)) {
      if (check == 1) {
      // we have the solid flag specified
      egg_group->set_collide_flags(EggGroup::CF_solid);
      }
    }
    if (node_desc->get_max_node()->GetUserPropInt(_T("center"), check)) {
      if (check == 1) {
      // we have the center flag specified
      egg_group->set_collide_flags(EggGroup::CF_center);
      }
    }
    if (node_desc->get_max_node()->GetUserPropInt(_T("turnstile"), check)) {
      if (check == 1) {
      // we have the turnstile flag specified
      egg_group->set_collide_flags(EggGroup::CF_turnstile);
      }
    }
    if (node_desc->get_max_node()->GetUserPropInt(_T("level"), check)) {
      if (check == 1) {
      // we have the level flag specified
      egg_group->set_collide_flags(EggGroup::CF_level);
      }
    }
    if (node_desc->get_max_node()->GetUserPropInt(_T("intangible"), check)) {
      if (check == 1) {
      // we have the intangible flag specified
      egg_group->set_collide_flags(EggGroup::CF_intangible);
      }
    }
    return;
}
