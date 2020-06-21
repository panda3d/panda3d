#ifndef NAVMESHQUERY_H
#define NAVMESHQUERY_H

#include "DetourNavMeshQuery.h"
#include "navMeshBuilder.h"

class NavMeshQuery
{
private:
  class dtNavMeshQuery *_nav_query;
  
public:
  void set_nav_query(NavMeshBuilder *nav_mesh) { _nav_query = nav_mesh->get_nav_query(); }
  bool nearest_point(LPoint3 &p);
  
};

#endif // NAVMESHQUERY_H