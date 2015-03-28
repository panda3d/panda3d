// Filename: aiNode.h
// Created by:  Deepak, John, Navin (18Nov2009)
//
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

#ifndef AINODE_H
#define AINODE_H

#include "aiGlobals.h"

////////////////////////////////////////////////////////////////////
//       Class : AINode
// Description : This class is used to assign the nodes on the mesh.
//               It holds all the data necessary to compute A*
//               algorithm. It also maintains a lot of vital
//               information such as the neighbor nodes of each
//               node and also its position on the mesh.
//               Note: The Mesh Generator which is a standalone
//               tool makes use of this class to generate the nodes
//               on the mesh.
////////////////////////////////////////////////////////////////////
class EXPCL_PANDAAI AINode {
public:
  // This variable specifies the node status whether open, close
  // or neutral.
  // open = belongs to _open_list.
  // close = belongs to _closed_list.
  // neutral = unexamined node.
  enum Status {
    ST_open,
    ST_close,
    ST_neutral
  };
  Status _status;

  // This variable specifies whether the node is an obtacle or not.
  // Used for dynamic obstacle addition to the environment.
  // obstacle = false
  // navigational = true
  bool _type;

  // The score is used to compute the traversal expense to nodes
  // when using A*.
  // _score = _cost + heuristic
  int _score;
  int _cost;
  int _heuristic;

  // Used to trace back the path after it is generated using A*.
  AINode *_prv_node;

  // Position of the node in the 2d grid.
  int _grid_x, _grid_y;

  // Position of the node in 3D space.
  LVecBase3 _position;

  // Dimensions of each face / cell on the mesh.
  // Height is given in case of expansion to a 3d mesh. Currently
  // not used.
  float _width, _length ,_height;
  AINode *_neighbours[8]; // anti-clockwise from top left corner.

  // The _next pointer is used for traversal during mesh
  // generation from the model.
  // Note: The data in this member is discarded when mesh data
  // is written into navmesh.csv file.
  AINode *_next;

PUBLISHED:
  AINode(int grid_x, int grid_y, LVecBase3 pos, float w, float l, float h);
  ~AINode();

  bool contains(float x, float y);
};

#endif
