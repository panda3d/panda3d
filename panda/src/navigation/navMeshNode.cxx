#include "navMeshNode.h"


NavMeshNode::NavMeshNode(std::string &name, NavMesh nav_mesh):
  PandaNode(name)
{
  _nav_mesh = nav_mesh;
}

NavMeshNode::~NavMeshNode() {
  
}


