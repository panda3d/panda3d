/**
 * PANDA 3D SOFTWARE
 * Copyright (c) Carnegie Mellon University.  All rights reserved.
 *
 * All use of this software is subject to the terms of the revised BSD
 * license.  You should have received a copy of this license along
 * with this source code in a file named "LICENSE."
 *
 * @file pathFind.cxx
 * @author Deepak, John, Navin
 * @date 2009-10-12
 */

#include "pathFind.h"

#include "pathFollow.h"

using std::cout;
using std::endl;
using std::string;

PathFind::PathFind(AICharacter *ai_ch) {
  _ai_char = ai_ch;

  _parent = new GeomNode("parent");
  _ai_char->_window_render.attach_new_node(_parent);

    _pen = new LineSegs("pen");
  _pen->set_color(1.0, 0.0, 0.0);
  _pen->set_thickness(2.0);

  _path_finder_obj = nullptr;
  _dynamic_avoid = false;
}

PathFind::~PathFind() {
}

/**
 * This function recreates the navigation mesh from the .csv file
 */
void PathFind::create_nav_mesh(const char* navmesh_filename) {
  // Stage variables.
  int grid_x, grid_y;
  float l, w, h;
  LVecBase3 position;

  // Variable to hold line data read from file.
  string line;

  // Array for storing data members obtained from each line of the file.
  string fields[10];

  // Open data file for reading.
  std::ifstream nav_mesh_file (navmesh_filename);

  if(nav_mesh_file.is_open()) {
    // Capture the grid size from the file.
    getline(nav_mesh_file, line);
    int pos = line.find(",");
    _grid_size = atoi((line.substr(pos + 1)).c_str());

    // Initialize the stage mesh with NULL nodes.
    for(int r = 0; r < _grid_size; ++r) {
      _nav_mesh.push_back(std::vector<Node*>());
      for(int c = 0; c < _grid_size; ++c) {
        _nav_mesh[r].push_back(nullptr);
      }
    }

    // Ignore the header of the navmesh.csv file.
    getline(nav_mesh_file, line);

    // Begin reading data from the file.
    while(!nav_mesh_file.eof()) {
      getline(nav_mesh_file, line);
      std::stringstream linestream (line);

      // Stores all the data members in the line to the array.  Data
      // structure:
      // NULL,NodeType,GridX,GridY,Length,Width,Height,PosX,PosY,PosZ
      for(int i = 0; i < 10; ++i) {
        getline(linestream, fields[i], ',');
      }

      // Populate the main nodes into stage mesh.
      if(fields[0] == "0" && fields[1] == "0") {
        grid_x = atoi(fields[2].c_str());
        grid_y = atoi(fields[3].c_str());
        l = atof(fields[4].c_str());
        w = atof(fields[5].c_str());
        h = atof(fields[6].c_str());
        position = LVecBase3(atof(fields[7].c_str()), atof(fields[8].c_str()), atof(fields[9].c_str()));

        Node *stage_node = new Node(grid_x, grid_y, position, w, l, h);


        _nav_mesh[grid_y][grid_x] = stage_node;
      }
      else if(fields[0] == "") {
        // End of file reached at this point.
        nav_mesh_file.close();

        // Assign the neighbor nodes for each of the main nodes that just got
        // populated into the stage mesh.
        assign_neighbor_nodes(navmesh_filename);
      }
    }
  }
  else {
    cout<<"error opening navmesh.csv file!"<<endl;
  }
}

/**
 * This function assigns the neighbor nodes for each main node present in
 * _nav_mesh.
 */
void PathFind::assign_neighbor_nodes(const char* navmesh_filename){
  std::ifstream nav_mesh_file (navmesh_filename);

  // Stage variables.
  int gd_x, gd_y, gd_xn, gd_yn;
  string ln;
  string fields[10];
  string fields_n[10];

  if(nav_mesh_file.is_open()) {
    getline(nav_mesh_file, ln); // Get rid of grid size line.
    getline(nav_mesh_file, ln); // Get rid of the header.

    while(!nav_mesh_file.eof()) {
      getline(nav_mesh_file, ln); // Gets main node data only. No neighbor nodes.
      std::stringstream linestream (ln);
      for(int i = 0; i < 10; ++i) {
        getline(linestream, fields[i], ',');
      }
      if(fields[0] == "0" && fields[1] == "0") {
        // Usable main node.
        gd_x = atoi(fields[2].c_str());
        gd_y = atoi(fields[3].c_str());
        for(int i = 0; i < 8; ++i) {
          getline(nav_mesh_file, ln); // Gets neighbor node data only. No main nodes.
          std::stringstream linestream_n (ln);
          for(int j = 0; j < 10; ++j) {
            getline(linestream_n, fields_n[j], ',');
          }
          gd_xn = atoi(fields_n[2].c_str());
          gd_yn = atoi(fields_n[3].c_str());

          if(fields_n[0] == "0" && fields_n[1] == "1") {
            // Usable neighbor for main node.  TODO: The indices of the vector
            // are inverted when compared to the values of the nodes on actual
            // grid.  Fix this!
            _nav_mesh[gd_y][gd_x]->_neighbours[i] = _nav_mesh[gd_yn][gd_xn];
          }
          else if(fields_n[0] == "1" && fields_n[1] == "1") {
            // NULL neighbor.
            _nav_mesh[gd_y][gd_x]->_neighbours[i] = nullptr;
          }
          else {
            cout<<"Warning: Corrupt data!"<<endl;
          }
        }
      }
      else if(fields[0] == "") {
        // End of file reached at this point.
        nav_mesh_file.close();
      }
    }
  }
  else {
    cout<<"error opening navmesh.csv file!"<<endl;
  }
}

/**
 * This function starts the path finding process after reading the given
 * navigation mesh.
 */
void PathFind::set_path_find(const char* navmesh_filename) {
  create_nav_mesh(navmesh_filename);

  if(_ai_char->_steering->_path_follow_obj) {
    _ai_char->_steering->remove_ai("pathfollow");
  }

  _ai_char->_steering->path_follow(1.0f);

  if(_path_finder_obj) {
    delete _path_finder_obj;
    _path_finder_obj = nullptr;
  }

  _path_finder_obj = new PathFinder(_nav_mesh);
}

/**
 * This function checks for the source and target in the navigation mesh for
 * its availability and then finds the best path via the A* algorithm Then it
 * calls the path follower to make the object follow the path.
 */
void PathFind::path_find(LVecBase3 pos, string type) {
  if(type == "addPath") {
    if(_ai_char->_steering->_path_follow_obj) {
      _ai_char->_steering->remove_ai("pathfollow");
    }

    _ai_char->_steering->path_follow(1.0f);
  }

  clear_path();

  Node* src = find_in_mesh(_nav_mesh, _ai_char->_ai_char_np.get_pos(_ai_char->_window_render), _grid_size);

  if(src == nullptr) {
    cout<<"couldnt find source"<<endl;
  }

  Node* dst = find_in_mesh(_nav_mesh, pos, _grid_size);

  if(dst == nullptr) {
    cout<<"couldnt find destination"<<endl;
  }

  if(src != nullptr && dst != nullptr) {
    _path_finder_obj->find_path(src, dst);
    trace_path(src);
  }

  if(!_ai_char->_steering->_path_follow_obj->_start) {
    _ai_char->_steering->start_follow();
  }
}

/**
 * This function checks for the source and target in the navigation mesh for
 * its availability and then finds the best path via the A* algorithm Then it
 * calls the path follower to make the object follow the path.
 */
void PathFind::path_find(NodePath target, string type) {
  if(type == "addPath") {
    if(_ai_char->_steering->_path_follow_obj) {
      _ai_char->_steering->remove_ai("pathfollow");
    }

    _ai_char->_steering->path_follow(1.0f);
  }

  clear_path();

  _path_find_target = target;
  _prev_position = target.get_pos(_ai_char->_window_render);

  Node* src = find_in_mesh(_nav_mesh, _ai_char->_ai_char_np.get_pos(_ai_char->_window_render), _grid_size);

  if(src == nullptr) {
    cout<<"couldnt find source"<<endl;
  }

  Node* dst = find_in_mesh(_nav_mesh, _prev_position, _grid_size);

  if(dst == nullptr) {
    cout<<"couldnt find destination"<<endl;
  }

  if(src != nullptr && dst != nullptr) {
    _path_finder_obj->find_path(src, dst);
    trace_path(src);
  }

  if(_ai_char->_steering->_path_follow_obj!=nullptr) {
    if(!_ai_char->_steering->_path_follow_obj->_start) {
      _ai_char->_steering->start_follow("pathfind");
    }
  }
}

/**
 * Helper function to restore the path and mesh to its initial state
 */
void PathFind::clear_path() {
  // Initialize to zero
  for(int i = 0; i < _grid_size; ++i) {
    for(int j = 0; j < _grid_size; ++j) {
      if(_nav_mesh[i][j] != nullptr) {
        _nav_mesh[i][j]->_status = _nav_mesh[i][j]->neutral;
        _nav_mesh[i][j]->_cost = 0;
        _nav_mesh[i][j]->_heuristic = 0;
        _nav_mesh[i][j]->_score = 0;
        _nav_mesh[i][j]->_prv_node = nullptr;
      }
    }
  }

  if(_path_finder_obj) {
    _path_finder_obj->_open_list.clear();
    _path_finder_obj->_closed_list.clear();
  }
}

/**
 * This function is the function which sends the path information one by one
 * to the path follower so that it can store the path needed to be traversed
 * by the pathfinding object
 */
void PathFind::trace_path(Node* src) {
    if(_ai_char->_pf_guide) {
      _parent->remove_all_children();
    }
    else {
      _parent->remove_all_children();
    }

    if(_path_finder_obj->_closed_list.size() > 0) {
      Node *traversor = _path_finder_obj->_closed_list[_path_finder_obj->_closed_list.size() - 0.5];
      while(traversor != src) {
        if(_ai_char->_pf_guide) {
          _pen->move_to(traversor->_position.get_x(), traversor->_position.get_y(), 1);
          _pen->draw_to(traversor->_prv_node->_position.get_x(), traversor->_prv_node->_position.get_y(), 0.5);
          PT(GeomNode) gnode = _pen->create();
          _parent->add_child(gnode);
        }
        _ai_char->_steering->add_to_path(traversor->_position);
        traversor = traversor->_prv_node;
      }
    }
}

/**
 * This function allows the user to dynamically add obstacles to the game
 * environment.  The function will update the nodes within the bounding volume
 * of the obstacle as non-traversable.  Hence will not be considered by the
 * pathfinding algorithm.
 */
void PathFind::add_obstacle_to_mesh(NodePath obstacle) {
  PT(BoundingVolume) np_bounds = obstacle.get_bounds();
  CPT(BoundingSphere) np_sphere = np_bounds->as_bounding_sphere();

  Node* temp = find_in_mesh(_nav_mesh, obstacle.get_pos(), _grid_size);

  if(temp != nullptr) {
    float left = temp->_position.get_x() - np_sphere->get_radius();
    float right = temp->_position.get_x() + np_sphere->get_radius();
    float top = temp->_position.get_y() + np_sphere->get_radius();
    float down = temp->_position.get_y() - np_sphere->get_radius();

    for(int i = 0; i < _grid_size; ++i) {
        for(int j = 0; j < _grid_size; ++j) {
          if(_nav_mesh[i][j] != nullptr && _nav_mesh[i][j]->_type == true) {
            if(_nav_mesh[i][j]->_position.get_x() >= left && _nav_mesh[i][j]->_position.get_x() <= right &&
               _nav_mesh[i][j]->_position.get_y() >= down && _nav_mesh[i][j]->_position.get_y() <= top) {
              _nav_mesh[i][j]->_type = false;
              _previous_obstacles.insert(_previous_obstacles.end(), i);
              _previous_obstacles.insert(_previous_obstacles.end(), j);
            }
          }
        }
    }
  }
}

/**
 * This function does the updation of the collisions to the mesh based on the
 * new positions of the obstacles.
 */
void PathFind::do_dynamic_avoid() {
  clear_previous_obstacles();
  _previous_obstacles.clear();
  for(unsigned int i = 0; i < _dynamic_obstacle.size(); ++i) {
    add_obstacle_to_mesh(_dynamic_obstacle[i]);
  }
}

/**
 * Helper function to reset the collisions if the obstacle is not on the node
 * anymore
 */
void PathFind::clear_previous_obstacles(){
  for(unsigned int i = 0; i < _previous_obstacles.size(); i = i + 2) {
      _nav_mesh[_previous_obstacles[i]][_previous_obstacles[i + 1]]->_type = true;
  }
}

/**
 * This function starts the pathfinding obstacle navigation for the passed in
 * obstacle.
 */
void PathFind::dynamic_avoid(NodePath obstacle) {
  _dynamic_avoid = true;
  _dynamic_obstacle.insert(_dynamic_obstacle.end(), obstacle);
}
