
#include "meshNode.h"

Node::Node(int grid_x, int grid_y, LVecBase3 pos, float w, float l, float h) {
  for(int i = 0; i < 8; ++i) {
    _neighbours[i] = nullptr;
  }

  _position = pos;
  _width = w;
  _length = l;
  _height = h;
  _grid_x = grid_x;
  _grid_y = grid_y;
  _status = neutral;
  _type = true;
  _score = 0;
  _cost = 0;
  _heuristic = 0;
  _next = nullptr;
  _prv_node =  nullptr;
}

Node::~Node() {
}

/**
 * This is a handy function which returns true if the passed position is
 * within the node's dimensions.
 */
bool Node::contains(float x, float y) {
  if(_position.get_x() - _width / 2 <= x && _position.get_x() + _width / 2 >= x &&
    _position.get_y() - _length / 2 <= y && _position.get_y() + _length / 2 >= y) {
    return true;
  }
  else {
    return false;
  }
}
