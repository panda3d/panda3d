// Filename: qpgeomVertexData.h
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

#ifndef qpGEOMVERTEXDATA_H
#define qpGEOMVERTEXDATA_H

#include "pandabase.h"
#include "typedWritableReferenceCount.h"
#include "qpgeomVertexFormat.h"
#include "qpgeomVertexColumn.h"
#include "qpgeomVertexArrayData.h"
#include "qpgeomEnums.h"
#include "qpgeomCacheEntry.h"
#include "transformTable.h"
#include "transformBlendTable.h"
#include "sliderTable.h"
#include "internalName.h"
#include "cycleData.h"
#include "cycleDataReader.h"
#include "cycleDataWriter.h"
#include "pipelineCycler.h"
#include "pStatCollector.h"
#include "pointerTo.h"
#include "pmap.h"
#include "pvector.h"

class FactoryParams;
class qpGeomVertexColumn;

////////////////////////////////////////////////////////////////////
//       Class : qpGeomVertexData
// Description : This defines the actual numeric vertex data stored in
//               a Geom, in the structure defined by a particular
//               GeomVertexFormat object.
//
//               The data consists of one or more arrays, each of
//               which in turn consists of a series of rows, one per
//               vertex.  All arrays should have the same number of
//               rows; each vertex is defined by the column data from
//               a particular row across all arrays.
//
//               Often, there will be only one array per Geom, and the
//               various columns defined in the GeomVertexFormat will
//               be interleaved within that array.  However, it is
//               also possible to have multiple different arrays, with
//               a certain subset of the total columns defined in each
//               array.
//
//               However the data is distributed, the effect is of a
//               single table of vertices, where each vertex is
//               represented by one row of the table.
//
//               In general, application code should not attempt to
//               directly manipulate the vertex data through this
//               structure; instead, use the GeomVertexReader,
//               GeomVertexWriter, and GeomVertexRewriter objects to
//               read and write vertex data at a high level.
//
//               This is part of the experimental Geom rewrite.
////////////////////////////////////////////////////////////////////
class EXPCL_PANDA qpGeomVertexData : public TypedWritableReferenceCount, public qpGeomEnums {
private:
  qpGeomVertexData();
PUBLISHED:
  qpGeomVertexData(const string &name,
                   const qpGeomVertexFormat *format, 
                   UsageHint usage_hint);
  qpGeomVertexData(const qpGeomVertexData &copy);
  qpGeomVertexData(const qpGeomVertexData &copy,
                   const qpGeomVertexFormat *format);
  void operator = (const qpGeomVertexData &copy);
  virtual ~qpGeomVertexData();

  INLINE const string &get_name() const;
  void set_name(const string &name);

  INLINE const qpGeomVertexFormat *get_format() const;
  INLINE UsageHint get_usage_hint() const;
  void set_usage_hint(UsageHint usage_hint);

  INLINE bool has_column(const InternalName *name) const;

  int get_num_rows() const;
  INLINE bool set_num_rows(int n);
  void clear_rows();

  INLINE int get_num_arrays() const;
  INLINE const qpGeomVertexArrayData *get_array(int i) const;
  qpGeomVertexArrayData *modify_array(int i);
  void set_array(int i, const qpGeomVertexArrayData *array);

  INLINE const TransformTable *get_transform_table() const;
  void set_transform_table(const TransformTable *table);
  INLINE void clear_transform_table();

  INLINE const TransformBlendTable *get_transform_blend_table() const;
  TransformBlendTable *modify_transform_blend_table();
  void set_transform_blend_table(const TransformBlendTable *table);
  INLINE void clear_transform_blend_table();

  INLINE const SliderTable *get_slider_table() const;
  void set_slider_table(const SliderTable *table);
  INLINE void clear_slider_table();

  int get_num_bytes() const;
  INLINE UpdateSeq get_modified() const;

  void copy_from(const qpGeomVertexData *source, bool keep_data_objects);
  void copy_row_from(int dest_row, const qpGeomVertexData *source, 
                     int source_row);
  CPT(qpGeomVertexData) convert_to(const qpGeomVertexFormat *new_format) const;
  CPT(qpGeomVertexData) 
    scale_color(const LVecBase4f &color_scale) const;
  CPT(qpGeomVertexData) 
    scale_color(const LVecBase4f &color_scale, int num_components,
                NumericType numeric_type, Contents contents) const;
  CPT(qpGeomVertexData) 
    set_color(const Colorf &color) const;
  CPT(qpGeomVertexData) 
    set_color(const Colorf &color, int num_components,
              NumericType numeric_type, Contents contents) const;

  CPT(qpGeomVertexData) animate_vertices() const;

  PT(qpGeomVertexData) 
    replace_column(const InternalName *name, int num_components,
                   NumericType numeric_type, Contents contents) const;

  void output(ostream &out) const;
  void write(ostream &out, int indent_level = 0) const;

  void clear_cache();

public:
  bool get_array_info(const InternalName *name, 
                      const qpGeomVertexArrayData *&array_data,
                      int &num_values, NumericType &numeric_type, 
                      int &start, int &stride) const;

  INLINE bool has_vertex() const;
  INLINE bool is_vertex_transformed() const;
  bool get_vertex_info(const qpGeomVertexArrayData *&array_data,
                       int &num_values, NumericType &numeric_type, 
                       int &start, int &stride) const;

  INLINE bool has_normal() const;
  bool get_normal_info(const qpGeomVertexArrayData *&array_data,
                       NumericType &numeric_type,
                       int &start, int &stride) const;

  INLINE bool has_color() const;
  bool get_color_info(const qpGeomVertexArrayData *&array_data,
                      int &num_values, NumericType &numeric_type, 
                      int &start, int &stride) const;

  static INLINE PN_uint32 pack_abcd(unsigned int a, unsigned int b,
                                    unsigned int c, unsigned int d);
  static INLINE unsigned int unpack_abcd_a(PN_uint32 data);
  static INLINE unsigned int unpack_abcd_b(PN_uint32 data);
  static INLINE unsigned int unpack_abcd_c(PN_uint32 data);
  static INLINE unsigned int unpack_abcd_d(PN_uint32 data);

private:
  static void bytewise_copy(unsigned char *to, int to_stride,
                            const unsigned char *from, int from_stride,
                            const qpGeomVertexColumn *from_type,
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
  string _name;
  CPT(qpGeomVertexFormat) _format;

  typedef pvector< PT(qpGeomVertexArrayData) > Arrays;

  class CacheEntry : public qpGeomCacheEntry {
  public:
    INLINE CacheEntry(const qpGeomVertexFormat *modifier);
    INLINE CacheEntry(qpGeomVertexData *source,
                      const qpGeomVertexFormat *modifier,
                      const qpGeomVertexData *result);
    INLINE bool operator < (const CacheEntry &other) const;

    virtual void evict_callback();
    virtual void output(ostream &out) const;

    qpGeomVertexData *_source;
    CPT(qpGeomVertexFormat) _modifier;
    CPT(qpGeomVertexData) _result;
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

    UsageHint _usage_hint;
    Arrays _arrays;
    CPT(TransformTable) _transform_table;
    PT(TransformBlendTable) _transform_blend_table;
    CPT(SliderTable) _slider_table;
    PT(qpGeomVertexData) _animated_vertices;
    UpdateSeq _animated_vertices_modified;
    UpdateSeq _modified;
    Cache _cache;
  };

  PipelineCycler<CData> _cycler;
  typedef CycleDataReader<CData> CDReader;
  typedef CycleDataWriter<CData> CDWriter;

private:
  bool do_set_num_rows(int n, CDWriter &cdata);
  void update_animated_vertices(CDWriter &cdata);

  static PStatCollector _convert_pcollector;
  static PStatCollector _scale_color_pcollector;
  static PStatCollector _set_color_pcollector;
  static PStatCollector _animation_pcollector;

  PStatCollector _char_pcollector;
  PStatCollector _skinning_pcollector;
  PStatCollector _morphs_pcollector;

public:
  static void register_with_read_factory();
  virtual void write_datagram(BamWriter *manager, Datagram &dg);
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
    TypedWritableReferenceCount::init_type();
    register_type(_type_handle, "qpGeomVertexData",
                  TypedWritableReferenceCount::get_class_type());
  }
  virtual TypeHandle get_type() const {
    return get_class_type();
  }
  virtual TypeHandle force_init_type() {init_type(); return get_class_type();}

private:
  static TypeHandle _type_handle;

  friend class CacheEntry;
};

INLINE ostream &operator << (ostream &out, const qpGeomVertexData &obj);

#include "qpgeomVertexData.I"

#endif
