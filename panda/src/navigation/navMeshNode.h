#ifndef NAVMESHNODE_H
#define NAVMESHNODE_H

#include <string>
#include "geom.h"
#include "pandaFramework.h"
#include "pandaSystem.h"
#include "navMesh.h"
#include <string>

class NavMeshNode: public PandaNode
{
private:
  NavMesh _nav_mesh;

public:
  NavMeshNode(std::string &name, NavMesh nav_mesh);
  ~NavMeshNode();
};

#endif // NAVMESHNODE_H