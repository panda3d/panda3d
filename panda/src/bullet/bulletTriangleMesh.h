/**
 * PANDA 3D SOFTWARE
 * Copyright (c) Carnegie Mellon University.  All rights reserved.
 *
 * All use of this software is subject to the terms of the revised BSD
 * license.  You should have received a copy of this license along
 * with this source code in a file named "LICENSE."
 *
 * @file bulletTriangleMesh.h
 * @author enn0x
 * @date 2010-02-09
 */

#ifndef __BULLET_TRIANGLE_MESH_H__
#define __BULLET_TRIANGLE_MESH_H__

#include "pandabase.h"

#include "bullet_includes.h"
#include "bullet_utils.h"

#include "typedWritableReferenceCount.h"
#include "nodePath.h"
#include "luse.h"
#include "geom.h"
#include "pta_LVecBase3.h"
#include "pta_int.h"

/**
 *
 */
class EXPCL_PANDABULLET BulletTriangleMesh : public TypedWritableReferenceCount {
PUBLISHED:
  BulletTriangleMesh();
  ~BulletTriangleMesh() = default;

  void add_triangle(const LPoint3 &p0,
                    const LPoint3 &p1,
                    const LPoint3 &p2,
                    bool remove_duplicate_vertices=false);
  void add_array(const PTA_LVecBase3 &points,
                 const PTA_int &indices,
                 bool remove_duplicate_vertices=false);
  void add_geom(const Geom *geom,
                bool remove_duplicate_vertices=false,
                const TransformState *ts=TransformState::make_identity());

  void set_welding_distance(PN_stdfloat distance);
  void preallocate(int num_verts, int num_indices);

  size_t get_num_triangles() const;
  PN_stdfloat get_welding_distance() const;

  virtual void output(std::ostream &out) const;
  virtual void write(std::ostream &out, int indent_level) const;

public:
  size_t get_num_vertices() const;
  LPoint3 get_vertex(size_t index) const;

  LVecBase3i get_triangle(size_t index) const;

  size_t do_get_num_triangles() const;
  void do_add_triangle(const LPoint3 &p0,
                       const LPoint3 &p1,
                       const LPoint3 &p2,
                       bool remove_duplicate_vertices=false);

PUBLISHED:
  MAKE_PROPERTY(welding_distance, get_welding_distance, set_welding_distance);

  MAKE_SEQ_PROPERTY(vertices, get_num_vertices, get_vertex);
  MAKE_SEQ_PROPERTY(triangles, get_num_triangles, get_triangle);

public:
  INLINE btStridingMeshInterface *ptr() const;

private:
  unsigned int find_or_add_vertex(const LVecBase3 &p);

  btTriangleIndexVertexArray _mesh;
  btAlignedObjectArray<btVector3> _vertices;
  btAlignedObjectArray<unsigned int> _indices;
  PN_stdfloat _welding_distance;

public:
  static void register_with_read_factory();
  virtual void write_datagram(BamWriter *manager, Datagram &dg);

protected:
  static TypedWritable *make_from_bam(const FactoryParams &params);
  void fillin(DatagramIterator &scan, BamReader *manager);

public:
  static TypeHandle get_class_type() {
    return _type_handle;
  }
  static void init_type() {
    TypedWritableReferenceCount::init_type();
    register_type(_type_handle, "BulletTriangleMesh",
                  TypedWritableReferenceCount::get_class_type());
  }
  virtual TypeHandle get_type() const {
    return get_class_type();
  }
  virtual TypeHandle force_init_type() {
    init_type();
    return get_class_type();
  }

private:
  static TypeHandle _type_handle;
};

INLINE std::ostream &operator << (std::ostream &out, const BulletTriangleMesh &obj);

#include "bulletTriangleMesh.I"

#endif // __BULLET_TRIANGLE_MESH_H__
