// Filename: geom.h
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

#ifndef GEOM_H
#define GEOM_H

#include "pandabase.h"
#include "typedWritableReferenceCount.h"
#include "boundedObject.h"
#include "cycleData.h"
#include "cycleDataReader.h"
#include "cycleDataWriter.h"
#include "pipelineCycler.h"
#include "geomVertexData.h"
#include "geomPrimitive.h"
#include "geomMunger.h"
#include "geomEnums.h"
#include "geomCacheEntry.h"
#include "textureStage.h"
#include "updateSeq.h"
#include "pointerTo.h"
#include "indirectLess.h"
#include "pset.h"

class GeomContext;
class PreparedGraphicsObjects;

////////////////////////////////////////////////////////////////////
//       Class : Geom
// Description : A container for geometry primitives.  This class
//               associates one or more GeomPrimitive objects with a
//               table of vertices defined by a GeomVertexData object.
//               All of the primitives stored in a particular Geom are
//               drawn from the same set of vertices (each primitive
//               uses a subset of all of the vertices in the table),
//               and all of them must be rendered at the same time, in
//               the same graphics state.
////////////////////////////////////////////////////////////////////
class EXPCL_PANDA Geom : public TypedWritableReferenceCount, public BoundedObject, public GeomEnums {
PUBLISHED:
  Geom(const GeomVertexData *data);
protected:
  Geom(const Geom &copy);
PUBLISHED:
  void operator = (const Geom &copy);
  virtual ~Geom();

  virtual Geom *make_copy() const;

  INLINE PrimitiveType get_primitive_type() const;
  INLINE ShadeModel get_shade_model() const;
  INLINE int get_geom_rendering() const;

  INLINE UsageHint get_usage_hint() const;
  void set_usage_hint(UsageHint usage_hint);

  INLINE CPT(GeomVertexData) get_vertex_data() const;
  PT(GeomVertexData) modify_vertex_data();
  void set_vertex_data(const GeomVertexData *data);
  void offset_vertices(const GeomVertexData *data, int offset);
  int make_nonindexed(bool composite_only);

  INLINE int get_num_primitives() const;
  INLINE const GeomPrimitive *get_primitive(int i) const;
  INLINE GeomPrimitive *modify_primitive(int i);
  void set_primitive(int i, const GeomPrimitive *primitive);
  void add_primitive(const GeomPrimitive *primitive);
  void remove_primitive(int i);
  void clear_primitives();

  INLINE PT(Geom) decompose() const;
  INLINE PT(Geom) rotate() const;
  INLINE PT(Geom) unify() const;

  void decompose_in_place();
  void rotate_in_place();
  void unify_in_place();

  virtual bool copy_primitives_from(const Geom *other);

  int get_num_bytes() const;
  INLINE UpdateSeq get_modified() const;

  // Temporarily virtual.
  virtual void transform_vertices(const LMatrix4f &mat);

  // Temporarily virtual.
  virtual bool check_valid() const;

  virtual void output(ostream &out) const;
  virtual void write(ostream &out, int indent_level = 0) const;

  void clear_cache();

  void prepare(PreparedGraphicsObjects *prepared_objects);
  bool release(PreparedGraphicsObjects *prepared_objects);
  int release_all();

public:
  GeomContext *prepare_now(PreparedGraphicsObjects *prepared_objects, 
                           GraphicsStateGuardianBase *gsg);

  void draw(GraphicsStateGuardianBase *gsg, 
            const GeomMunger *munger,
            const GeomVertexData *vertex_data) const;

  void calc_tight_bounds(LPoint3f &min_point, LPoint3f &max_point,
                         bool &found_any, 
                         const GeomVertexData *vertex_data,
                         bool got_mat, const LMatrix4f &mat) const;
  INLINE void calc_tight_bounds(LPoint3f &min_point, LPoint3f &max_point,
                                bool &found_any) const;

  static UpdateSeq get_next_modified();

public:
  typedef pvector< PT(TextureStage) > ActiveTextureStages;
  typedef pset<TextureStage *> NoTexCoordStages;

protected:
  virtual BoundingVolume *recompute_bound();

private:
  void clear_prepared(PreparedGraphicsObjects *prepared_objects);
  bool check_will_be_valid(const GeomVertexData *vertex_data) const;

private:
  typedef pvector<PT(GeomPrimitive) > Primitives;

  // We have to use reference-counting pointers here instead of having
  // explicit cleanup in the GeomVertexFormat destructor, because the
  // cache needs to be stored in the CycleData, which makes accurate
  // cleanup more difficult.  We use the GeomCacheManager class to
  // avoid cache bloat.
  class CacheEntry : public GeomCacheEntry {
  public:
    INLINE CacheEntry(const GeomVertexData *source_data,
                      const GeomMunger *modifier);
    INLINE CacheEntry(Geom *source, 
                      const GeomVertexData *source_data,
                      const GeomMunger *modifier, 
                      const Geom *geom_result, 
                      const GeomVertexData *data_result);
    virtual ~CacheEntry();
    INLINE bool operator < (const CacheEntry &other) const;

    virtual void evict_callback();
    virtual void output(ostream &out) const;

    Geom *_source;
    CPT(GeomVertexData) _source_data;
    CPT(GeomMunger) _modifier;
    const Geom *_geom_result;  // ref-counted if not same as _source
    CPT(GeomVertexData) _data_result;
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

    PT(GeomVertexData) _data;
    Primitives _primitives;
    PrimitiveType _primitive_type;
    ShadeModel _shade_model;
    int _geom_rendering;
    UsageHint _usage_hint;
    bool _got_usage_hint;
    UpdateSeq _modified;
    Cache _cache;
  };

  PipelineCycler<CData> _cycler;
  typedef CycleDataReader<CData> CDReader;
  typedef CycleDataWriter<CData> CDWriter;

  void reset_usage_hint(CDWriter &cdata);
  void reset_geom_rendering(CDWriter &cdata);

  // This works just like the Texture contexts: each Geom keeps a
  // record of all the PGO objects that hold the Geom, and vice-versa.
  typedef pmap<PreparedGraphicsObjects *, GeomContext *> Contexts;
  Contexts _contexts;

  static UpdateSeq _next_modified;

public:
  static void register_with_read_factory();
  virtual void write_datagram(BamWriter *manager, Datagram &dg);

  virtual void finalize(BamReader *manager);

protected:
  static TypedWritable *make_from_bam(const FactoryParams &params);
  void fillin(DatagramIterator &scan, BamReader *manager);

public:
  static TypeHandle get_class_type() {
    return _type_handle;
  }
  static void init_type() {
    TypedWritableReferenceCount::init_type();
    BoundedObject::init_type();
    register_type(_type_handle, "Geom",
                  TypedWritableReferenceCount::get_class_type(),
                  BoundedObject::get_class_type());
  }
  virtual TypeHandle get_type() const {
    return get_class_type();
  }
  virtual TypeHandle force_init_type() {init_type(); return get_class_type();}

private:
  static TypeHandle _type_handle;

  friend class CacheEntry;
  friend class GeomMunger;
  friend class GeomContext;
  friend class PreparedGraphicsObjects;
};

INLINE ostream &operator << (ostream &out, const Geom &obj);

#include "geom.I"

#endif
