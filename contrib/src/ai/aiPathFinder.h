// Filename: aiPathfinder.h
// Created by:  Deepak, John, Navin (08Sep09)
//
////////////////////////////////////////////////////////////////////
//
// PANDA 3D SOFTWARE
// Copyright (c) Carnegie Mellon University.  All rights reserved.
//
// All use of this software is subject to the terms of the revised 
// BSD license. You should have received a copy of this license along
// with this source code in a file named "LICENSE."
//
////////////////////////////////////////////////////////////////////

#ifndef PATHFINDER_H
#define PATHFINDER_H

#include "aiNode.h"
#include "cmath.h"
#include "lineSegs.h"

typedef vector<AINode *> NodeArray;
typedef vector<NodeArray> NavMesh;

AINode* find_in_mesh(NavMesh nav_mesh, LVecBase3f pos, int grid_size);

////////////////////////////////////////////////////////////////////
//       Class : PathFinder
// Description : This class implements pathfinding using the A*
//               algorithm. It also uses a Binary Heap search to
//               search the open list. The heuristics are
//               calculated using the manhattan method.
////////////////////////////////////////////////////////////////////
class PathFinder {
public:
  AINode *_src_node;
  AINode *_dest_node;
  vector<AINode*> _open_list;
  vector<AINode*> _closed_list;

  NavMesh _grid;

  void identify_neighbors(AINode *nd);
  int calc_cost_frm_src(AINode *nd);
  int calc_heuristic(AINode *nd);
  void calc_node_score(AINode *nd);
  bool is_diagonal_node(AINode *nd);

  void add_to_olist(AINode *nd);
  void remove_from_olist();

  void add_to_clist(AINode *nd);
  void remove_from_clist(int r, int c);

  void generate_path();
  void find_path(AINode *src_node, AINode *dest_node);
  PathFinder(NavMesh nav_mesh);
  ~PathFinder();
};

#endif
