/**
 * PANDA 3D SOFTWARE
 * Copyright (c) Carnegie Mellon University.  All rights reserved.
 *
 * All use of this software is subject to the terms of the revised BSD
 * license.  You should have received a copy of this license along
 * with this source code in a file named "LICENSE."
 *
 * @file geom.h
 * @author drose
 * @date 2005-03-06
 */

#ifndef GEOM_H
#define GEOM_H

#include "pandabase.h"
#include "copyOnWriteObject.h"
#include "copyOnWritePointer.h"
#include "cycleData.h"
#include "cycleDataLockedReader.h"
#include "cycleDataReader.h"
#include "cycleDataWriter.h"
#include "cycleDataStageReader.h"
#include "cycleDataStageWriter.h"
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
#include "pmap.h"
#include "boundingVolume.h"
#include "pStatCollector.h"
#include "deletedChain.h"
#include "lightMutex.h"

class GeomContext;
class PreparedGraphicsObjects;

/**
 * A container for geometry primitives.  This class associates one or more
 * GeomPrimitive objects with a table of vertices defined by a GeomVertexData
 * object.  All of the primitives stored in a particular Geom are drawn from
 * the same set of vertices (each primitive uses a subset of all of the
 * vertices in the table), and all of them must be rendered at the same time,
 * in the same graphics state.
 */
class EXPCL_PANDA_GOBJ Geom : public CopyOnWriteObject, public GeomEnums {
protected:
  virtual PT(CopyOnWriteObject) make_cow_copy();

PUBLISHED:
  explicit Geom(const GeomVertexData *data);

protected:
  Geom(const Geom &copy);

PUBLISHED:
  void operator = (const Geom &copy);
  virtual ~Geom();
  ALLOC_DELETED_CHAIN(Geom);

  virtual Geom *make_copy() const;

  INLINE PrimitiveType get_primitive_type() const;
  INLINE ShadeModel get_shade_model() const;
  INLINE int get_geom_rendering() const;
  MAKE_PROPERTY(primitive_type, get_primitive_type);
  MAKE_PROPERTY(shade_model, get_shade_model);
  MAKE_PROPERTY(geom_rendering, get_geom_rendering);

  UsageHint get_usage_hint() const;
  void set_usage_hint(UsageHint usage_hint);
  //MAKE_PROPERTY(usage_hint, get_usage_hint, set_usage_hint);

  INLINE CPT(GeomVertexData) get_vertex_data(Thread *current_thread = Thread::get_current_thread()) const;
  PT(GeomVertexData) modify_vertex_data();
  void set_vertex_data(const GeomVertexData *data);
  void offset_vertices(const GeomVertexData *data, int offset);
  int make_nonindexed(bool composite_only);

  CPT(GeomVertexData) get_animated_vertex_data(bool force, Thread *current_thread = Thread::get_current_thread()) const;

  INLINE bool is_empty() const;

  INLINE size_t get_num_primitives() const;
  INLINE CPT(GeomPrimitive) get_primitive(size_t i) const;
  MAKE_SEQ(get_primitives, get_num_primitives, get_primitive);
  INLINE PT(GeomPrimitive) modify_primitive(size_t i);
  void set_primitive(size_t i, const GeomPrimitive *primitive);
  void insert_primitive(size_t i, const GeomPrimitive *primitive);
  INLINE void add_primitive(const GeomPrimitive *primitive);
  void remove_primitive(size_t i);
  void clear_primitives();
  MAKE_SEQ_PROPERTY(primitives, get_num_primitives, get_primitive, set_primitive, remove_primitive, insert_primitive);

  INLINE PT(Geom) decompose() const;
  INLINE PT(Geom) doubleside() const;
  INLINE PT(Geom) reverse() const;
  INLINE PT(Geom) rotate() const;
  INLINE PT(Geom) unify(int max_indices, bool preserve_order) const;
  INLINE PT(Geom) make_points() const;
  INLINE PT(Geom) make_lines() const;
  INLINE PT(Geom) make_patches() const;
  INLINE PT(Geom) make_adjacency() const;

  void decompose_in_place();
  void doubleside_in_place();
  void reverse_in_place();
  void rotate_in_place();
  void unify_in_place(int max_indices, bool preserve_order);
  void make_points_in_place();
  void make_lines_in_place();
  void make_patches_in_place();
  void make_adjacency_in_place();

  virtual bool copy_primitives_from(const Geom *other);

  int get_num_bytes() const;
  INLINE UpdateSeq get_modified(Thread *current_thread = Thread::get_current_thread()) const;
  MAKE_PROPERTY(num_bytes, get_num_bytes);
  MAKE_PROPERTY(modified, get_modified);

  bool request_resident() const;

  void transform_vertices(const LMatrix4 &mat);
  bool check_valid() const;
  bool check_valid(const GeomVertexData *vertex_data) const;

  CPT(BoundingVolume) get_bounds(Thread *current_thread = Thread::get_current_thread()) const;
  int get_nested_vertices(Thread *current_thread = Thread::get_current_thread()) const;
  INLINE void mark_bounds_stale() const;
  INLINE void set_bounds_type(BoundingVolume::BoundsType bounds_type);
  INLINE BoundingVolume::BoundsType get_bounds_type() const;
  INLINE void set_bounds(const BoundingVolume *volume);
  INLINE void clear_bounds();
  MAKE_PROPERTY(bounds_type, get_bounds_type, set_bounds_type);

  virtual void output(std::ostream &out) const;
  virtual void write(std::ostream &out, int indent_level = 0) const;

  void clear_cache();
  void clear_cache_stage(Thread *current_thread);

  void prepare(PreparedGraphicsObjects *prepared_objects);
  bool is_prepared(PreparedGraphicsObjects *prepared_objects) const;
  bool release(PreparedGraphicsObjects *prepared_objects);
  int release_all();

  GeomContext *prepare_now(PreparedGraphicsObjects *prepared_objects,
                           GraphicsStateGuardianBase *gsg);

public:
  bool draw(GraphicsStateGuardianBase *gsg,
            const GeomVertexData *vertex_data,
            bool force, Thread *current_thread) const;

  INLINE void calc_tight_bounds(LPoint3 &min_point, LPoint3 &max_point,
                                bool &found_any,
                                const GeomVertexData *vertex_data,
                                bool got_mat, const LMatrix4 &mat,
                                Thread *current_thread) const;
  INLINE void calc_tight_bounds(LPoint3 &min_point, LPoint3 &max_point,
                                bool &found_any, Thread *current_thread) const;
  INLINE void calc_tight_bounds(LPoint3 &min_point, LPoint3 &max_point,
                                bool &found_any,
                                const GeomVertexData *vertex_data,
                                bool got_mat, const LMatrix4 &mat,
                                const InternalName *column_name,
                                Thread *current_thread) const;

  static UpdateSeq get_next_modified();

private:
  class CData;

  INLINE void mark_internal_bounds_stale(CData *cdata);
  void compute_internal_bounds(CData *cdata, Thread *current_thread) const;

  void do_calc_tight_bounds(LPoint3 &min_point, LPoint3 &max_point,
                            PN_stdfloat &sq_center_dist, bool &found_any,
                            const GeomVertexData *vertex_data,
                            bool got_mat, const LMatrix4 &mat,
                            const InternalName *column_name,
                            const CData *cdata, Thread *current_thread) const;

  void do_calc_sphere_radius(const LPoint3 &center,
                             PN_stdfloat &sq_radius, bool &found_any,
                             const GeomVertexData *vertex_data,
                             const CData *cdata, Thread *current_thread) const;

  void clear_prepared(PreparedGraphicsObjects *prepared_objects);
  bool check_will_be_valid(const GeomVertexData *vertex_data) const;

  void reset_geom_rendering(CData *cdata);

  void combine_primitives(GeomPrimitive *a_prim, CPT(GeomPrimitive) b_prim,
                          Thread *current_thread);

private:
  typedef pvector<COWPT(GeomPrimitive) > Primitives;

  // We have to use reference-counting pointers here instead of having
  // explicit cleanup in the GeomVertexFormat destructor, because the cache
  // needs to be stored in the CycleData, which makes accurate cleanup more
  // difficult.  We use the GeomCacheManager class to avoid cache bloat.

  // Note: the above comment is no longer true.  The cache is not stored in
  // the CycleData, which just causes problems; instead, we cycle each
  // individual CacheEntry as needed.  Need to investigate if we could
  // simplify the cache system now.

  // The pipelined data with each CacheEntry.
  class EXPCL_PANDA_GOBJ CDataCache : public CycleData {
  public:
    INLINE CDataCache();
    INLINE CDataCache(const CDataCache &copy);
    virtual ~CDataCache();
    ALLOC_DELETED_CHAIN(CDataCache);
    virtual CycleData *make_copy() const;
    virtual TypeHandle get_parent_type() const {
      return Geom::get_class_type();
    }

    INLINE void set_result(const Geom *geom_result, const GeomVertexData *data_result);

    Geom *_source;  // A back pointer to the containing Geom
    const Geom *_geom_result;  // ref-counted if not NULL and not same as _source
    CPT(GeomVertexData) _data_result;

  public:
    static TypeHandle get_class_type() {
      return _type_handle;
    }
    static void init_type() {
      register_type(_type_handle, "Geom::CDataCache");
    }

  private:
    static TypeHandle _type_handle;
  };
  typedef CycleDataReader<CDataCache> CDCacheReader;
  typedef CycleDataWriter<CDataCache> CDCacheWriter;

public:
  // The CacheKey class separates out just the part of CacheEntry that is used
  // to key the cache entry within the map.  We have this as a separate class
  // so we can easily look up a new entry in the map, without having to
  // execute the relatively expensive CacheEntry constructor.
  class CacheKey {
  public:
    INLINE CacheKey(const GeomVertexData *source_data,
                    const GeomMunger *modifier);
    INLINE CacheKey(const CacheKey &copy);
    INLINE CacheKey(CacheKey &&from) noexcept;

    INLINE bool operator < (const CacheKey &other) const;

    CPT(GeomVertexData) _source_data;
    CPT(GeomMunger) _modifier;
  };
  // It is not clear why MSVC7 needs this class to be public.
  class CacheEntry : public GeomCacheEntry {
  public:
    INLINE CacheEntry(Geom *source,
                      const GeomVertexData *source_data,
                      const GeomMunger *modifier);
    INLINE CacheEntry(Geom *source, const CacheKey &key);
    INLINE CacheEntry(Geom *source, CacheKey &&key) noexcept;

    ALLOC_DELETED_CHAIN(CacheEntry);

    virtual void evict_callback();
    virtual void output(std::ostream &out) const;

    Geom *_source;  // A back pointer to the containing Geom
    CacheKey _key;

    PipelineCycler<CDataCache> _cycler;

  public:
    static TypeHandle get_class_type() {
      return _type_handle;
    }
    static void init_type() {
      GeomCacheEntry::init_type();
      register_type(_type_handle, "Geom::CacheEntry",
                    GeomCacheEntry::get_class_type());
    }

  private:
    static TypeHandle _type_handle;
  };
  typedef pmap<const CacheKey *, PT(CacheEntry), IndirectLess<CacheKey> > Cache;

private:
  // This is the data that must be cycled between pipeline stages.
  class EXPCL_PANDA_GOBJ CData : public CycleData {
  public:
    INLINE CData();
    INLINE CData(GeomVertexData *data);

    ALLOC_DELETED_CHAIN(CData);
    virtual CycleData *make_copy() const;
    virtual void write_datagram(BamWriter *manager, Datagram &dg) const;
    virtual int complete_pointers(TypedWritable **plist, BamReader *manager);
    virtual void fillin(DatagramIterator &scan, BamReader *manager);
    virtual TypeHandle get_parent_type() const {
      return Geom::get_class_type();
    }

    COWPT(GeomVertexData) _data;
    Primitives _primitives;
    PrimitiveType _primitive_type;
    ShadeModel _shade_model;
    int _geom_rendering;
    UpdateSeq _modified;

    CPT(BoundingVolume) _internal_bounds;
    int _nested_vertices;
    bool _internal_bounds_stale;
    BoundingVolume::BoundsType _bounds_type;
    CPT(BoundingVolume) _user_bounds;

  public:
    static TypeHandle get_class_type() {
      return _type_handle;
    }
    static void init_type() {
      register_type(_type_handle, "Geom::CData");
    }

  private:
    static TypeHandle _type_handle;
  };

  PipelineCycler<CData> _cycler;
  typedef CycleDataLockedReader<CData> CDLockedReader;
  typedef CycleDataReader<CData> CDReader;
  typedef CycleDataWriter<CData> CDWriter;
  typedef CycleDataStageReader<CData> CDStageReader;
  typedef CycleDataStageWriter<CData> CDStageWriter;

  Cache _cache;
  LightMutex _cache_lock;

  // This works just like the Texture contexts: each Geom keeps a record of
  // all the PGO objects that hold the Geom, and vice-versa.
  typedef pmap<PreparedGraphicsObjects *, GeomContext *> Contexts;
  Contexts _contexts;

  static UpdateSeq _next_modified;
  static PStatCollector _draw_primitive_setup_pcollector;

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
    CopyOnWriteObject::init_type();
    register_type(_type_handle, "Geom",
                  CopyOnWriteObject::get_class_type());
    CDataCache::init_type();
    CacheEntry::init_type();
    CData::init_type();
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
  friend class GeomPipelineReader;
  friend class PreparedGraphicsObjects;
};

/**
 * Encapsulates the data from a Geom, pre-fetched for one stage of the
 * pipeline.
 *
 * Does not hold a reference to the Geom.  The caller must ensure that the
 * Geom persists for at least the lifetime of the GeomPipelineReader.
 */
class EXPCL_PANDA_GOBJ GeomPipelineReader : public GeomEnums {
public:
  INLINE GeomPipelineReader(Thread *current_thread);
  INLINE GeomPipelineReader(const Geom *object, Thread *current_thread);
  GeomPipelineReader(const GeomPipelineReader &copy) = delete;
  INLINE ~GeomPipelineReader();

  ALLOC_DELETED_CHAIN(GeomPipelineReader);

  GeomPipelineReader &operator = (const GeomPipelineReader &copy) = delete;

  INLINE void set_object(const Geom *object);
  INLINE const Geom *get_object() const;
  INLINE Thread *get_current_thread() const;

  INLINE PrimitiveType get_primitive_type() const;
  INLINE ShadeModel get_shade_model() const;
  INLINE int get_geom_rendering() const;
  INLINE CPT(GeomVertexData) get_vertex_data() const;
  INLINE int get_num_primitives() const;
  INLINE CPT(GeomPrimitive) get_primitive(int i) const;

  INLINE UpdateSeq get_modified() const;

  bool check_valid(const GeomVertexDataPipelineReader *data_reader) const;

  INLINE GeomContext *prepare_now(PreparedGraphicsObjects *prepared_objects,
                                  GraphicsStateGuardianBase *gsg) const;

  bool draw(GraphicsStateGuardianBase *gsg,
            const GeomVertexDataPipelineReader *data_reader,
            bool force) const;

private:
  const Geom *_object;
  Thread *_current_thread;
  const Geom::CData *_cdata;

public:
  static TypeHandle get_class_type() {
    return _type_handle;
  }
  static void init_type() {
    register_type(_type_handle, "GeomPipelineReader");
  }

private:
  static TypeHandle _type_handle;
};

INLINE std::ostream &operator << (std::ostream &out, const Geom &obj);

#include "geom.I"

#endif
