////////////////////////////////////////////////////////////////////////
// Filename    : aiPathFinder.h
// Created by  : Deepak, John, Navin
// Date        :  10 Nov 09
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

#ifndef _PATHFINDER_H
#define _PATHFINDER_H

#include "meshNode.h"
#include "cmath.h"
#include "lineSegs.h"

typedef vector<Node *> NodeArray;
typedef vector<NodeArray> NavMesh;

Node* find_in_mesh(NavMesh nav_mesh, LVecBase3 pos, int grid_size);

////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//
// Class : PathFinder
//  Description : This class implements pathfinding using A* algorithm. It also uses a Binary Heap search to
//               search the open list. The heuristics are calculated using the manhattan method.

////////////////////////////////////////////////////////////////////////////////////////////////////////////////

class EXPCL_PANDAAI PathFinder {
public:
  Node *_src_node;
  Node *_dest_node;
  vector<Node*> _open_list;
  vector<Node*> _closed_list;

  NavMesh _grid;

  void identify_neighbors(Node *nd);
  int calc_cost_frm_src(Node *nd);
  int calc_heuristic(Node *nd);
  void calc_node_score(Node *nd);
  bool is_diagonal_node(Node *nd);

  void add_to_olist(Node *nd);
  void remove_from_olist();

  void add_to_clist(Node *nd);
  void remove_from_clist(int r, int c);

  void generate_path();
  void find_path(Node *src_node, Node *dest_node);
  PathFinder(NavMesh nav_mesh);
  ~PathFinder();
};

#endif

