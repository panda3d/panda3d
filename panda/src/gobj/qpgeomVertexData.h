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
#include "transformPalette.h"
#include "transformBlendPalette.h"
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
  void operator = (const qpGeomVertexData &copy);
  virtual ~qpGeomVertexData();

  INLINE const string &get_name() const;
  INLINE void set_name(const string &name);

  INLINE const qpGeomVertexFormat *get_format() const;
  INLINE UsageHint get_usage_hint() const;
  void set_usage_hint(UsageHint usage_hint);

  INLINE bool has_column(const InternalName *name) const;

  int get_num_vertices() const;
  INLINE bool set_num_vertices(int n);
  void clear_vertices();

  INLINE int get_num_arrays() const;
  INLINE const qpGeomVertexArrayData *get_array(int i) const;
  qpGeomVertexArrayData *modify_array(int i);
  void set_array(int i, const qpGeomVertexArrayData *array);

  INLINE const TransformPalette *get_transform_palette() const;
  void set_transform_palette(const TransformPalette *palette);
  INLINE void clear_transform_palette();

  INLINE const TransformBlendPalette *get_transform_blend_palette() const;
  TransformBlendPalette *modify_transform_blend_palette();
  void set_transform_blend_palette(const TransformBlendPalette *palette);
  INLINE void clear_transform_blend_palette();

  INLINE const SliderTable *get_slider_table() const;
  void set_slider_table(const SliderTable *palette);
  INLINE void clear_slider_table();

  int get_num_bytes() const;
  INLINE UpdateSeq get_modified() const;

  void copy_from(const qpGeomVertexData &source, bool keep_data_objects);
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

  INLINE CPT(qpGeomVertexData) animate_vertices() const;

  PT(qpGeomVertexData) 
    replace_column(const InternalName *name, int num_components,
                   NumericType numeric_type, Contents contents,
                   UsageHint usage_hint, bool keep_animation) const;

  void output(ostream &out) const;
  void write(ostream &out, int indent_level = 0) const;

public:
  INLINE CPT(qpGeomVertexData) animate_vertices_cull() const;

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
  CPT(qpGeomVertexData) do_animate_vertices(bool from_app) const;

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
  add_transform(TransformPalette *palette, const VertexTransform *transform,
                TransformMap &already_added);
  
private:
  string _name;
  CPT(qpGeomVertexFormat) _format;

  typedef pvector< PT(qpGeomVertexArrayData) > Arrays;

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
    CPT(TransformPalette) _transform_palette;
    PT(TransformBlendPalette) _transform_blend_palette;
    CPT(SliderTable) _slider_table;
    PT(qpGeomVertexData) _animated_vertices;
    UpdateSeq _animated_vertices_modified;
    UpdateSeq _modified;
  };

  PipelineCycler<CData> _cycler;
  typedef CycleDataReader<CData> CDReader;
  typedef CycleDataWriter<CData> CDWriter;

private:
  bool do_set_num_vertices(int n, CDWriter &cdata);
  void update_animated_vertices(CDWriter &cdata, bool from_app);
  CPT(qpGeomVertexFormat) get_post_animated_format() const;

  static PStatCollector _convert_pcollector;
  static PStatCollector _scale_color_pcollector;
  static PStatCollector _set_color_pcollector;
  static PStatCollector _app_animation_pcollector;
  static PStatCollector _cull_animation_pcollector;

  PStatCollector _app_char_pcollector;
  PStatCollector _cull_char_pcollector;

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
};

INLINE ostream &operator << (ostream &out, const qpGeomVertexData &obj);

#include "qpgeomVertexData.I"

#endif
