// Filename: maxNodeTree.h
// Created by: crevilla
// from mayaNodeTree.h created by:  drose (06Jun03)
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

#ifndef MAXNODETREE_H
#define MAXNODETREE_H

#include "pandatoolbase.h"

#include "maxNodeDesc.h"

class EggData;
class EggGroupNode;

////////////////////////////////////////////////////////////////////
//       Class : MaxNodeTree
// Description : Describes a complete tree of max nodes for
//               conversion.
////////////////////////////////////////////////////////////////////
class MaxNodeTree {
public:
  MaxNodeTree();
  MaxNodeDesc *build_node(INode *max_node);
  MaxNodeDesc *build_joint(INode *max_node, MaxNodeDesc *node_joint);
  bool build_complete_hierarchy(INode *root);
  bool build_selected_hierarchy(INode *root);
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
  float _fps;

private:
  EggData *_egg_data;
  EggGroupNode *_egg_root;
  EggGroupNode *_skeleton_node;

  MaxNodeDesc *r_build_node(INode *max_node);
  MaxNodeDesc *r_build_joint(MaxNodeDesc *node_desc, INode *max_node);
  bool r_build_hierarchy(INode *root);

  typedef pmap<ULONG, MaxNodeDesc *> NodesByPath;
  NodesByPath _nodes_by_path;

  typedef pvector<MaxNodeDesc *> Nodes;
  Nodes _nodes;
};

#endif
