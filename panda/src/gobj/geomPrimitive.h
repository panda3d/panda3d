/**
 * PANDA 3D SOFTWARE
 * Copyright (c) Carnegie Mellon University.  All rights reserved.
 *
 * All use of this software is subject to the terms of the revised BSD
 * license.  You should have received a copy of this license along
 * with this source code in a file named "LICENSE."
 *
 * @file geomPrimitive.h
 * @author drose
 * @date 2005-03-06
 */

#ifndef GEOMPRIMITIVE_H
#define GEOMPRIMITIVE_H

#include "pandabase.h"
#include "geomEnums.h"
#include "geomVertexArrayData.h"
#include "geomVertexData.h"
#include "copyOnWriteObject.h"
#include "luse.h"
#include "updateSeq.h"
#include "pointerTo.h"
#include "pta_int.h"
#include "pStatCollector.h"
#include "cycleData.h"
#include "cycleDataReader.h"
#include "cycleDataWriter.h"
#include "cycleDataStageReader.h"
#include "cycleDataStageWriter.h"
#include "pipelineCycler.h"
#include "deletedChain.h"

class PreparedGraphicsObjects;
class IndexBufferContext;
class GraphicsStateGuardianBase;
class FactoryParams;
class GeomPrimitivePipelineReader;

/**
 * This is an abstract base class for a family of classes that represent the
 * fundamental geometry primitives that may be stored in a Geom.
 *
 * They all have in common the fact that they are defined by tables of vertex
 * data stored in a GeomVertexData object.  Each GeomPrimitive object contains
 * an ordered list of integers, which index into the vertex array defined by
 * the GeomVertexData and define the particular vertices of the GeomVertexData
 * that are used for this primitive.
 *
 * The meaning of a given arrangement of vertices is defined by each
 * individual primitive type; for instance, a GeomTriangle renders a triangle
 * from each three consecutive vertices, while a GeomTriangleStrip renders a
 * strip of (n - 2) connected triangles from each sequence of n vertices.
 */
class EXPCL_PANDA_GOBJ GeomPrimitive : public CopyOnWriteObject, public GeomEnums {
protected:
  GeomPrimitive();
  virtual PT(CopyOnWriteObject) make_cow_copy();

PUBLISHED:
  explicit GeomPrimitive(UsageHint usage_hint);
  GeomPrimitive(const GeomPrimitive &copy);
  void operator = (const GeomPrimitive &copy);
  virtual ~GeomPrimitive();
  ALLOC_DELETED_CHAIN(GeomPrimitive);

  virtual PT(GeomPrimitive) make_copy() const=0;

  virtual PrimitiveType get_primitive_type() const=0;
  virtual int get_geom_rendering() const;
  MAKE_PROPERTY(primitive_type, get_primitive_type);
  MAKE_PROPERTY(geom_rendering, get_geom_rendering);

  INLINE ShadeModel get_shade_model() const;
  INLINE void set_shade_model(ShadeModel shade_model);
  MAKE_PROPERTY(shade_model, get_shade_model);

  INLINE UsageHint get_usage_hint() const;
  void set_usage_hint(UsageHint usage_hint);
  MAKE_PROPERTY(usage_hint, get_usage_hint);

  INLINE NumericType get_index_type() const;
  void set_index_type(NumericType index_type);
  MAKE_PROPERTY(index_type, get_index_type);

  // The following published methods are provided for safe, high-level
  // iteration through the vertices and sub-primitives within the
  // GeomPrimitive class.  These work correctly regardless of the primitive
  // type and without depending on knowledge about the way primitives' lengths
  // are encoded.  You can also safely build up a composite primitive using
  // these methods.

  INLINE bool is_composite() const;
  INLINE bool is_indexed() const;
  INLINE int get_first_vertex() const;
  INLINE int get_num_vertices() const;
  INLINE int get_vertex(int i) const;
  MAKE_SEQ(get_vertex_list, get_num_vertices, get_vertex);
  void add_vertex(int vertex);
  INLINE void add_vertices(int v1, int v2);
  INLINE void add_vertices(int v1, int v2, int v3);
  INLINE void add_vertices(int v1, int v2, int v3, int v4);
  void add_consecutive_vertices(int start, int num_vertices);
  void add_next_vertices(int num_vertices);
  void reserve_num_vertices(int num_vertices);
  bool close_primitive();
  void clear_vertices();
  void offset_vertices(int offset);
  void offset_vertices(int offset, int begin_row, int end_row);
  void make_nonindexed(GeomVertexData *dest, const GeomVertexData *source);
  void pack_vertices(GeomVertexData *dest, const GeomVertexData *source);
  void make_indexed();

  INLINE int get_num_primitives() const;
  int get_primitive_start(int n) const;
  int get_primitive_end(int n) const;
  int get_primitive_num_vertices(int n) const;
  int get_num_used_vertices() const;

  INLINE int get_num_faces() const;
  INLINE int get_primitive_num_faces(int n) const;

  INLINE int get_min_vertex() const;
  int get_primitive_min_vertex(int n) const;
  INLINE int get_max_vertex() const;
  int get_primitive_max_vertex(int n) const;

  CPT(GeomPrimitive) decompose() const;
  CPT(GeomPrimitive) rotate() const;
  CPT(GeomPrimitive) doubleside() const;
  CPT(GeomPrimitive) reverse() const;
  CPT(GeomPrimitive) match_shade_model(ShadeModel shade_model) const;
  CPT(GeomPrimitive) make_points() const;
  CPT(GeomPrimitive) make_lines() const;
  CPT(GeomPrimitive) make_patches() const;
  virtual CPT(GeomPrimitive) make_adjacency() const;

  int get_num_bytes() const;
  INLINE int get_data_size_bytes() const;
  INLINE UpdateSeq get_modified() const;
  MAKE_PROPERTY(num_bytes, get_num_bytes);
  MAKE_PROPERTY(data_size_bytes, get_data_size_bytes);
  MAKE_PROPERTY(modified, get_modified);

  bool request_resident(Thread *current_thread = Thread::get_current_thread()) const;

  INLINE bool check_valid(const GeomVertexData *vertex_data) const;
  INLINE bool check_valid(const GeomVertexDataPipelineReader *data_reader) const;

  virtual void output(std::ostream &out) const;
  virtual void write(std::ostream &out, int indent_level) const;

PUBLISHED:
/*
 * These public methods are not intended for high-level usage.  They are
 * public so that low-level code that absolutely needs fast access to the
 * primitive data can get to it, but using them requires knowledge about how
 * the component primitives are encoded within the GeomPrimitive class, and
 * it's easy to screw something up.  Also, if too many code samples depend on
 * this internal knowledge, it may make it difficult to extend this class
 * later.  It is recommended that application-level code use the above
 * interfaces instead.
 */

  INLINE CPT(GeomVertexArrayData) get_vertices() const;
  INLINE CPT(GeomVertexArrayDataHandle) get_vertices_handle(Thread *current_thread) const;
  PT(GeomVertexArrayData) modify_vertices(int num_vertices = -1);
  INLINE PT(GeomVertexArrayDataHandle) modify_vertices_handle(Thread *current_thread);
  void set_vertices(const GeomVertexArrayData *vertices, int num_vertices = -1);
  void set_nonindexed_vertices(int first_vertex, int num_vertices);

  INLINE int get_index_stride() const;
  INLINE int get_strip_cut_index() const;
  MAKE_PROPERTY(index_stride, get_index_stride);
  MAKE_PROPERTY(strip_cut_index, get_strip_cut_index);

  INLINE CPTA_int get_ends() const;
  PTA_int modify_ends();
  void set_ends(PTA_int ends);

  INLINE CPT(GeomVertexArrayData) get_mins() const;
  INLINE CPT(GeomVertexArrayData) get_maxs() const;
  MAKE_PROPERTY(mins, get_mins);
  MAKE_PROPERTY(maxs, get_maxs);

  void set_minmax(int min_vertex, int max_vertex,
                  GeomVertexArrayData *mins, GeomVertexArrayData *maxs);
  void clear_minmax();

  virtual int get_num_vertices_per_primitive() const;
  virtual int get_min_num_vertices_per_primitive() const;
  virtual int get_num_unused_vertices_per_primitive() const;
  MAKE_PROPERTY(num_vertices_per_primitive, get_num_vertices_per_primitive);
  MAKE_PROPERTY(min_num_vertices_per_primitive, get_min_num_vertices_per_primitive);
  MAKE_PROPERTY(num_unused_vertices_per_primitive, get_num_unused_vertices_per_primitive);

public:
  void prepare(PreparedGraphicsObjects *prepared_objects);
  bool is_prepared(PreparedGraphicsObjects *prepared_objects) const;

  IndexBufferContext *prepare_now(PreparedGraphicsObjects *prepared_objects,
                                  GraphicsStateGuardianBase *gsg);
  bool release(PreparedGraphicsObjects *prepared_objects);
  int release_all();

  static const GeomVertexArrayFormat *get_index_format(NumericType index_type);
  INLINE const GeomVertexArrayFormat *get_index_format() const;
  INLINE PT(GeomVertexArrayData) make_index_data() const;

private:
  static CPT(GeomVertexArrayFormat) make_index_format(NumericType index_type);

  void clear_prepared(PreparedGraphicsObjects *prepared_objects);
  static int get_highest_index_value(NumericType index_type);
  static int get_strip_cut_index(NumericType index_type);

public:
  virtual bool draw(GraphicsStateGuardianBase *gsg,
                    const GeomPrimitivePipelineReader *reader,
                    bool force) const=0;

  void calc_tight_bounds(LPoint3 &min_point, LPoint3 &max_point,
                         PN_stdfloat &sq_center_dist, bool &found_any,
                         const GeomVertexData *vertex_data,
                         bool got_mat, const LMatrix4 &mat,
                         const InternalName *column_name,
                         Thread *current_thread) const;

  void calc_sphere_radius(const LPoint3 &center,
                          PN_stdfloat &sq_radius, bool &found_any,
                          const GeomVertexData *vertex_data,
                          Thread *current_thread) const;

protected:
  virtual CPT(GeomPrimitive) decompose_impl() const;
  virtual CPT(GeomVertexArrayData) rotate_impl() const;
  virtual CPT(GeomPrimitive) doubleside_impl() const;
  virtual CPT(GeomPrimitive) reverse_impl() const;
  virtual bool requires_unused_vertices() const;
  virtual void append_unused_vertices(GeomVertexArrayData *vertices,
                                      int vertex);

private:
  class CData;

  void recompute_minmax(CData *cdata);
  void do_make_indexed(CData *cdata);
  void consider_elevate_index_type(CData *cdata, int vertex);
  void do_set_index_type(CData *cdata, NumericType index_type);
  PT(GeomVertexArrayData) do_modify_vertices(CData *cdata);

private:
  // A GeomPrimitive keeps a list (actually, a map) of all the
  // PreparedGraphicsObjects tables that it has been prepared into.  Each PGO
  // conversely keeps a list (a set) of all the Geoms that have been prepared
  // there.  When either destructs, it removes itself from the other's list.
  typedef pmap<PreparedGraphicsObjects *, IndexBufferContext *> Contexts;
  Contexts _contexts;

  // This is the data that must be cycled between pipeline stages.
  class EXPCL_PANDA_GOBJ CData : public CycleData {
  public:
    INLINE CData();
    INLINE CData(const CData &copy);
    ALLOC_DELETED_CHAIN(CData);

    virtual CycleData *make_copy() const;
    virtual void write_datagram(BamWriter *manager, Datagram &dg) const;
    virtual int complete_pointers(TypedWritable **plist, BamReader *manager);
    virtual void fillin(DatagramIterator &scan, BamReader *manager);
    virtual TypeHandle get_parent_type() const {
      return GeomPrimitive::get_class_type();
    }

    ShadeModel _shade_model;
    int _first_vertex;
    int _num_vertices;
    NumericType _index_type;
    UsageHint _usage_hint;
    COWPT(GeomVertexArrayData) _vertices;
    PTA_int _ends;
    COWPT(GeomVertexArrayData) _mins;
    COWPT(GeomVertexArrayData) _maxs;
    UpdateSeq _modified;

    bool _got_minmax;
    unsigned int _min_vertex;
    unsigned int _max_vertex;

  public:
    static TypeHandle get_class_type() {
      return _type_handle;
    }
    static void init_type() {
      register_type(_type_handle, "GeomPrimitive::CData");
    }

  private:
    static TypeHandle _type_handle;

    friend class GeomPrimitive;
  };

  PipelineCycler<CData> _cycler;
  typedef CycleDataReader<CData> CDReader;
  typedef CycleDataWriter<CData> CDWriter;
  typedef CycleDataStageReader<CData> CDStageReader;
  typedef CycleDataStageWriter<CData> CDStageWriter;

private:
  static PStatCollector _decompose_pcollector;
  static PStatCollector _doubleside_pcollector;
  static PStatCollector _reverse_pcollector;
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
    CopyOnWriteObject::init_type();
    register_type(_type_handle, "GeomPrimitive",
                  CopyOnWriteObject::get_class_type());
    CData::init_type();
  }
  virtual TypeHandle get_type() const {
    return get_class_type();
  }
  virtual TypeHandle force_init_type() {init_type(); return get_class_type();}

private:
  static TypeHandle _type_handle;

  friend class Geom;
  friend class PreparedGraphicsObjects;
  friend class GeomPrimitivePipelineReader;
};

/**
 * Encapsulates the data from a GeomPrimitive, pre-fetched for one stage of
 * the pipeline.
 */
class EXPCL_PANDA_GOBJ GeomPrimitivePipelineReader : public GeomEnums {
public:
  INLINE GeomPrimitivePipelineReader(CPT(GeomPrimitive) object, Thread *current_thread);
  GeomPrimitivePipelineReader(const GeomPrimitivePipelineReader &copy) = delete;
  INLINE ~GeomPrimitivePipelineReader();

  ALLOC_DELETED_CHAIN(GeomPrimitivePipelineReader);

  GeomPrimitivePipelineReader &operator = (const GeomPrimitivePipelineReader &copy) = delete;

  INLINE const GeomPrimitive *get_object() const;
  INLINE Thread *get_current_thread() const;

  void check_minmax() const;

  INLINE ShadeModel get_shade_model() const;
  INLINE UsageHint get_usage_hint() const;
  INLINE NumericType get_index_type() const;
  INLINE bool is_indexed() const;
  int get_first_vertex() const;
  INLINE int get_num_vertices() const;
  int get_vertex(int i) const;
  int get_num_primitives() const;
  void get_referenced_vertices(BitArray &bits) const;
  INLINE int get_min_vertex() const;
  INLINE int get_max_vertex() const;
  INLINE int get_data_size_bytes() const;
  INLINE UpdateSeq get_modified() const;
  bool check_valid(const GeomVertexDataPipelineReader *data_reader) const;
  INLINE int get_index_stride() const;
  INLINE const unsigned char *get_read_pointer(bool force) const;
  INLINE int get_strip_cut_index() const;
  INLINE CPTA_int get_ends() const;
  INLINE CPT(GeomVertexArrayData) get_mins() const;
  INLINE CPT(GeomVertexArrayData) get_maxs() const;

  INLINE IndexBufferContext *prepare_now(PreparedGraphicsObjects *prepared_objects,
                                         GraphicsStateGuardianBase *gsg) const;
  INLINE bool draw(GraphicsStateGuardianBase *gsg, bool force) const;

private:
  CPT(GeomPrimitive) _object;
  Thread *_current_thread;
  const GeomPrimitive::CData *_cdata;

  CPT(GeomVertexArrayData) _vertices;
  const GeomVertexArrayData::CData *_vertices_cdata;

public:
  static TypeHandle get_class_type() {
    return _type_handle;
  }
  static void init_type() {
    register_type(_type_handle, "GeomPrimitivePipelineReader");
  }

private:
  static TypeHandle _type_handle;
};

INLINE std::ostream &operator << (std::ostream &out, const GeomPrimitive &obj);

#include "geomPrimitive.I"

#endif
