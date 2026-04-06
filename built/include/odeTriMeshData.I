/**
 * PANDA 3D SOFTWARE
 * Copyright (c) Carnegie Mellon University.  All rights reserved.
 *
 * All use of this software is subject to the terms of the revised BSD
 * license.  You should have received a copy of this license along
 * with this source code in a file named "LICENSE."
 *
 * @file odeTriMeshData.I
 * @author joswilso
 * @date 2006-12-27
 */

INLINE OdeTriMeshData::TriMeshDataMap &OdeTriMeshData::
get_tri_mesh_data_map() {
  if (_tri_mesh_data_map == nullptr) {
    _tri_mesh_data_map = new TriMeshDataMap;
  }
  return *_tri_mesh_data_map;
}

INLINE dTriMeshDataID OdeTriMeshData::
get_id() const {
  return _id;
}

/*
INLINE void OdeTriMeshData::
set(int data_id, void* in_data) {
  dGeomTriMeshDataSet(_id, data_id, in_data);
}

INLINE void* OdeTriMeshData::
get(int data_id) {
  dGeomTriMeshDataGet(_id,data_id);
}
*/

INLINE void OdeTriMeshData::
build_single(const void* vertices, int vertex_stride, int vertex_count, const void* indices, int index_count, int tri_stride) {
  if (odetrimeshdata_cat.is_debug()) {
    odetrimeshdata_cat.debug() << "build_single(" << vertices << ", " << vertex_stride << ", " << vertex_count << ", " << indices << ", " << index_count << ", " << tri_stride << ")\n";
  }
  dGeomTriMeshDataBuildSingle(_id, vertices, vertex_stride, vertex_count, indices, index_count, tri_stride);
}

INLINE void OdeTriMeshData::
build_single1(const void* vertices, int vertex_stride, int vertex_count, const void* indices, int index_count, int tri_stride, const void* normals) {
  dGeomTriMeshDataBuildSingle1(_id, vertices, vertex_stride, vertex_count, indices, index_count, tri_stride, normals);
}

INLINE void OdeTriMeshData::
build_double(const void* vertices, int vertex_stride, int vertex_count, const void* indices, int index_count, int tri_stride) {
  dGeomTriMeshDataBuildDouble(_id, vertices, vertex_stride, vertex_count, indices, index_count, tri_stride);
}

INLINE void OdeTriMeshData::
build_double1(const void* vertices, int vertex_stride, int vertex_count, const void* indices, int index_count, int tri_stride, const void* normals) {
  dGeomTriMeshDataBuildDouble1(_id, vertices, vertex_stride, vertex_count, indices, index_count, tri_stride, normals);
}

/*
INLINE void OdeTriMeshData::
build_simple(const dReal* vertices, int vertex_count, const int* indices, int index_count) {
  dGeomTriMeshDataBuildSimple(_id, vertices, vertex_count, indices, index_count);
}
*/

/*
INLINE void OdeTriMeshData::
build_simple1(const dReal* vertices, int vertex_count, const int* indices, int index_count, const int* normals) {
  dGeomTriMeshDataBuildSimple1(_id, vertices, vertex_count, indices, index_count, normals);
}
*/

INLINE void OdeTriMeshData::
preprocess() {
  dGeomTriMeshDataPreprocess(_id);
}

/*
INLINE void OdeTriMeshData::
get_buffer(unsigned char** buf, int* buf_len) const {
  return dGeomTriMeshDataGetBuffer(_id, buf, buf_len);
}

INLINE void OdeTriMeshData::
set_buffer(unsigned char* buf) {
  dGeomTriMeshDataSetBuffer(_id, buf);
}

INLINE void OdeTriMeshData::
update() {
  dGeomTriMeshDataUpdate(_id);
}

*/
