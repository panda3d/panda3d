// Filename: qpgeom.h
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

#ifndef qpGEOM_H
#define qpGEOM_H

#include "pandabase.h"
#include "typedWritableReferenceCount.h"
#include "boundedObject.h"
#include "cycleData.h"
#include "cycleDataReader.h"
#include "cycleDataWriter.h"
#include "pipelineCycler.h"
#include "qpgeomVertexData.h"
#include "qpgeomPrimitive.h"
#include "qpgeomMunger.h"
#include "qpgeomUsageHint.h"
#include "qpgeomCacheEntry.h"
#include "updateSeq.h"
#include "pointerTo.h"
#include "geom.h"
#include "indirectLess.h"
#include "pset.h"

////////////////////////////////////////////////////////////////////
//       Class : qpGeom
// Description : A container for geometry primitives.  This class
//               associates one or more GeomPrimitive objects with a
//               table of vertices defined by a GeomVertexData object.
//               All of the primitives stored in a particular Geom are
//               drawn from the same set of vertices (each primitive
//               uses a subset of all of the vertices in the table),
//               and all of them must be rendered at the same time, in
//               the same graphics state.
//
//               This is part of the experimental Geom rewrite.
////////////////////////////////////////////////////////////////////
class EXPCL_PANDA qpGeom /* : public TypedWritableReferenceCount, public BoundedObject */
// We temporarily inherit from Geom, merely so we can store this
// pointer where a Geom should go, while we have both implementations
// in the codebase.  We pick up some additional cruft from Geom that
// we're not really using.
  : public Geom
{
PUBLISHED:
  qpGeom();
  qpGeom(const qpGeom &copy);
  void operator = (const qpGeom &copy);
  virtual ~qpGeom();

  // Temporary.
  virtual Geom *make_copy() const;

  INLINE qpGeomUsageHint::UsageHint get_usage_hint() const;

  INLINE CPT(qpGeomVertexData) get_vertex_data() const;
  PT(qpGeomVertexData) modify_vertex_data();
  void set_vertex_data(const qpGeomVertexData *data);

  INLINE int get_num_primitives() const;
  INLINE const qpGeomPrimitive *get_primitive(int i) const;
  INLINE qpGeomPrimitive *modify_primitive(int i);
  INLINE void set_primitive(int i, const qpGeomPrimitive *primitive);
  void add_primitive(const qpGeomPrimitive *primitive);
  void remove_primitive(int i);
  void clear_primitives();

  int get_num_bytes() const;
  INLINE UpdateSeq get_modified() const;

  void munge_geom(const qpGeomMunger *munger,
                  CPT(qpGeom) &result, CPT(qpGeomVertexData) &data) const;

  void output(ostream &out) const;
  void write(ostream &out, int indent_level = 0) const;

  void clear_cache();

public:
  void draw(GraphicsStateGuardianBase *gsg, 
            const qpGeomMunger *munger,
            const qpGeomVertexData *vertex_data) const;

  static UpdateSeq get_next_modified();

protected:
  virtual BoundingVolume *recompute_bound();

private:
  typedef pvector<PT(qpGeomPrimitive) > Primitives;

  // We have to use reference-counting pointers here instead of having
  // explicit cleanup in the GeomVertexFormat destructor, because the
  // cache needs to be stored in the CycleData, which makes accurate
  // cleanup more difficult.  We use the GeomVertexCacheManager class
  // to avoid cache bloat.
  class CacheEntry : public qpGeomCacheEntry {
  public:
    INLINE CacheEntry(const qpGeomMunger *modifier);
    INLINE bool operator < (const CacheEntry &other) const;

    virtual void evict_callback();
    virtual int get_result_size() const;
    virtual void output(ostream &out) const;

    qpGeom *_source;
    CPT(qpGeomMunger) _modifier;
    CPT(qpGeom) _geom_result;
    CPT(qpGeomVertexData) _data_result;
  };
  typedef pset<PT(CacheEntry), IndirectLess<CacheEntry> > Cache;

  // This is the data that must be cycled between pipeline stages.
  class EXPCL_PANDA CData : public CycleData {
  public:
    INLINE CData();
    INLINE CData(const CData &copy);
    virtual CycleData *make_copy() const;
    virtual void write_datagram(BamWriter *manager, Datagram &dg) const;
    virtual int complete_pointers(TypedWritable **plist, BamReader *manager);
    virtual void fillin(DatagramIterator &scan, BamReader *manager);

    PT(qpGeomVertexData) _data;
    Primitives _primitives;
    qpGeomUsageHint::UsageHint _usage_hint;
    bool _got_usage_hint;
    UpdateSeq _modified;
    Cache _cache;
  };

  PipelineCycler<CData> _cycler;
  typedef CycleDataReader<CData> CDReader;
  typedef CycleDataWriter<CData> CDWriter;

  void reset_usage_hint(CDWriter &cdata);

  static UpdateSeq _next_modified;

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
    /*TypedWritableReferenceCount::init_type();
      BoundedObject::init_type();*/
    Geom::init_type();
    register_type(_type_handle, "qpGeom",
                  Geom::get_class_type()
                  /*TypedWritableReferenceCount::get_class_type(),
                    BoundedObject::get_class_type()*/);
  }
  virtual TypeHandle get_type() const {
    return get_class_type();
  }
  virtual TypeHandle force_init_type() {init_type(); return get_class_type();}

private:
  static TypeHandle _type_handle;

  friend class CacheEntry;
};

INLINE ostream &operator << (ostream &out, const qpGeom &obj);

#include "qpgeom.I"

#endif
