// Filename: qpgeomPrimitive.h
// Created by:  drose (06Mar05)
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

#ifndef qpGEOMPRIMITIVE_H
#define qpGEOMPRIMITIVE_H

#include "pandabase.h"
#include "qpgeomUsageHint.h"
#include "qpgeomCacheEntry.h"
#include "typedWritableReferenceCount.h"
#include "luse.h"
#include "updateSeq.h"
#include "pointerTo.h"
#include "pta_ushort.h"
#include "pta_int.h"
#include "pStatCollector.h"
#include "cycleData.h"
#include "cycleDataReader.h"
#include "cycleDataWriter.h"
#include "pipelineCycler.h"

class qpGeomVertexData;
class PreparedGraphicsObjects;
class IndexBufferContext;
class GraphicsStateGuardianBase;
class FactoryParams;

////////////////////////////////////////////////////////////////////
//       Class : qpGeomPrimitive
// Description : This is an abstract base class for a family of
//               classes that represent the fundamental geometry
//               primitives that may be stored in a Geom.
//
//               They all have in common the fact that they are
//               defined by tables of vertex data stored in a
//               GeomVertexData object.  Each GeomPrimitive object
//               contains an ordered list of integers, which index
//               into the vertex array defined by the GeomVertexData
//               and define the particular vertices of the
//               GeomVertexData that are used for this primitive.
//
//               The meaning of a given arrangement of vertices is
//               defined by each individual primitive type; for
//               instance, a GeomTriangle renders a triangle from each
//               three consecutive vertices, while a GeomTriangleStrip
//               renders a strip of (n - 2) connected triangles from
//               each sequence of n vertices.
//
//               This is part of the experimental Geom rewrite.
////////////////////////////////////////////////////////////////////
class EXPCL_PANDA qpGeomPrimitive : public TypedWritableReferenceCount {
PUBLISHED:
  qpGeomPrimitive(qpGeomUsageHint::UsageHint usage_hint);
  qpGeomPrimitive(const qpGeomPrimitive &copy);
  virtual ~qpGeomPrimitive();

  virtual PT(qpGeomPrimitive) make_copy() const=0;

  enum ShadeModel {
    // SM_smooth: vertices within a single face have different
    // colors/normals that should be smoothed across the face.  This
    // primitive should be rendered with SmoothModelAttrib::M_smooth.
    SM_smooth,  

    // SM_uniform: all vertices across all faces have the same colors
    // and normals.  It doesn't really matter which ShadeModelAttrib
    // mode is used to render this primitive.
    SM_uniform, 

    // SM_flat_(first,last)_vertex: each face within the primitive
    // might have a different color/normal than the other faces, but
    // across a particular face there is only one color/normal.  Each
    // face's color/normal is taken from the (first, last) vertex of
    // the face.  This primitive should be rendered with
    // SmoothModelAttrib::M_flat.
    SM_flat_first_vertex,
    SM_flat_last_vertex,
  };

  enum PrimitiveType {
    PT_none,
    PT_polygons,
    PT_lines,
    PT_points
  };

  virtual PrimitiveType get_primitive_type() const=0;
  INLINE qpGeomUsageHint::UsageHint get_usage_hint() const;

  INLINE ShadeModel get_shade_model() const;
  INLINE void set_shade_model(ShadeModel shade_model);

  // The following published methods are provided for safe, high-level
  // iteration through the vertices and sub-primitives within the
  // GeomPrimitive class.  These work correctly regardless of the
  // primitive type and without depending on knowledge about the way
  // primitives' lengths are encoded.  You can also safely build up a
  // composite primitive using these methods.

  INLINE int get_num_vertices() const;
  INLINE int get_vertex(int i) const;
  void add_vertex(int vertex);
  void add_consecutive_vertices(int start, int num_vertices);
  void add_next_vertices(int num_vertices);
  bool close_primitive();
  void clear_vertices();

  int get_num_primitives() const;
  int get_primitive_start(int n) const;
  int get_primitive_end(int n) const;
  int get_primitive_num_vertices(int n) const;

  INLINE int get_num_faces() const;
  INLINE int get_primitive_num_faces(int n) const;

  INLINE int get_min_vertex() const;
  INLINE int get_primitive_min_vertex(int n) const;
  INLINE int get_max_vertex() const;
  INLINE int get_primitive_max_vertex(int n) const;

  CPT(qpGeomPrimitive) decompose() const;

  int get_num_bytes() const;
  INLINE int get_data_size_bytes() const;
  INLINE UpdateSeq get_modified() const;

  bool check_valid(const qpGeomVertexData *vertex_data) const;

  virtual void output(ostream &out) const;
  virtual void write(ostream &out, int indent_level) const;

public:
  // These public methods are not intended for high-level usage.  They
  // are public so that C++ code that absolutely needs fast access to
  // the primitive data can get to it, but using them requires
  // knowledge about how the component primitives are encoded within
  // the GeomPrimitive class, and it's easy to screw something up.
  // Also, if too many code samples depend on this internal knowledge,
  // it may make it difficult to extend this class later.  It is
  // recommended that application-level code use the above interfaces
  // instead.

  INLINE CPTA_ushort get_vertices() const;
  INLINE CPTA_ushort get_flat_first_vertices() const;
  INLINE CPTA_ushort get_flat_last_vertices() const;
  PTA_ushort modify_vertices();
  void set_vertices(CPTA_ushort vertices);

  INLINE CPTA_int get_ends() const;
  PTA_int modify_ends();
  void set_ends(CPTA_int ends);

  INLINE CPTA_ushort get_mins() const;
  INLINE CPTA_ushort get_maxs() const;

  virtual int get_num_vertices_per_primitive() const;
  virtual int get_min_num_vertices_per_primitive() const;
  virtual int get_num_unused_vertices_per_primitive() const;

  void clear_cache();
  void prepare(PreparedGraphicsObjects *prepared_objects);

public:
  IndexBufferContext *prepare_now(PreparedGraphicsObjects *prepared_objects, 
                                  GraphicsStateGuardianBase *gsg);
  bool release(PreparedGraphicsObjects *prepared_objects);
  int release_all();

private:
  void clear_prepared(PreparedGraphicsObjects *prepared_objects);

public:
  virtual void draw(GraphicsStateGuardianBase *gsg) const=0;

  void calc_tight_bounds(LPoint3f &min_point, LPoint3f &max_point,
                         bool &found_any, 
                         const qpGeomVertexData *vertex_data,
                         bool got_mat, const LMatrix4f &mat) const;

protected:
  virtual CPT(qpGeomPrimitive) decompose_impl() const;
  virtual CPTA_ushort rotate_impl() const;
  virtual void append_unused_vertices(PTA_ushort &vertices, int vertex);

protected:
  static PStatCollector _rotate_pcollector;

private:
  qpGeomUsageHint::UsageHint _usage_hint;

  // A GeomPrimitive keeps a list (actually, a map) of all the
  // PreparedGraphicsObjects tables that it has been prepared into.
  // Each PGO conversely keeps a list (a set) of all the Geoms that
  // have been prepared there.  When either destructs, it removes
  // itself from the other's list.
  typedef pmap<PreparedGraphicsObjects *, IndexBufferContext *> Contexts;
  Contexts _contexts;

  class CacheEntry : public qpGeomCacheEntry {
  public:
    virtual void evict_callback();
    virtual int get_result_size() const;
    virtual void output(ostream &out) const;

    qpGeomPrimitive *_source;
    CPT(qpGeomPrimitive) _decomposed;
  };
    
  // This is the data that must be cycled between pipeline stages.
  class EXPCL_PANDA CData : public CycleData {
  public:
    INLINE CData();
    INLINE CData(const CData &copy);
    virtual CycleData *make_copy() const;
    virtual void write_datagram(BamWriter *manager, Datagram &dg) const;
    virtual int complete_pointers(TypedWritable **plist, BamReader *manager);
    virtual void fillin(DatagramIterator &scan, BamReader *manager);

    ShadeModel _shade_model;
    PTA_ushort _vertices;
    CPTA_ushort _rotated_vertices;
    PTA_int _ends;
    PTA_ushort _mins;
    PTA_ushort _maxs;
    UpdateSeq _modified;

    bool _got_minmax;
    unsigned short _min_vertex;
    unsigned short _max_vertex;

    PT(CacheEntry) _cache;
  };

  PipelineCycler<CData> _cycler;
  typedef CycleDataReader<CData> CDReader;
  typedef CycleDataWriter<CData> CDWriter;

  CPTA_ushort do_rotate(CDReader &cdata);
  void recompute_minmax(CDWriter &cdata);

public:
  virtual void write_datagram(BamWriter *manager, Datagram &dg);

protected:
  void fillin(DatagramIterator &scan, BamReader *manager);

public:
  static TypeHandle get_class_type() {
    return _type_handle;
  }
  static void init_type() {
    TypedWritableReferenceCount::init_type();
    register_type(_type_handle, "qpGeomPrimitive",
                  TypedWritableReferenceCount::get_class_type());
  }
  virtual TypeHandle get_type() const {
    return get_class_type();
  }
  virtual TypeHandle force_init_type() {init_type(); return get_class_type();}

private:
  static TypeHandle _type_handle;

  friend class qpGeom;
  friend class PreparedGraphicsObjects;
};

#include "qpgeomPrimitive.I"

#endif
