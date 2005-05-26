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
#include "qpgeomEnums.h"
#include "qpgeomVertexArrayData.h"
#include "typedWritableReferenceCount.h"
#include "luse.h"
#include "updateSeq.h"
#include "pointerTo.h"
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
class EXPCL_PANDA qpGeomPrimitive : public TypedWritableReferenceCount, public qpGeomEnums {
protected:
  qpGeomPrimitive();

PUBLISHED:
  qpGeomPrimitive(UsageHint usage_hint);
  qpGeomPrimitive(const qpGeomPrimitive &copy);
  virtual ~qpGeomPrimitive();

  virtual PT(qpGeomPrimitive) make_copy() const=0;

  virtual PrimitiveType get_primitive_type() const=0;
  virtual int get_geom_rendering() const;

  INLINE ShadeModel get_shade_model() const;
  INLINE void set_shade_model(ShadeModel shade_model);

  INLINE UsageHint get_usage_hint() const;
  void set_usage_hint(UsageHint usage_hint);

  INLINE NumericType get_index_type() const;
  void set_index_type(NumericType index_type);

  // The following published methods are provided for safe, high-level
  // iteration through the vertices and sub-primitives within the
  // GeomPrimitive class.  These work correctly regardless of the
  // primitive type and without depending on knowledge about the way
  // primitives' lengths are encoded.  You can also safely build up a
  // composite primitive using these methods.

  INLINE bool is_composite() const;
  INLINE bool is_indexed() const;
  int get_first_vertex() const;
  INLINE int get_num_vertices() const;
  int get_vertex(int i) const;
  void add_vertex(int vertex);
  void add_consecutive_vertices(int start, int num_vertices);
  void add_next_vertices(int num_vertices);
  bool close_primitive();
  void clear_vertices();
  void offset_vertices(int offset);
  void make_nonindexed(qpGeomVertexData *dest, const qpGeomVertexData *source);
  void pack_vertices(qpGeomVertexData *dest, const qpGeomVertexData *source);
  void make_indexed();

  int get_num_primitives() const;
  int get_primitive_start(int n) const;
  int get_primitive_end(int n) const;
  int get_primitive_num_vertices(int n) const;

  INLINE int get_num_faces() const;
  INLINE int get_primitive_num_faces(int n) const;

  INLINE int get_min_vertex() const;
  int get_primitive_min_vertex(int n) const;
  INLINE int get_max_vertex() const;
  int get_primitive_max_vertex(int n) const;

  CPT(qpGeomPrimitive) decompose() const;
  CPT(qpGeomPrimitive) rotate() const;
  CPT(qpGeomPrimitive) match_shade_model(ShadeModel shade_model) const;

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

  INLINE const qpGeomVertexArrayData *get_vertices() const;
  qpGeomVertexArrayData *modify_vertices();
  void set_vertices(const qpGeomVertexArrayData *vertices);
  void set_nonindexed_vertices(int first_vertex, int num_vertices);

  INLINE int get_index_stride() const;
  INLINE CPTA_uchar get_data() const;

  INLINE CPTA_int get_ends() const;
  PTA_int modify_ends();
  void set_ends(CPTA_int ends);

  INLINE const qpGeomVertexArrayData *get_mins() const;
  INLINE const qpGeomVertexArrayData *get_maxs() const;

  virtual int get_num_vertices_per_primitive() const;
  virtual int get_min_num_vertices_per_primitive() const;
  virtual int get_num_unused_vertices_per_primitive() const;

  void prepare(PreparedGraphicsObjects *prepared_objects);

public:
  IndexBufferContext *prepare_now(PreparedGraphicsObjects *prepared_objects, 
                                  GraphicsStateGuardianBase *gsg);
  bool release(PreparedGraphicsObjects *prepared_objects);
  int release_all();

protected:
  INLINE CPT(qpGeomVertexArrayFormat) get_index_format() const;
  INLINE PT(qpGeomVertexArrayData) make_index_data() const;

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
  virtual CPT(qpGeomVertexArrayData) rotate_impl() const;
  virtual bool requires_unused_vertices() const;
  virtual void append_unused_vertices(qpGeomVertexArrayData *vertices, 
                                      int vertex);

private:
  // A GeomPrimitive keeps a list (actually, a map) of all the
  // PreparedGraphicsObjects tables that it has been prepared into.
  // Each PGO conversely keeps a list (a set) of all the Geoms that
  // have been prepared there.  When either destructs, it removes
  // itself from the other's list.
  typedef pmap<PreparedGraphicsObjects *, IndexBufferContext *> Contexts;
  Contexts _contexts;
    
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
    int _first_vertex;
    int _num_vertices;
    NumericType _index_type;
    UsageHint _usage_hint;
    PT(qpGeomVertexArrayData) _vertices;
    PTA_int _ends;
    PT(qpGeomVertexArrayData) _mins;
    PT(qpGeomVertexArrayData) _maxs;
    UpdateSeq _modified;

    bool _got_minmax;
    unsigned int _min_vertex;
    unsigned int _max_vertex;
  };

  PipelineCycler<CData> _cycler;
  typedef CycleDataReader<CData> CDReader;
  typedef CycleDataWriter<CData> CDWriter;

  void recompute_minmax(CDWriter &cdata);
  void do_make_indexed(CDWriter &cdata);

  static PStatCollector _decompose_pcollector;
  static PStatCollector _rotate_pcollector;

public:
  virtual void write_datagram(BamWriter *manager, Datagram &dg);

  virtual void finalize(BamReader *manager);

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

INLINE ostream &operator << (ostream &out, const qpGeomPrimitive &obj);

#include "qpgeomPrimitive.I"

#endif
