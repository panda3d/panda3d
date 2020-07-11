/**
 * PANDA 3D SOFTWARE
 * Copyright (c) Carnegie Mellon University.  All rights reserved.
 *
 * All use of this software is subject to the terms of the revised BSD
 * license.  You should have received a copy of this license along
 * with this source code in a file named "LICENSE."
 *
 * @file navMesh.cxx
 * @author ashwini
 * @date 2020-060-21
 */


#include "navMesh.h"
#include "geom.h"
#include "geomTrifans.h"
#include <iostream>

TypeHandle NavMesh::_type_handle;

NavMesh::NavMesh():
  _nav_mesh(0) {}

NavMesh::~NavMesh() {
  dtFreeNavMesh(_nav_mesh);
  _nav_mesh = 0;
}

NavMesh::NavMesh(dtNavMesh *nav_mesh) {
  _nav_mesh = nav_mesh;
}

bool NavMesh::init_nav_mesh() {
	unsigned char *nav_data = 0;
  int nav_data_size = 0;

  if (!dtCreateNavMeshData(&_params, &nav_data, &nav_data_size)) {
    std::cout<<"\nCould not build Detour navmesh.\n";
    return false;
  }

  _nav_mesh = dtAllocNavMesh();

  if (!_nav_mesh) {
    dtFree(nav_data);
    std::cout<<"\nCould not create Detour navmesh\n";
    return false;
  }

  dtStatus status;

  status = _nav_mesh->init(nav_data, nav_data_size, DT_TILE_FREE_DATA);
  if (dtStatusFailed(status)) {
    dtFree(nav_data);
    std::cout<<"\nCould not init Detour navmesh\n";
    return false;
  }
}

/**
 * Tells the BamReader how to create objects of type NavMesh.
 */
void NavMesh::
register_with_read_factory() {
  std::cout<<"\nCalled NavMesh::register_with_read_factory()\n";
  BamReader::get_factory()->register_factory(get_class_type(), make_from_bam);
}

/**
 * Writes the contents of this object to the datagram for shipping out to a
 * Bam file.
 */
void NavMesh::
write_datagram(BamWriter *manager, Datagram &dg) {
  //NavMesh::write_datagram(manager, dg);
  
  dg.add_int32(_params.vertCount);
  dg.add_int32(_params.polyCount);
  dg.add_int32(_params.nvp);
  dg.add_int32(_params.detailVertsCount);
  dg.add_int32(_params.detailTriCount);
  dg.add_float32(_params.walkableHeight);
  dg.add_float32(_params.walkableRadius);
  dg.add_float32(_params.walkableClimb);
  dg.add_float32(_params.cs);
  dg.add_float32(_params.ch);
  dg.add_bool(_params.buildBvTree);

  //the bmin and bmax values
  for(int i=0;i<3;i++) {
    dg.add_float32(_params.bmin[i]);
  }
  for(int i=0;i<3;i++) {
    dg.add_float32(_params.bmax[i]);
  }
  
  //POLYGON MESH ATTRIBUTES

  //the vertices
  for(int i=0 ; i < (_params.vertCount) * 3 ; i++) {
    dg.add_uint16(_params.verts[i]);
  }

  //the polygon data
  for(int i=0 ; i < (_params.polyCount) * 2 * (_params.nvp) ;i++) {
    dg.add_uint16(_params.polys[i]);
  }

  //the polygon flags
  for(int i=0 ; i < _params.polyCount ;i++) {
    dg.add_uint16(_params.polyFlags[i]);
  }

  //the polygon area IDs
  for(int i=0 ; i < _params.polyCount ;i++) {
    dg.add_uint8(_params.polyAreas[i]);           
  }
  
  //POLYGON MESH DETAIL ATTRIBUTES

  //height detail sub-mesh data
  for(int i=0 ; i < (_params.polyCount) * 4 ;i++) {
    dg.add_uint32(_params.detailMeshes[i]);
  }

  //detail mesh vertices
  for(int i=0 ; i < (_params.detailVertsCount) * 3 ;i++) {
    dg.add_float32(_params.detailVerts[i]);
  }

  //detail mesh vertices
  for(int i=0 ; i < (_params.detailTriCount) * 4 ;i++) {
    dg.add_uint8(_params.detailTris[i]);
  }

}

/**
 * This function is called by the BamReader's factory when a new object of
 * type NavMesh is encountered in the Bam file.  It should create the
 * NavMesh and extract its information from the file.
 */
TypedWritable *NavMesh::
make_from_bam(const FactoryParams &params) {
  NavMesh *param = new NavMesh;
  DatagramIterator scan;
  BamReader *manager;

  parse_params(params, scan, manager);
  param->fillin(scan, manager);

  return param;
}

/**
 * This internal function is called by make_from_bam to read in all of the
 * relevant data from the BamFile for the new NavMesh.
 */
void NavMesh::
fillin(DatagramIterator &scan, BamReader *manager) {
  //NavMesh::fillin(scan, manager);
  
  int vert_count = scan.get_int32();
  int poly_count = scan.get_int32();
  int nvp = scan.get_int32();
  int detail_vert_count = scan.get_int32();
  int detail_tri_count = scan.get_int32();
  float walkable_height = scan.get_float32();
  float walkable_radius = scan.get_float32();
  float walkable_climb = scan.get_float32();
  float cs = scan.get_float32();
  float ch = scan.get_float32();
  bool build_bv_tree = scan.get_bool();

  float b_min[3];
  for(int i=0;i<3;i++) {
    b_min[i] = scan.get_float32();
  }
  float b_max[3];
  for(int i=0;i<3;i++) {
    b_max[i] = scan.get_float32();
  }

  //POLYGON MESH ATTRIBUTES

  unsigned short* verts = new unsigned short[3 * vert_count];
  for(int i=0 ; i < 3 * vert_count ; i++) {
    verts[i] = scan.get_uint16();
  }

  unsigned short* polys = new unsigned short[poly_count * 2 * nvp];
  for(int i=0 ; i < poly_count * 2 * nvp ; i++) {
    polys[i] = scan.get_uint16();
  }

  unsigned short* poly_flags = new unsigned short[poly_count];
  for(int i=0 ; i < poly_count; i++) {
    poly_flags[i] = scan.get_uint16();
  }

  unsigned char* poly_areas = new unsigned char[poly_count];
  for(int i=0 ; i < poly_count; i++) {
    poly_areas[i] = scan.get_uint8();
  }

  //POLYGON MESH DETAIL ATTRIBUTES

  unsigned int* detail_meshes = new unsigned int[poly_count * 4];
  for(int i=0 ; i < poly_count * 4 ;i++) {
    detail_meshes[i] = scan.get_uint32();
  }

  float* detail_verts = new float[detail_vert_count * 3];
  for(int i=0 ; i < detail_vert_count * 3 ;i++) {
    detail_verts[i] = scan.get_float32();
  }

  unsigned char* detail_tris = new unsigned char[detail_tri_count * 4];
  for(int i=0 ; i < detail_tri_count * 4 ;i++) {
    detail_tris[i] = scan.get_uint8();
  }

  memset(&(_params), 0, sizeof(_params));

  _params.verts = verts;
  _params.vertCount = vert_count;
  _params.polys = polys;
  _params.polyAreas = poly_areas;
  _params.polyFlags = poly_flags;
  _params.polyCount = poly_count;
  _params.nvp = nvp;
  _params.detailMeshes = detail_meshes;
  _params.detailVerts = detail_verts;
  _params.detailVertsCount = detail_vert_count;
  _params.detailTris = detail_tris;
  _params.detailTriCount = detail_tri_count;


  _params.walkableHeight = walkable_height;
  _params.walkableRadius = walkable_radius;
  _params.walkableClimb = walkable_climb;

  _params.bmin[0] = b_min[0];
  _params.bmin[1] = b_min[1];
  _params.bmin[2] = b_min[2];
  _params.bmax[0] = b_max[0];
  _params.bmax[1] = b_max[1];
  _params.bmax[2] = b_max[2];
  
  _params.cs = cs;
  _params.ch = ch;
  _params.buildBvTree = build_bv_tree;

  init_nav_mesh();


}
