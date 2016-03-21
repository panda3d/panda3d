/**
 * PANDA 3D SOFTWARE
 * Copyright (c) Carnegie Mellon University.  All rights reserved.
 *
 * All use of this software is subject to the terms of the revised BSD
 * license.  You should have received a copy of this license along
 * with this source code in a file named "LICENSE."
 *
 * @file maxNodeTree.h
 * @author crevilla
 * from mayaNodeTree.h created by:  drose (06Jun03)
 */

#ifndef MAXNODETREE_H
#define MAXNODETREE_H

class EggData;
class EggGroupNode;

/**
 * Describes a complete tree of max nodes for conversion.
 */
class MaxNodeTree {
public:
  MaxNodeTree();
  MaxNodeDesc *build_node(INode *max_node);
  MaxNodeDesc *build_joint(INode *max_node, MaxNodeDesc *node_joint);
  bool build_complete_hierarchy(INode *root, ULONG *selection_list, int len);
  MaxNodeDesc *find_node(INode *max_node);
  MaxNodeDesc *find_joint(INode *max_node);

  int get_num_nodes() const;
  MaxNodeDesc *get_node(int n) const;

  void clear_egg(EggData *egg_data, EggGroupNode *egg_root,
                 EggGroupNode *skeleton_node);
  EggGroup *get_egg_group(MaxNodeDesc *node_desc);
  EggTable *get_egg_table(MaxNodeDesc *node_desc);
  EggXfmSAnim *get_egg_anim(MaxNodeDesc *node_desc);

  MaxNodeDesc* _root;
  PN_stdfloat _fps;
  // the flag for the setting up collision bool _has_collision;
  // EggGroup::CollideFlags _cf_type; EggGroup::CollisionSolidType _cs_type;
  bool _export_mesh;

private:
  EggData *_egg_data;
  EggGroupNode *_egg_root;
  EggGroupNode *_skeleton_node;

  MaxNodeDesc *r_build_node(INode *max_node);
  MaxNodeDesc *r_build_joint(MaxNodeDesc *node_desc, INode *max_node);
  bool node_in_list(ULONG handle, ULONG *list, int len);
  bool r_build_hierarchy(INode *root, ULONG *selection_list, int len);
  bool is_joint(INode *node);
  void set_collision_tags(MaxNodeDesc *node_desc, EggGroup *egg_group);

  typedef pmap<ULONG, MaxNodeDesc *> NodesByPath;
  NodesByPath _nodes_by_path;

  typedef pvector<MaxNodeDesc *> Nodes;
  Nodes _nodes;
};

#endif
