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
#include "typedWritableReferenceCount.h"
#include "luse.h"
#include "pointerTo.h"
#include "pta_ushort.h"
#include "pta_int.h"
#include "pStatCollector.h"
#include "cycleData.h"
#include "cycleDataReader.h"
#include "cycleDataWriter.h"
#include "pipelineCycler.h"

class qpGeomVertexData;
class GraphicsStateGuardianBase;
class GeomContext;
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
  qpGeomPrimitive();
  qpGeomPrimitive(const qpGeomPrimitive &copy);
  virtual ~qpGeomPrimitive();

  virtual PT(qpGeomPrimitive) make_copy() const=0;

  enum ShadeModel {
    SM_smooth,
    SM_uniform,
    SM_flat_first_vertex,
    SM_flat_last_vertex,
  };

  INLINE ShadeModel get_shade_model() const;
  INLINE void set_shade_model(ShadeModel shade_model);

  INLINE int get_num_vertices() const;
  INLINE int get_vertex(int i) const;
  void add_vertex(int vertex);
  void add_consecutive_vertices(int start, int num_vertices);
  void close_primitive();
  void clear_vertices();

  INLINE CPTA_ushort get_vertices() const;
  INLINE CPTA_ushort get_flat_first_vertices() const;
  INLINE CPTA_ushort get_flat_last_vertices() const;
  PTA_ushort modify_vertices();
  void set_vertices(PTA_ushort vertices);

  INLINE CPTA_int get_ends() const;
  PTA_int modify_ends();
  void set_ends(PTA_int ends);

  int get_num_bytes() const;

  INLINE int get_min_vertex() const;
  INLINE int get_max_vertex() const;

  virtual int get_num_vertices_per_primitive() const;
  virtual int get_min_num_vertices_per_primitive() const;
  int get_num_primitives() const;
  int get_primitive_start(int i) const;
  INLINE int get_primitive_end(int i) const;
  int get_primitive_num_vertices(int i) const;

  CPT(qpGeomPrimitive) decompose() const;

  virtual void output(ostream &out) const;
  virtual void write(ostream &out, int indent_level) const;

  void clear_cache();

public:
  virtual void draw(GraphicsStateGuardianBase *gsg) const=0;

  virtual void calc_tight_bounds(LPoint3f &min_point, LPoint3f &max_point,
                                 bool &found_any, 
                                 const qpGeomVertexData *vertex_data) const;

protected:
  virtual CPT(qpGeomPrimitive) decompose_impl() const;
  virtual CPTA_ushort rotate_impl() const;

private: 
  void do_rotate();
  void remove_cache_entry() const;

protected:
  static PStatCollector _rotate_pcollector;

private:
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

    bool _got_minmax;
    unsigned short _min_vertex;
    unsigned short _max_vertex;

    CPT(qpGeomPrimitive) _decomposed;
  };

  PipelineCycler<CData> _cycler;
  typedef CycleDataReader<CData> CDReader;
  typedef CycleDataWriter<CData> CDWriter;

  void recompute_minmax();

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
  friend class qpGeomVertexCacheManager;
};

#include "qpgeomPrimitive.I"

#endif
