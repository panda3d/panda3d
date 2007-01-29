// Filename: odeTriMeshGeom.I
// Created by:  joswilso (27Dec06)
//
////////////////////////////////////////////////////////////////////
//
// PANDA 3D SOFTWARE
// Copyright (c) 2001 - 2004, Disney Enterprises, Inc.  All rights reserved
//
// All use of this software is subject to the terms of the Panda 3d
// Software license.  You should have received a copy of this license
// along with this source code; you will also find a current copy of
// the license at http://etc.cmu.edu/panda3d/docs/license/ .
//
// To contact the maintainers of this program write to
// panda3d-general@lists.sourceforge.net .
//
////////////////////////////////////////////////////////////////////

INLINE void OdeTriMeshGeom::
set_data(OdeTriMeshData &data) {
  unlink_data(_id);
  dGeomTriMeshSetData(_id, data.get_id());
  link_data(_id, data);
}

INLINE PT(OdeTriMeshData) OdeTriMeshGeom::
get_data() const {
  return MeshData[_id];
}

INLINE void OdeTriMeshGeom::
enable_TC(int geom_class, int enable){
  dGeomTriMeshEnableTC(_id, geom_class, enable);
}

INLINE int OdeTriMeshGeom::
is_TC_enabled(int geom_class) const {
  return dGeomTriMeshIsTCEnabled(_id, geom_class);
}

INLINE void OdeTriMeshGeom::
clear_TC_cache(const OdeGeom &geom){
  dGeomTriMeshClearTCCache(_id);
}

INLINE void OdeTriMeshGeom::
get_triangle(int face_index, LPoint3f &v0, LPoint3f &v1, LPoint3f &v2) const {
  dVector3 dv0, dv1, dv2;
  dGeomTriMeshGetTriangle(_id, face_index, &dv0, &dv1, &dv2);
  
  v0.set(dv0[0], dv0[1], dv0[2]);
  v1.set(dv1[0], dv1[1], dv1[2]);
  v2.set(dv2[0], dv2[1], dv2[2]);
}

INLINE LPoint3f OdeTriMeshGeom::
get_point(int face_index, dReal u, dReal v) const {
  dVector3 out;
  dGeomTriMeshGetPoint(_id, face_index, u, v, out);
  return LPoint3f(out[0], out[1], out[2]);
}

INLINE int OdeTriMeshGeom::
get_num_triangles() const {
  return dGeomTriMeshGetTriangleCount(_id);
}

INLINE dTriMeshDataID OdeTriMeshGeom::
get_data_id() const {
  return dGeomTriMeshGetTriMeshDataID(_id);
}