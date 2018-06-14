/**
 * PANDA 3D SOFTWARE
 * Copyright (c) Carnegie Mellon University.  All rights reserved.
 *
 * All use of this software is subject to the terms of the revised BSD
 * license.  You should have received a copy of this license along
 * with this source code in a file named "LICENSE."
 *
 * @file geomVertexArrayData.h
 * @author drose
 * @date 2005-03-17
 */

#ifndef GEOMVERTEXARRAYDATA_H
#define GEOMVERTEXARRAYDATA_H

#include "pandabase.h"
#include "copyOnWriteObject.h"
#include "geomVertexArrayFormat.h"
#include "geomEnums.h"
#include "pta_uchar.h"
#include "updateSeq.h"
#include "cycleData.h"
#include "cycleDataReader.h"
#include "cycleDataWriter.h"
#include "cycleDataStageReader.h"
#include "cycleDataStageWriter.h"
#include "pipelineCycler.h"
#include "pmap.h"
#include "reMutex.h"
#include "simpleLru.h"
#include "vertexDataBuffer.h"
#include "config_gobj.h"
#include "bamReader.h"

class PreparedGraphicsObjects;
class VertexBufferContext;
class GraphicsStateGuardianBase;
class GeomVertexArrayDataHandle;
class VertexDataBook;
class SimpleAllocatorBlock;

/**
 * This is the data for one array of a GeomVertexData structure.  Many
 * GeomVertexData structures will only define one array, with all data
 * elements interleaved (DirectX 8.0 and before insisted on this format); some
 * will define multiple arrays.
 *
 * DirectX calls this concept of one array a "stream". It also closely
 * correlates with the concept of a vertex buffer.
 *
 * This object is just a block of data.  In general, you should not be
 * directly messing with this object from application code.  See
 * GeomVertexData for the organizing structure, and see
 * GeomVertexReader/Writer/Rewriter for high-level tools to manipulate the
 * actual vertex data.
 */
class EXPCL_PANDA_GOBJ GeomVertexArrayData : public CopyOnWriteObject, public SimpleLruPage, public GeomEnums {
private:
  GeomVertexArrayData();

protected:
  virtual PT(CopyOnWriteObject) make_cow_copy();

PUBLISHED:
  explicit GeomVertexArrayData(const GeomVertexArrayFormat *array_format,
                               UsageHint usage_hint);
  GeomVertexArrayData(const GeomVertexArrayData &copy);
  void operator = (const GeomVertexArrayData &copy);
  virtual ~GeomVertexArrayData();
  ALLOC_DELETED_CHAIN(GeomVertexArrayData);

  int compare_to(const GeomVertexArrayData &other) const;

  INLINE const GeomVertexArrayFormat *get_array_format() const;
  MAKE_PROPERTY(array_format, get_array_format);

  INLINE UsageHint get_usage_hint() const;
  void set_usage_hint(UsageHint usage_hint);
  MAKE_PROPERTY(usage_hint, get_usage_hint, set_usage_hint);

  INLINE bool has_column(const InternalName *name) const;

  INLINE int get_num_rows() const;
  INLINE bool set_num_rows(int n);
  INLINE bool unclean_set_num_rows(int n);
  INLINE bool reserve_num_rows(int n);
  INLINE void clear_rows();

  INLINE size_t get_data_size_bytes() const;
  INLINE UpdateSeq get_modified() const;
  MAKE_PROPERTY(data_size_bytes, get_data_size_bytes);
  MAKE_PROPERTY(modified, get_modified);

  void output(std::ostream &out) const;
  void write(std::ostream &out, int indent_level = 0) const;

  INLINE bool request_resident(Thread *current_thread = Thread::get_current_thread()) const;

  INLINE CPT(GeomVertexArrayDataHandle) get_handle(Thread *current_thread = Thread::get_current_thread()) const;
  INLINE PT(GeomVertexArrayDataHandle) modify_handle(Thread *current_thread = Thread::get_current_thread());

  void prepare(PreparedGraphicsObjects *prepared_objects);
  bool is_prepared(PreparedGraphicsObjects *prepared_objects) const;

  VertexBufferContext *prepare_now(PreparedGraphicsObjects *prepared_objects,
                                   GraphicsStateGuardianBase *gsg);
  bool release(PreparedGraphicsObjects *prepared_objects);
  int release_all();

  INLINE static SimpleLru *get_independent_lru();
  INLINE static SimpleLru *get_small_lru();
  static void lru_epoch();
  INLINE static VertexDataBook &get_book();

#ifdef HAVE_PYTHON
  EXTENSION(int __getbuffer__(PyObject *self, Py_buffer *view, int flags));
  EXTENSION(int __getbuffer__(PyObject *self, Py_buffer *view, int flags) const);
  EXTENSION(void __releasebuffer__(PyObject *self, Py_buffer *view) const);
#endif

public:
  virtual void evict_lru();

private:
  INLINE void set_lru_size(size_t lru_size);
  INLINE void mark_used();

  void clear_prepared(PreparedGraphicsObjects *prepared_objects);
  void reverse_data_endianness(unsigned char *dest,
                               const unsigned char *source, size_t size);


  CPT(GeomVertexArrayFormat) _array_format;

  // A GeomVertexArrayData keeps a list (actually, a map) of all the
  // PreparedGraphicsObjects tables that it has been prepared into.  Each PGO
  // conversely keeps a list (a set) of all the Geoms that have been prepared
  // there.  When either destructs, it removes itself from the other's list.
  typedef pmap<PreparedGraphicsObjects *, VertexBufferContext *> Contexts;
  Contexts *_contexts;

  // This data is only needed when reading from a bam file.
  class BamAuxData : public BamReader::AuxData {
  public:
    // set true to indicate the data must be endian-reversed in finalize().
    bool _endian_reversed;
  };

  // This is the data that must be cycled between pipeline stages.
  class EXPCL_PANDA_GOBJ CData : public CycleData {
  public:
    INLINE CData(UsageHint usage_hint = UH_unspecified);
    INLINE CData(CData &&from) noexcept;
    INLINE CData(const CData &copy);
    INLINE void operator = (const CData &copy);

    virtual ~CData();
    ALLOC_DELETED_CHAIN(CData);
    virtual CycleData *make_copy() const;
    virtual void write_datagram(BamWriter *manager, Datagram &dg,
                                void *extra_data) const;
    virtual void fillin(DatagramIterator &scan, BamReader *manager,
                        void *extra_data);
    virtual TypeHandle get_parent_type() const {
      return GeomVertexArrayData::get_class_type();
    }

    UsageHint _usage_hint;
    VertexDataBuffer _buffer;
    UpdateSeq _modified;

    // This implements read-write locking.  Anyone who gets the data for
    // reading or writing will hold this mutex during the lock.
    ReMutex _rw_lock;

  public:
    static TypeHandle get_class_type() {
      return _type_handle;
    }
    static void init_type() {
      register_type(_type_handle, "GeomVertexArrayData::CData");
    }

  private:
    static TypeHandle _type_handle;

    friend class GeomVertexArrayData;
  };

  PipelineCycler<CData> _cycler;
  typedef CycleDataReader<CData> CDReader;
  typedef CycleDataWriter<CData> CDWriter;
  typedef CycleDataStageReader<CData> CDStageReader;
  typedef CycleDataStageWriter<CData> CDStageWriter;

  static SimpleLru _independent_lru;
  static SimpleLru _small_lru;
  static VertexDataBook _book;

public:
  static void register_with_read_factory();
  virtual void write_datagram(BamWriter *manager, Datagram &dg);
  PTA_uchar read_raw_data(BamReader *manager, DatagramIterator &source);
  virtual int complete_pointers(TypedWritable **plist, BamReader *manager);

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
    register_type(_type_handle, "GeomVertexArrayData",
                  CopyOnWriteObject::get_class_type());
    CData::init_type();
  }
  virtual TypeHandle get_type() const {
    return get_class_type();
  }
  virtual TypeHandle force_init_type() {init_type(); return get_class_type();}

private:
  static TypeHandle _type_handle;

  friend class GeomCacheManager;
  friend class GeomVertexData;
  friend class PreparedGraphicsObjects;
  friend class GeomVertexArrayDataHandle;
  friend class GeomPrimitivePipelineReader;
};

/**
 * This data object is returned by GeomVertexArrayData::get_handle() or
 * modify_handle(). As long as it exists, the data is locked; when the last of
 * these destructs, the data is unlocked.
 *
 * Only one thread at a time may lock the data; other threads attempting to
 * lock the data will block.  A given thread may simultaneously lock the data
 * multiple times.
 *
 * This class serves in lieu of a pair of GeomVertexArrayDataPipelineReader
 * and GeomVertexArrayDataPipelineWriter classes
 */
class EXPCL_PANDA_GOBJ GeomVertexArrayDataHandle : public ReferenceCount, public GeomEnums {
private:
  INLINE GeomVertexArrayDataHandle(CPT(GeomVertexArrayData) object,
                                   Thread *current_thread);
  INLINE GeomVertexArrayDataHandle(const GeomVertexArrayData *object,
                                   Thread *current_thread);
  INLINE GeomVertexArrayDataHandle(PT(GeomVertexArrayData) object,
                                   Thread *current_thread);
  INLINE GeomVertexArrayDataHandle(GeomVertexArrayData *object,
                                   Thread *current_thread);

PUBLISHED:
  INLINE ~GeomVertexArrayDataHandle();

public:
  GeomVertexArrayDataHandle(const GeomVertexArrayDataHandle &) = delete;

  ALLOC_DELETED_CHAIN_DECL(GeomVertexArrayDataHandle);

  GeomVertexArrayDataHandle &operator = (const GeomVertexArrayDataHandle &) = delete;

  INLINE Thread *get_current_thread() const;

  INLINE const unsigned char *get_read_pointer(bool force) const RETURNS_ALIGNED(MEMORY_HOOK_ALIGNMENT);
  unsigned char *get_write_pointer() RETURNS_ALIGNED(MEMORY_HOOK_ALIGNMENT);

PUBLISHED:
  INLINE const GeomVertexArrayData *get_object() const;
  INLINE GeomVertexArrayData *get_object();
  MAKE_PROPERTY(object, get_object);

  INLINE const GeomVertexArrayFormat *get_array_format() const;
  INLINE UsageHint get_usage_hint() const;
  MAKE_PROPERTY(array_format, get_array_format);
  MAKE_PROPERTY(usage_hint, get_usage_hint);

  INLINE int get_num_rows() const;
  bool set_num_rows(int n);
  bool unclean_set_num_rows(int n);
  bool reserve_num_rows(int n);
  INLINE void clear_rows();

  INLINE size_t get_data_size_bytes() const;
  INLINE UpdateSeq get_modified() const;
  MAKE_PROPERTY(data_size_bytes, get_data_size_bytes);
  MAKE_PROPERTY(modified, get_modified);

  INLINE bool request_resident() const;

  INLINE VertexBufferContext *prepare_now(PreparedGraphicsObjects *prepared_objects,
                                          GraphicsStateGuardianBase *gsg) const;

  void copy_data_from(const GeomVertexArrayDataHandle *other);
  void copy_subdata_from(size_t to_start, size_t to_size,
                         const GeomVertexArrayDataHandle *other,
                         size_t from_start, size_t from_size);

  void copy_data_from(const unsigned char *source, size_t size);
  void copy_subdata_from(size_t to_start, size_t to_size,
                         const unsigned char *source,
                         size_t from_start, size_t from_size);

  EXTENSION(void copy_data_from(PyObject *buffer));
  EXTENSION(void copy_subdata_from(size_t to_start, size_t to_size,
                                   PyObject *buffer));
  EXTENSION(void copy_subdata_from(size_t to_start, size_t to_size,
                                   PyObject *buffer,
                                   size_t from_start, size_t from_size));

  INLINE vector_uchar get_data() const;
  void set_data(const vector_uchar &data);
  INLINE vector_uchar get_subdata(size_t start, size_t size) const;
  void set_subdata(size_t start, size_t size, const vector_uchar &data);

  INLINE void mark_used() const;

private:
  PT(GeomVertexArrayData) _object;
  Thread *const _current_thread;
  GeomVertexArrayData::CData *_cdata;
  bool _writable;

public:
  static TypeHandle get_class_type() {
    return _type_handle;
  }
  static void init_type() {
    ReferenceCount::init_type();
    register_type(_type_handle, "GeomVertexArrayDataHandle",
                  ReferenceCount::get_class_type());
  }

private:
  static TypeHandle _type_handle;

  friend class Geom;
  friend class GeomPrimitive;
  friend class GeomVertexData;
  friend class GeomVertexDataPipelineReader;
  friend class GeomVertexDataPipelineWriter;
  friend class GeomVertexArrayData;
};

INLINE std::ostream &operator << (std::ostream &out, const GeomVertexArrayData &obj);

#include "geomVertexArrayData.I"

#endif
