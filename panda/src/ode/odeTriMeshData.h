// Filename: odeTriMeshData.h
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

////////////////////////////////////////////////////////////////////
//       Class : OdeTriMeshData
// Description : 
////////////////////////////////////////////////////////////////////
class EXPCL_PANDAODE OdeTriMeshData : public TypedReferenceCount {
PUBLISHED:
  OdeTriMeshData(const NodePath& model, bool use_normals = false);
  virtual ~OdeTriMeshData();

  enum DataType { DT_face_normals,
		  DT_last_transformation };
  // INLINE void set(int data_id, void* in_data);
  // INLINE void* get(int data_id);
  // INLINE void get_buffer(unsigned char** buf, int* buf_len) const;
  // INLINE void set_buffer(unsigned char* buf);
  // INLINE void update();
  virtual void write(ostream &out = cout, unsigned int indent=0) const;
  void write_faces(ostream &out) const;

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
  INLINE void build_simple(const dReal* vertices, int vertex_count, \
			   const int* indices, int index_count);
  INLINE void build_simple1(const dReal* vertices, int vertex_count, \
			    const int* indices, int index_count, \
			    const int* normals);
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

protected:
  struct StridedVertex{
    dReal Vertex[3];
  };  
  struct StridedTri{
    int Indices[3];
  };
  struct FaceNormal{
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
