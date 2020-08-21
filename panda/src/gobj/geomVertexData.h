/**
 * PANDA 3D SOFTWARE
 * Copyright (c) Carnegie Mellon University.  All rights reserved.
 *
 * All use of this software is subject to the terms of the revised BSD
 * license.  You should have received a copy of this license along
 * with this source code in a file named "LICENSE."
 *
 * @file geomVertexData.h
 * @author drose
 * @date 2005-03-06
 */

#ifndef GEOMVERTEXDATA_H
#define GEOMVERTEXDATA_H

#include "pandabase.h"
#include "copyOnWriteObject.h"
#include "copyOnWritePointer.h"
#include "geomVertexFormat.h"
#include "geomVertexColumn.h"
#include "geomVertexArrayData.h"
#include "geomEnums.h"
#include "geomCacheEntry.h"
#include "transformTable.h"
#include "transformBlendTable.h"
#include "sliderTable.h"
#include "internalName.h"
#include "cycleData.h"
#include "cycleDataLockedReader.h"
#include "cycleDataReader.h"
#include "cycleDataWriter.h"
#include "cycleDataStageReader.h"
#include "cycleDataStageWriter.h"
#include "pipelineCycler.h"
#include "pStatCollector.h"
#include "pointerTo.h"
#include "pmap.h"
#include "pvector.h"
#include "deletedChain.h"

class FactoryParams;
class GeomVertexColumn;
class GeomVertexRewriter;

/**
 * This defines the actual numeric vertex data stored in a Geom, in the
 * structure defined by a particular GeomVertexFormat object.
 *
 * The data consists of one or more arrays, each of which in turn consists of
 * a series of rows, one per vertex.  All arrays should have the same number
 * of rows; each vertex is defined by the column data from a particular row
 * across all arrays.
 *
 * Often, there will be only one array per Geom, and the various columns
 * defined in the GeomVertexFormat will be interleaved within that array.
 * However, it is also possible to have multiple different arrays, with a
 * certain subset of the total columns defined in each array.
 *
 * However the data is distributed, the effect is of a single table of
 * vertices, where each vertex is represented by one row of the table.
 *
 * In general, application code should not attempt to directly manipulate the
 * vertex data through this structure; instead, use the GeomVertexReader,
 * GeomVertexWriter, and GeomVertexRewriter objects to read and write vertex
 * data at a high level.
 */
class EXPCL_PANDA_GOBJ GeomVertexData : public CopyOnWriteObject, public GeomEnums {
private:
  GeomVertexData();
protected:
  virtual PT(CopyOnWriteObject) make_cow_copy();

PUBLISHED:
  explicit GeomVertexData(const std::string &name,
                          const GeomVertexFormat *format,
                          UsageHint usage_hint);
  GeomVertexData(const GeomVertexData &copy);
  explicit GeomVertexData(const GeomVertexData &copy,
                          const GeomVertexFormat *format);
  void operator = (const GeomVertexData &copy);
  virtual ~GeomVertexData();
  ALLOC_DELETED_CHAIN(GeomVertexData);

  int compare_to(const GeomVertexData &other) const;

  INLINE const std::string &get_name() const;
  void set_name(const std::string &name);
  MAKE_PROPERTY(name, get_name, set_name);

  INLINE UsageHint get_usage_hint() const;
  void set_usage_hint(UsageHint usage_hint);
  MAKE_PROPERTY(usage_hint, get_usage_hint, set_usage_hint);

  INLINE const GeomVertexFormat *get_format() const;
  void set_format(const GeomVertexFormat *format);
  void unclean_set_format(const GeomVertexFormat *format);
  MAKE_PROPERTY(format, get_format, set_format);

  INLINE bool has_column(const InternalName *name) const;

  INLINE int get_num_rows() const;
  INLINE bool set_num_rows(int n);
  INLINE bool unclean_set_num_rows(int n);
  INLINE bool reserve_num_rows(int n);
  void clear_rows();

  INLINE size_t get_num_arrays() const;
  INLINE CPT(GeomVertexArrayData) get_array(size_t i) const;
  INLINE CPT(GeomVertexArrayDataHandle) get_array_handle(size_t i) const;
  MAKE_SEQ(get_arrays, get_num_arrays, get_array);
  INLINE PT(GeomVertexArrayData) modify_array(size_t i);
  INLINE PT(GeomVertexArrayDataHandle) modify_array_handle(size_t i);
  INLINE void set_array(size_t i, const GeomVertexArrayData *array);
  MAKE_SEQ_PROPERTY(arrays, get_num_arrays, get_array, set_array);

  INLINE const TransformTable *get_transform_table() const;
  void set_transform_table(const TransformTable *table);
  INLINE void clear_transform_table();
  MAKE_PROPERTY(transform_table, get_transform_table, set_transform_table);

  INLINE CPT(TransformBlendTable) get_transform_blend_table() const;
  PT(TransformBlendTable) modify_transform_blend_table();
  void set_transform_blend_table(const TransformBlendTable *table);
  INLINE void clear_transform_blend_table();

  INLINE const SliderTable *get_slider_table() const;
  void set_slider_table(const SliderTable *table);
  INLINE void clear_slider_table();
  MAKE_PROPERTY(slider_table, get_slider_table, set_slider_table);

  INLINE int get_num_bytes() const;
  INLINE UpdateSeq get_modified(Thread *current_thread = Thread::get_current_thread()) const;
  MAKE_PROPERTY(num_bytes, get_num_bytes);
  MAKE_PROPERTY(modified, get_modified);

  bool request_resident() const;

  void copy_from(const GeomVertexData *source, bool keep_data_objects,
                 Thread *current_thread = Thread::get_current_thread());
  void copy_row_from(int dest_row, const GeomVertexData *source,
                     int source_row, Thread *current_thread);
  CPT(GeomVertexData) convert_to(const GeomVertexFormat *new_format) const;
  CPT(GeomVertexData)
    scale_color(const LVecBase4 &color_scale) const;
  CPT(GeomVertexData)
    scale_color(const LVecBase4 &color_scale, int num_components,
                NumericType numeric_type, Contents contents) const;
  CPT(GeomVertexData)
    set_color(const LColor &color) const;
  CPT(GeomVertexData)
    set_color(const LColor &color, int num_components,
              NumericType numeric_type, Contents contents) const;

  CPT(GeomVertexData) reverse_normals() const;

  CPT(GeomVertexData) animate_vertices(bool force, Thread *current_thread) const;
  void clear_animated_vertices();
  void transform_vertices(const LMatrix4 &mat);
  void transform_vertices(const LMatrix4 &mat, int begin_row, int end_row);
  void transform_vertices(const LMatrix4 &mat, const SparseArray &rows);

  PT(GeomVertexData)
    replace_column(InternalName *name, int num_components,
                   NumericType numeric_type, Contents contents) const;

  void output(std::ostream &out) const;
  void write(std::ostream &out, int indent_level = 0) const;
  void describe_vertex(std::ostream &out, int row) const;

  void clear_cache();
  void clear_cache_stage();

public:
  static INLINE uint32_t pack_abcd(unsigned int a, unsigned int b,
                                    unsigned int c, unsigned int d);
  static INLINE unsigned int unpack_abcd_a(uint32_t data);
  static INLINE unsigned int unpack_abcd_b(uint32_t data);
  static INLINE unsigned int unpack_abcd_c(uint32_t data);
  static INLINE unsigned int unpack_abcd_d(uint32_t data);

  static INLINE uint32_t pack_ufloat(float a, float b, float c);
  static INLINE float unpack_ufloat_a(uint32_t data);
  static INLINE float unpack_ufloat_b(uint32_t data);
  static INLINE float unpack_ufloat_c(uint32_t data);

private:
  static void do_set_color(GeomVertexData *vdata, const LColor &color);

  static void bytewise_copy(unsigned char *to, int to_stride,
                            const unsigned char *from, int from_stride,
                            const GeomVertexColumn *from_type,
                            int num_records);
  static void
  packed_argb_to_uint8_rgba(unsigned char *to, int to_stride,
                            const unsigned char *from, int from_stride,
                            int num_records);
  static void
  uint8_rgba_to_packed_argb(unsigned char *to, int to_stride,
                            const unsigned char *from, int from_stride,
                            int num_records);

  typedef pmap<const VertexTransform *, int> TransformMap;
  INLINE static int
  add_transform(TransformTable *table, const VertexTransform *transform,
                TransformMap &already_added);

private:
  std::string _name;

  typedef pvector< COWPT(GeomVertexArrayData) > Arrays;

  // The pipelined data with each CacheEntry.
  class EXPCL_PANDA_GOBJ CDataCache : public CycleData {
  public:
    INLINE CDataCache();
    INLINE CDataCache(const CDataCache &copy);
    ALLOC_DELETED_CHAIN(CDataCache);
    virtual CycleData *make_copy() const;
    virtual TypeHandle get_parent_type() const {
      return GeomVertexData::get_class_type();
    }

    CPT(GeomVertexData) _result;

  public:
    static TypeHandle get_class_type() {
      return _type_handle;
    }
    static void init_type() {
      register_type(_type_handle, "GeomVertexData::CDataCache");
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
  class EXPCL_PANDA_GOBJ CacheKey {
  public:
    INLINE CacheKey(const GeomVertexFormat *modifier);
    INLINE CacheKey(const CacheKey &copy);
    INLINE CacheKey(CacheKey &&from) noexcept;

    INLINE bool operator < (const CacheKey &other) const;

    CPT(GeomVertexFormat) _modifier;
  };
  // It is not clear why MSVC7 needs this class to be public.
  class EXPCL_PANDA_GOBJ CacheEntry : public GeomCacheEntry {
  public:
    INLINE CacheEntry(GeomVertexData *source,
                      const GeomVertexFormat *modifier);
    INLINE CacheEntry(GeomVertexData *source, const CacheKey &key);
    INLINE CacheEntry(GeomVertexData *source, CacheKey &&key) noexcept;

    ALLOC_DELETED_CHAIN(CacheEntry);

    virtual void evict_callback();
    virtual void output(std::ostream &out) const;

    GeomVertexData *_source;  // A back pointer to the containing data.
    CacheKey _key;

    PipelineCycler<CDataCache> _cycler;

  public:
    static TypeHandle get_class_type() {
      return _type_handle;
    }
    static void init_type() {
      GeomCacheEntry::init_type();
      register_type(_type_handle, "GeomVertexData::CacheEntry",
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
    INLINE CData(const GeomVertexFormat *format, UsageHint usage_hint);

    ALLOC_DELETED_CHAIN(CData);
    virtual CycleData *make_copy() const;
    virtual void write_datagram(BamWriter *manager, Datagram &dg) const;
    virtual int complete_pointers(TypedWritable **plist, BamReader *manager);
    virtual void fillin(DatagramIterator &scan, BamReader *manager);
    virtual TypeHandle get_parent_type() const {
      return GeomVertexData::get_class_type();
    }

    UsageHint _usage_hint;
    CPT(GeomVertexFormat) _format;
    Arrays _arrays;
    CPT(TransformTable) _transform_table;
    COWPT(TransformBlendTable) _transform_blend_table;
    CPT(SliderTable) _slider_table;
    PT(GeomVertexData) _animated_vertices;
    UpdateSeq _animated_vertices_modified;
    UpdateSeq _modified;

  public:
    static TypeHandle get_class_type() {
      return _type_handle;
    }
    static void init_type() {
      register_type(_type_handle, "GeomVertexData::CData");
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

private:
  void update_animated_vertices(CData *cdata, Thread *current_thread);
  void do_transform_point_column(const GeomVertexFormat *format, GeomVertexRewriter &data,
                                 const LMatrix4 &mat, int begin_row, int end_row);
  void do_transform_vector_column(const GeomVertexFormat *format, GeomVertexRewriter &data,
                                  const LMatrix4 &mat, int begin_row, int end_row);
  static void table_xform_point3f(unsigned char *datat, size_t num_rows,
                                  size_t stride, const LMatrix4f &matf);
  static void table_xform_normal3f(unsigned char *datat, size_t num_rows,
                                   size_t stride, const LMatrix4f &matf);
  static void table_xform_vector3f(unsigned char *datat, size_t num_rows,
                                   size_t stride, const LMatrix4f &matf);
  static void table_xform_vecbase4f(unsigned char *datat, size_t num_rows,
                                    size_t stride, const LMatrix4f &matf);

  static PStatCollector _convert_pcollector;
  static PStatCollector _scale_color_pcollector;
  static PStatCollector _set_color_pcollector;
  static PStatCollector _animation_pcollector;

  PStatCollector _char_pcollector;
  PStatCollector _skinning_pcollector;
  PStatCollector _morphs_pcollector;
  PStatCollector _blends_pcollector;

public:
  static void register_with_read_factory();
  virtual void write_datagram(BamWriter *manager, Datagram &dg);
  virtual int complete_pointers(TypedWritable **plist, BamReader *manager);
  virtual bool require_fully_complete() const;

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
    register_type(_type_handle, "GeomVertexData",
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
  friend class GeomVertexDataPipelineBase;
  friend class GeomVertexDataPipelineReader;
  friend class GeomVertexDataPipelineWriter;
};

/**
 * The common code from GeomVertexDataPipelineReader and
 * GeomVertexDataPipelineWriter.
 */
class EXPCL_PANDA_GOBJ GeomVertexDataPipelineBase : public GeomEnums {
protected:
  INLINE GeomVertexDataPipelineBase(Thread *current_thread);
  INLINE GeomVertexDataPipelineBase(GeomVertexData *object,
                                    Thread *current_thread,
                                    GeomVertexData::CData *cdata);

public:
  GeomVertexDataPipelineBase(const GeomVertexDataPipelineBase &copy) = delete;
  INLINE ~GeomVertexDataPipelineBase();

  GeomVertexDataPipelineBase &operator = (const GeomVertexDataPipelineBase &copy) = delete;

  INLINE Thread *get_current_thread() const;

  INLINE const GeomVertexFormat *get_format() const;
  INLINE bool has_column(const InternalName *name) const;

  INLINE UsageHint get_usage_hint() const;
  INLINE size_t get_num_arrays() const;
  INLINE CPT(GeomVertexArrayData) get_array(size_t i) const;
  INLINE const TransformTable *get_transform_table() const;
  INLINE CPT(TransformBlendTable) get_transform_blend_table() const;
  INLINE const SliderTable *get_slider_table() const;
  int get_num_bytes() const;
  INLINE UpdateSeq get_modified() const;

protected:
  GeomVertexData *_object;
  Thread *_current_thread;
  GeomVertexData::CData *_cdata;
};

/**
 * Encapsulates the data from a GeomVertexData, pre-fetched for one stage of
 * the pipeline.
 * Does not hold a reference to the GeomVertexData, so make sure it does not
 * go out of scope.
 */
class EXPCL_PANDA_GOBJ GeomVertexDataPipelineReader : public GeomVertexDataPipelineBase {
public:
  INLINE GeomVertexDataPipelineReader(Thread *current_thread);
  INLINE GeomVertexDataPipelineReader(const GeomVertexData *object, Thread *current_thread);

  ALLOC_DELETED_CHAIN(GeomVertexDataPipelineReader);

  INLINE void set_object(const GeomVertexData *object);
  INLINE const GeomVertexData *get_object() const;

  INLINE void check_array_readers() const;
  INLINE const GeomVertexArrayDataHandle *get_array_reader(int i) const;
  int get_num_rows() const;

  bool get_array_info(const InternalName *name,
                      const GeomVertexArrayDataHandle *&array_reader,
                      int &num_values, NumericType &numeric_type,
                      int &start, int &stride) const;

  bool get_array_info(const InternalName *name,
                      const GeomVertexArrayDataHandle *&array_reader,
                      int &num_values, NumericType &numeric_type,
                      bool &normalized, int &start, int &stride, int &divisor,
                      int &num_elements, int &element_stride) const;

  INLINE bool has_vertex() const;
  INLINE bool is_vertex_transformed() const;
  bool get_vertex_info(const GeomVertexArrayDataHandle *&array_reader,
                       int &num_values, NumericType &numeric_type,
                       int &start, int &stride) const;

  INLINE bool has_normal() const;
  bool get_normal_info(const GeomVertexArrayDataHandle *&array_reader,
                       NumericType &numeric_type,
                       int &start, int &stride) const;

  INLINE bool has_color() const;
  bool get_color_info(const GeomVertexArrayDataHandle *&array_reader,
                      int &num_values, NumericType &numeric_type,
                      int &start, int &stride) const;

private:
  void make_array_readers();

  bool _got_array_readers;
  typedef pvector<CPT(GeomVertexArrayDataHandle) > ArrayReaders;
  ArrayReaders _array_readers;

public:
  static TypeHandle get_class_type() {
    return _type_handle;
  }
  static void init_type() {
    register_type(_type_handle, "GeomVertexDataPipelineReader");
  }

private:
  static TypeHandle _type_handle;
};

/**
 * Encapsulates the data from a GeomVertexData, pre-fetched for one stage of
 * the pipeline.
 * Does not hold a reference to the GeomVertexData, so make sure it does not
 * go out of scope.
 */
class EXPCL_PANDA_GOBJ GeomVertexDataPipelineWriter : public GeomVertexDataPipelineBase {
public:
  INLINE GeomVertexDataPipelineWriter(GeomVertexData *object, bool force_to_0,
                                      Thread *current_thread);

  INLINE ~GeomVertexDataPipelineWriter();
  ALLOC_DELETED_CHAIN(GeomVertexDataPipelineWriter);

  INLINE GeomVertexData *get_object() const;

  INLINE void check_array_writers() const;
  INLINE GeomVertexArrayDataHandle *get_array_writer(size_t i) const;

  PT(GeomVertexArrayData) modify_array(size_t i);
  void set_array(size_t i, const GeomVertexArrayData *array);

  int get_num_rows() const;
  bool set_num_rows(int n);
  bool unclean_set_num_rows(int n);
  bool reserve_num_rows(int n);

  void copy_row_from(int dest_row, const GeomVertexDataPipelineReader &source,
                     int source_row);

private:
  void make_array_writers();
  void delete_array_writers();

  bool _got_array_writers;
  typedef pvector<PT(GeomVertexArrayDataHandle) > ArrayWriters;
  ArrayWriters _array_writers;

public:
  static TypeHandle get_class_type() {
    return _type_handle;
  }
  static void init_type() {
    register_type(_type_handle, "GeomVertexDataPipelineWriter");
  }

private:
  static TypeHandle _type_handle;
};

INLINE std::ostream &operator << (std::ostream &out, const GeomVertexData &obj);

#include "geomVertexData.I"

#endif
