/**
 * PANDA 3D SOFTWARE
 * Copyright (c) Carnegie Mellon University.  All rights reserved.
 *
 * All use of this software is subject to the terms of the revised BSD
 * license.  You should have received a copy of this license along
 * with this source code in a file named "LICENSE."
 *
 * @file odeTriMeshData.h
 * @author joswilso
 * @date 2006-12-27
 */

#ifndef ODETRIMESHDATA_H
#define ODETRIMESHDATA_H

#include "pandabase.h"
#include "typedReferenceCount.h"
#include "luse.h"

#include "ode_includes.h"

#include "nodePathCollection.h"
#include "geomNode.h"
#include "geomVertexData.h"
#include "geomVertexReader.h"
#include "geomTriangles.h"
#include "geomTristrips.h"

#include "config_ode.h"

/**
 *
 */
class EXPCL_PANDAODE OdeTriMeshData : public TypedReferenceCount {
public:
  static void link_data(dGeomID id, PT(OdeTriMeshData) data);
  static PT(OdeTriMeshData) get_data(dGeomID id);
  static void unlink_data(dGeomID id);
  static void remove_data(OdeTriMeshData *data);
  static void print_data(const std::string &marker);

private:
  typedef pmap<dGeomID, PT(OdeTriMeshData)> TriMeshDataMap;
  static TriMeshDataMap *_tri_mesh_data_map;
  static INLINE TriMeshDataMap &get_tri_mesh_data_map();

PUBLISHED:

  enum DataType { DT_face_normals = 0,
                  DT_last_transformation };

  OdeTriMeshData(const NodePath& model, bool use_normals = false);
  virtual ~OdeTriMeshData();

  void destroy();

  // INLINE void set(int data_id, void* in_data); INLINE void* get(int
  // data_id); INLINE void get_buffer(unsigned char** buf, int* buf_len)
  // const; INLINE void set_buffer(unsigned char* buf); INLINE void update();

  virtual void write(std::ostream &out = std::cout, unsigned int indent=0) const;
  void write_faces(std::ostream &out) const;

public:
  INLINE void build_single(const void* vertices, int vertex_stride, int vertex_count, \
                           const void* indices, int index_count, int tri_stride);
  INLINE void build_single1(const void* vertices, int vertex_stride, int vertex_count, \
                            const void* indices, int index_count, int tri_stride, \
                            const void* normals);
  INLINE void build_double(const void* vertices, int vertex_stride, int vertex_count, \
                           const void* indices, int index_count, int tri_stride);
  INLINE void build_double1(const void* vertices, int vertex_stride, int vertex_count, \
                            const void* indices, int index_count, int tri_stride, \
                            const void* normals);

  // Temporarily commenting these two out--ODE had an API change from (int
  // *indices) to (dTriIndex *indices).  But since there's no #define that
  // indicates the ODE version, we don't have any way to automatically put the
  // right symbol in here.  However, we're not using these methods right now
  // anyway.

  /*
  INLINE void build_simple(const dReal* vertices, int vertex_count, \
                           const int* indices, int index_count);
  INLINE void build_simple1(const dReal* vertices, int vertex_count, \
                            const int* indices, int index_count, \
                            const int* normals);
  */

  INLINE void preprocess();

  INLINE dTriMeshDataID get_id() const;

private:
  void process_model(const NodePath& model, bool &use_normals);
  void process_geom_node(const GeomNode *geomNode);
  void process_geom(const Geom *geom);
  void process_primitive(const GeomPrimitive *primitive,
                         CPT(GeomVertexData) vData);
  void analyze(const GeomNode *geomNode);
  void analyze(const Geom *geom);
  void analyze(const GeomPrimitive *geom);

  OdeTriMeshData(const OdeTriMeshData &other);
  void operator = (const OdeTriMeshData &other);

protected:
  struct StridedVertex {
    dReal Vertex[3];
  };
  struct StridedTri {
    int Indices[3];
  };
  struct FaceNormal {
    dVector3 Normal;
  };

  dTriMeshDataID _id;
  StridedVertex *_vertices;
  StridedTri *_faces;
  FaceNormal *_normals;

  unsigned int _num_vertices;
  unsigned int _num_faces;

public:
  static TypeHandle get_class_type() {
    return _type_handle;
  }
  static void init_type() {
    TypedReferenceCount::init_type();
    register_type(_type_handle, "OdeTriMeshData",
                  TypedReferenceCount::get_class_type());
  }
  virtual TypeHandle get_type() const {
    return get_class_type();
  }
  virtual TypeHandle force_init_type() {init_type(); return get_class_type();}

private:
  static TypeHandle _type_handle;
};

#include "odeTriMeshData.I"

#endif
