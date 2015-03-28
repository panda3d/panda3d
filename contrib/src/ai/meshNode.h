
#ifndef _MESHNODE_H
#define _MESHNODE_H

#include "aiGlobals.h"

////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//
// Class : Node
//  Description : This class is used to assign the nodes on the mesh. It holds all the data necessary to
//               compute A* algorithm. It also maintains a lot of vital information such as the neighbor
//               nodes of each node and also its position on the mesh.
// Note: The Mesh Generator which is a stand alone tool makes use of this class to generate the nodes on the
//       mesh.

////////////////////////////////////////////////////////////////////////////////////////////////////////////////

class EXPCL_PANDAAI Node {
public:
    // This variable specifies whether the node is an obtacle or not.
    // Used for dynamic obstacle addition to the environment.
    // obstacle = false
    // navigational = true
    bool _type;

    // This variable specifies the node status whether open, close or neutral.
    // open = belongs to _open_list.
    // close = belongs to _closed_list.
    // neutral = unexamined node.
    enum Status {
      open,
      close,
      neutral
    };
    Status _status;

    // The score is used to compute the traversal expense to nodes when using A*.
    // _score = _cost + heuristic
    int _score;
    int _cost;
    int _heuristic;

    // Used to trace back the path after it is generated using A*.
    Node *_prv_node;

    // Position of the node in the 2d grid.
    int _grid_x, _grid_y;

    // Position of the node in 3D space.
    LVecBase3 _position;

    // Dimensions of each face / cell on the mesh.
    // Height is given in case of expansion to a 3d mesh. Currently not used.
    float _width, _length ,_height;
    Node *_neighbours[8]; // anti-clockwise from top left corner.

    // The _next pointer is used for traversal during mesh generation from the model.
    // Note: The data in this member is discarded when mesh data is written into navmesh.csv file.
    Node *_next;

    Node(int grid_x, int grid_y, LVecBase3 pos, float w, float l, float h);
    ~Node();

    bool contains(float x, float y);
};

#endif
