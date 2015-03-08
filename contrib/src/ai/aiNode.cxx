// Filename: aiNode.cxx
// Created by:  Deepak, John, Navin (19Nov2009)
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

#include "aiNode.h"

AINode::AINode(int grid_x, int grid_y, LVecBase3 pos, float w, float l, float h) {
  for (int i = 0; i < 8; ++i) {
    _neighbours[i] = NULL;
  }

  _position = pos;
  _width = w;
  _length = l;
  _height = h;
  _grid_x = grid_x;
  _grid_y = grid_y;
  _status = ST_neutral;
  _type = true;
  _score = 0;
  _cost = 0;
  _heuristic = 0;
  _next = NULL;
  _prv_node =  NULL;
}

AINode::~AINode() {
}

////////////////////////////////////////////////////////////////////
//     Function: contains
//  Description: This is a handy function which returns true if the
//               passed position is within the node's dimensions.
////////////////////////////////////////////////////////////////////
bool AINode::contains(float x, float y) {
  if (_position.get_x() - _width / 2 <= x && _position.get_x() + _width / 2 >= x &&
    _position.get_y() - _length / 2 <= y && _position.get_y() + _length / 2 >= y) {
    return true;
  }
  else {
    return false;
  }
}
