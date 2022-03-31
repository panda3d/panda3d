/**
 * PANDA 3D SOFTWARE
 * Copyright (c) Carnegie Mellon University.  All rights reserved.
 *
 * All use of this software is subject to the terms of the revised BSD
 * license.  You should have received a copy of this license along
 * with this source code in a file named "LICENSE."
 *
 * @file aiPathFinder.cxx
 * @author Deepak, John, Navin
 * @date 2009-11-10
 */

#include "aiPathFinder.h"

PathFinder::PathFinder(NavMesh nav_mesh) {
  _grid = nav_mesh;
}

PathFinder::~PathFinder() {
}

/**
 * This function initializes the pathfinding process by accepting the source
 * and destination nodes.  It then calls the generate_path().
 */
void PathFinder::find_path(Node *src_node, Node *dest_node) {
  _src_node = src_node;
  _dest_node = dest_node;

  // Add a dummy node as the first element of the open list with score = -1.
  // Inorder to implement a binary heap the index of the elements should never
  // be 0.
  Node *_dummy_node = new Node(-1, -1, LVecBase3(0.0, 0.0, 0.0), 0, 0, 0);
  _dummy_node->_status = _dummy_node->open;
  _dummy_node->_score = -1;
  _open_list.push_back(_dummy_node);

  // Add the source node to the open list.
  add_to_olist(_src_node);

  // Generate the path.
  generate_path();
}

/**
 * This function performs the pathfinding process using the A* algorithm.  It
 * updates the openlist and closelist.
 */
void PathFinder::generate_path() {
  // All the A* algorithm is implemented here.  The check is > 1 due to the
  // existence of the dummy node.
  while(_open_list.size() > 1) {
    // The first element of the open list will always be the optimal node.
    // This is because the open list is a binary heap with element having the
    // smallest score at the top of the heap.
    Node* nxt_node = _open_list[1];

    if(nxt_node->_grid_x == _dest_node->_grid_x &&
              nxt_node->_grid_y == _dest_node->_grid_y) {
      // Remove the optimal node from the top of the heap.
      remove_from_olist();

      // add the used node to the closed list.
      add_to_clist(nxt_node);

      // At this point the destination is reached.
      return;
    }
    else {
      identify_neighbors(nxt_node);

      // add the used node to the closed list.
      add_to_clist(nxt_node);
    }
  }
  std::cout << "DESTINATION NOT REACHABLE MATE!" << std::endl;
  _closed_list.clear();
}

/**
 * This function traverses through the 8 neigbors of the parent node and then
 * adds the neighbors to the _open_list based on A* criteria.
 */
void PathFinder::identify_neighbors(Node *parent_node) {
  // Remove the parent node from the open_list so that it is not considered
  // while adding new nodes to the open list heap.
  remove_from_olist();
  for(int i = 0; i < 8; ++i) {
    if(parent_node->_neighbours[i] != nullptr) {
      if(parent_node->_neighbours[i]->_status == parent_node->_neighbours[i]->neutral
        && parent_node->_neighbours[i]->_type == true) {
        // Link the neighbor to the parent node.
        parent_node->_neighbours[i]->_prv_node = parent_node;
        // Calculate and update the score for the node.
        calc_node_score(parent_node->_neighbours[i]);
        // Add the neighbor to the open list.
        add_to_olist(parent_node->_neighbours[i]);
      }
    }
  }
}

/**
 * This function calculates the score of each node.  Score = Cost +
 * Heuristics.
 */
void PathFinder::calc_node_score(Node *nd) {
  nd->_cost = calc_cost_frm_src(nd);
  nd->_heuristic = calc_heuristic(nd);
  nd->_score = nd->_cost + nd->_heuristic;
}

/**
 * This function calculates the cost of each node by finding out the number of
 * node traversals required to reach the source node.  Diagonal traversals
 * have cost = 14.  Horizontal and vertical traversals have cost = 10.
 */
int PathFinder::calc_cost_frm_src(Node *nd) {
  int cost = 0;
  Node *start_node = nd;
  while(start_node->_prv_node != _src_node) {
    if(is_diagonal_node(start_node)) {
      cost += 14;
    }
    else {
      cost += 10;
    }
    start_node = start_node->_prv_node;
  }
  // Add the cost of traversal to the source node also.
  if(is_diagonal_node(start_node)) {
    cost += 14;
  }
  else {
    cost += 10;
  }
  return cost;
}

/**
 * This function calculates the heuristic of the nodes using Manhattan method.
 * All it does is predict the number of node traversals required to reach the
 * target node.  No diagonal traversals are allowed in this technique.
 */
int PathFinder::calc_heuristic(Node *nd) {
  int row_diff = abs(_dest_node->_grid_x - nd->_grid_x);
  int col_diff = abs(_dest_node->_grid_y - nd->_grid_y);

  int heuristic = 10 * (row_diff + col_diff);
  return heuristic;
}

/**
 * This function checks if the traversal from a node is diagonal.
 */
bool PathFinder::is_diagonal_node(Node *nd) {
  // Calculate the row and column differences between child and parent nodes.
  float row_diff = nd->_grid_x - nd->_prv_node->_grid_x;
  float col_diff = nd->_grid_y - nd->_prv_node->_grid_y;

  // Check if the relationship between child and parent node is diagonal.
  if(row_diff == 0 || col_diff == 0) {
    return false;
  }
  else {
    return true;
  }
}

/**
 * This function adds a node to the open list heap.  A binay heap is
 * maintained to improve the search.
 */
void PathFinder::add_to_olist(Node *nd) {
  // Variables required to search the binary heap.
  Node *child_node, *parent_node;
  int child_idx, parent_idx;

  // Set the status as open.
  nd->_status = nd->open;
  // Add the node to the open list.
  _open_list.push_back(nd);

  // Find the parent and child nodes and create temporary nodes out of them.
  // In a binary heap the children of a parent node are always i*2 and i*2 +
  // 1, where i is the index of the parent node in the heap.  And hence, the
  // parent of a node can be easily found out by dividing by 2 and rounding
  // it.
  child_idx = _open_list.size() - 1;
  parent_idx = child_idx / 2;
  child_node = _open_list[child_idx];
  parent_node = _open_list[parent_idx];

  // Keep traversing the heap until the lowest element in the list is bubbled
  // to the top of the heap.
  while(_open_list[child_idx]->_score <= _open_list[parent_idx]->_score) {
    // Swap the parent node and the child node.
    _open_list[parent_idx] = child_node;
    _open_list[child_idx] = parent_node;

    // Update the new child and parent indices.
    child_idx = parent_idx;
    parent_idx = child_idx / 2;

    // Update the new child and parent nodes.
    child_node = _open_list[child_idx];
    parent_node = _open_list[parent_idx];
  }

  // At this point the Node with the smallest score will be at the top of the
  // heap.
}

/**
 * This function removes a node from the open list.  During the removal the
 * binary heap is maintained.
 */
void PathFinder::remove_from_olist() {
  // Variables for maintaining the binary heap.
  Node *child_node, *child_node_1, *child_node_2;
  int child_idx, child_idx_1, child_idx_2;

  // Remove the Node at index 1 from the open list binary heap.  Note: Node at
  // index 0 of open list is a dummy node.
  _open_list.erase(_open_list.begin() + 1);

  if(_open_list.size() > 1) {
    // Store the last element in the open list to a temp_node.
    Node *temp_node = _open_list[_open_list.size() - 1];

    // Shift the elements of the open list to the right by 1 element
    // circularly, excluding element at 0 index.
    for(int i = _open_list.size() - 1; i > 1; --i) {
      _open_list[i] = _open_list[i - 1];
    }

    // Assign the temp_node to 1st element in the open list.
    _open_list[1] = temp_node;

    // Set the iterator for traversing the node from index 1 in the heap.
    unsigned int k = 1;

    // This loop traverses down the open list till the node reaches the
    // correct position in the binary heap.
    while(true) {
      if((k * 2 + 1) < _open_list.size()) {
        // Two children exists for the parent node.
        child_idx_1 = k * 2;
        child_idx_2 = k * 2 + 1;
        child_node_1 = _open_list[child_idx_1];
        child_node_2 = _open_list[child_idx_2];

        if(_open_list[child_idx_1]->_score < _open_list[child_idx_2]->_score) {
          if(_open_list[k]->_score > _open_list[child_idx_1]->_score) {
            // Swap the parent node and the child node.
            _open_list[child_idx_1] = _open_list[k];
            _open_list[k] = child_node_1;

            // Update the parent node index.
            k = child_idx_1;
          }
          else {
            break;
          }
        }
        else {
          if(_open_list[k]->_score > _open_list[child_idx_2]->_score) {
            // Swap the parent node and the child node.
            _open_list[child_idx_2] = _open_list[k];
            _open_list[k] = child_node_2;

            // Update the parent node index.
            k = child_idx_2;
          }
          else {
            break;
          }
        }
      }
      else if((k * 2) < _open_list.size()) {
        // Only one child exists for the parent node.
        child_idx = k * 2;
        child_node = _open_list[child_idx];

        if(_open_list[k]->_score > _open_list[child_idx]->_score) {
          // Swap the parent node and the child node.
          _open_list[child_idx] = _open_list[k];
          _open_list[k] = child_node;

          // Update the parent node index.
          k = child_idx;
        }
        else {
          break;
        }
      }
      else {
        // No children exists.
        break;
      }
    }
  }

  // At this point the Node was succesfully removed and the binary heap re-
  // arranged.
}

/**
 * This function adds a node to the closed list.
 */
void PathFinder::add_to_clist(Node *nd) {
  // Set the status as closed.
  nd->_status = nd->close;
  // Add the node to the close list.
  _closed_list.push_back(nd);
}

/**
 * This function removes a node from the closed list.
 */
void PathFinder::remove_from_clist(int r, int c) {
  for(unsigned int i = 0; i < _closed_list.size(); ++i) {
    if(_closed_list[i]->_grid_x == r && _closed_list[i]->_grid_y == c) {
      _closed_list.erase(_closed_list.begin() + i);
      break;
    }
  }
}

/**
 * This function allows the user to pass a position and it returns the
 * corresponding node on the navigation mesh.  A very useful function as it
 * allows for dynamic updation of the mesh based on position.
 */
Node* find_in_mesh(NavMesh nav_mesh, LVecBase3 pos, int grid_size) {
  int size = grid_size;
  float x = pos[0];
  float y = pos[1];

  for(int i = 0; i < size; ++i) {
    for(int j = 0; j < size; ++j) {
      if(nav_mesh[i][j] != nullptr && nav_mesh[i][j]->contains(x, y)) {
        return(nav_mesh[i][j]);
      }
    }
  }
  return nullptr;
}
