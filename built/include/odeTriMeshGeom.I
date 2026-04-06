/**
 * PANDA 3D SOFTWARE
 * Copyright (c) Carnegie Mellon University.  All rights reserved.
 *
 * All use of this software is subject to the terms of the revised BSD
 * license.  You should have received a copy of this license along
 * with this source code in a file named "LICENSE."
 *
 * @file odeTriMeshGeom.I
 * @author joswilso
 * @date 2006-12-27
 */

INLINE void OdeTriMeshGeom::
set_data(OdeTriMeshData &data) {
  odetrimeshdata_cat.warning()
    << "OdeTriMeshGeom::set_data() is deprecated, use OdeTriMeshGeom::set_tri_mesh_data() instead!\n";
  set_tri_mesh_data(data);
}

INLINE PT(OdeTriMeshData) OdeTriMeshGeom::
get_data() const {
  odetrimeshdata_cat.warning()
    << "OdeTriMeshGeom::get_data() is deprecated, use OdeTriMeshGeom::get_tri_mesh_data() instead!\n";
  return get_tri_mesh_data();
}

INLINE void OdeTriMeshGeom::
set_tri_mesh_data(OdeTriMeshData &data) {
  nassertv(_id != 0);
  dGeomTriMeshSetData(_id, data.get_id());
  OdeTriMeshData::link_data(_id, &data);
}

INLINE PT(OdeTriMeshData) OdeTriMeshGeom::
get_tri_mesh_data() const {
  nassertr(_id != 0, nullptr);
  return OdeTriMeshData::get_data(_id);
}

INLINE void OdeTriMeshGeom::
enable_TC(int geom_class, int enable){
  nassertv(_id != 0);
  dGeomTriMeshEnableTC(_id, geom_class, enable);
}

INLINE int OdeTriMeshGeom::
is_TC_enabled(int geom_class) const {
  nassertr(_id != 0, 0);
  return dGeomTriMeshIsTCEnabled(_id, geom_class);
}

INLINE void OdeTriMeshGeom::
clear_TC_cache(const OdeGeom &geom){
  nassertv(_id != 0);
  dGeomTriMeshClearTCCache(_id);
}

INLINE void OdeTriMeshGeom::
get_triangle(int face_index, LPoint3f &v0, LPoint3f &v1, LPoint3f &v2) const {
  nassertv(_id != 0);
  dVector3 dv0, dv1, dv2;
  dGeomTriMeshGetTriangle(_id, face_index, &dv0, &dv1, &dv2);

  v0.set(dv0[0], dv0[1], dv0[2]);
  v1.set(dv1[0], dv1[1], dv1[2]);
  v2.set(dv2[0], dv2[1], dv2[2]);
}

INLINE LPoint3f OdeTriMeshGeom::
get_point(int face_index, dReal u, dReal v) const {
  nassertr(_id != 0, LPoint3f(0));
  dVector3 out;
  dGeomTriMeshGetPoint(_id, face_index, u, v, out);
  return LPoint3f(out[0], out[1], out[2]);
}

INLINE int OdeTriMeshGeom::
get_num_triangles() const {
  nassertr(_id != 0, 0);
  return dGeomTriMeshGetTriangleCount(_id);
}

INLINE dTriMeshDataID OdeTriMeshGeom::
get_data_id() const {
  odetrimeshdata_cat.warning()
    << "OdeTriMeshGeom::get_data_id() is deprecated, use OdeTriMeshGeom::get_tri_mesh_data_id() instead!\n";
  return get_tri_mesh_data_id();
}

INLINE dTriMeshDataID OdeTriMeshGeom::
get_tri_mesh_data_id() const {
  nassertr(_id != 0, 0);
  return dGeomTriMeshGetTriMeshDataID(_id);
}
