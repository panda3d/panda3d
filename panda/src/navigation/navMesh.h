#ifndef NAVMESH_H
#define NAVMESH_H

#include "Recast.h"
#include "DetourNavMesh.h"
#include "pandaFramework.h"

class NavMesh
{
private:
  dtNavMesh *_nav_mesh;
  rcPolyMesh *_pmesh;
  rcPolyMeshDetail *_dmesh;
  
public:
  NavMesh();
  NavMesh(dtNavMesh *nav_mesh, rcPolyMesh *pmesh, rcPolyMeshDetail *dmesh);
  ~NavMesh();
  PT(GeomNode) draw_poly_mesh_geom();
  void set_nav_mesh(dtNavMesh *m) { _nav_mesh = m; }
  void set_poly_mesh(rcPolyMesh *p) { _pmesh = p; }
  void set_detail_poly_mesh(rcPolyMeshDetail *d) { _dmesh = d; }
  //has draw functions
};

#endif // NAVMESH_H
