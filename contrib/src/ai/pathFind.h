/**
 * PANDA 3D SOFTWARE
 * Copyright (c) Carnegie Mellon University.  All rights reserved.
 *
 * All use of this software is subject to the terms of the revised BSD
 * license.  You should have received a copy of this license along
 * with this source code in a file named "LICENSE."
 *
 * @file pathFind.h
 * @author Deepak, John, Navin
 * @date 2009-10-12
 */

#ifndef _PATHFIND_H
#define _PATHFIND_H

#include "aiGlobals.h"
#include "aiCharacter.h"
#include "aiPathFinder.h"
#include "boundingSphere.h"

class AICharacter;

/**
 * This class contains all the members and functions that are required to form
 * an interface between the AIBehaviors class and the PathFinder class.  An
 * object (pointer) of this class is provided in the AIBehaviors class.  It is
 * only via this object that the user can activate pathfinding.
 */
class EXPCL_PANDAAI PathFind {
public:
  AICharacter *_ai_char;
  PathFinder *_path_finder_obj;

  NavMesh _nav_mesh;
  NavMesh _stage_mesh;

  int _grid_size;
  NodePath _path_find_target;
  LVecBase3 _prev_position;
  PT(GeomNode) _parent;
  LineSegs *_pen;
  std::vector<int> _previous_obstacles;
  bool _dynamic_avoid;
  std::vector<NodePath> _dynamic_obstacle;

  PathFind(AICharacter *ai_ch);
  ~PathFind();

  void clear_path();
  void trace_path(Node* src);

  void create_nav_mesh(const char* navmesh_filename);
  void assign_neighbor_nodes(const char* navmesh_filename);
  void do_dynamic_avoid();
  void clear_previous_obstacles();

  void set_path_find(const char* navmesh_filename);
  void path_find(LVecBase3 pos, std::string type = "normal");
  void path_find(NodePath target, std::string type = "normal");
  void add_obstacle_to_mesh(NodePath obstacle);
  void dynamic_avoid(NodePath obstacle);
};

#endif
