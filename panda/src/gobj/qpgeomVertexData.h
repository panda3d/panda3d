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
#include "qpgeomVertexDataType.h"
#include "qpgeomVertexArrayData.h"
#include "qpgeomUsageHint.h"
#include "transformBlendPalette.h"
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
class qpGeomVertexDataType;

////////////////////////////////////////////////////////////////////
//       Class : qpGeomVertexData
// Description : This defines the actual numeric vertex data stored in
//               a Geom, in the structure defined by a particular
//               GeomVertexFormat object.
//
//               The data consists of one or more arrays of floats.
//               Typically, there will be only one array per Geom, and
//               the various data types defined in the
//               GeomVertexFormat will be interleaved throughout that
//               array.  However, it is possible to have multiple
//               different arrays, with different combinations of data
//               types through each one.
//
//               However the data is distributed, the effect is of a
//               table of vertices, with a value for each of the
//               GeomVertexFormat's data types, stored for each
//               vertex.
//
//               This is part of the experimental Geom rewrite.
////////////////////////////////////////////////////////////////////
class EXPCL_PANDA qpGeomVertexData : public TypedWritableReferenceCount {
private:
  qpGeomVertexData();
PUBLISHED:
  qpGeomVertexData(const string &name,
                   const qpGeomVertexFormat *format, 
                   qpGeomUsageHint::UsageHint usage_hint);
  qpGeomVertexData(const qpGeomVertexData &copy);
  void operator = (const qpGeomVertexData &copy);
  virtual ~qpGeomVertexData();

  INLINE const string &get_name() const;
  INLINE void set_name(const string &name);

  INLINE const qpGeomVertexFormat *get_format() const;
  INLINE qpGeomUsageHint::UsageHint get_usage_hint() const;

  int get_num_vertices() const;
  INLINE bool set_num_vertices(int n);
  void clear_vertices();

  INLINE int get_num_arrays() const;
  INLINE const qpGeomVertexArrayData *get_array(int i) const;
  qpGeomVertexArrayData *modify_array(int i);
  void set_array(int i, const qpGeomVertexArrayData *array);

  INLINE const TransformBlendPalette *get_transform_blend_palette() const;
  TransformBlendPalette *modify_transform_blend_palette();
  void set_transform_blend_palette(const TransformBlendPalette *palette);
  INLINE void clear_transform_blend_palette();

  int get_num_bytes() const;
  INLINE UpdateSeq get_modified() const;

  CPT(qpGeomVertexData) convert_to(const qpGeomVertexFormat *new_format) const;
  CPT(qpGeomVertexData) 
    scale_color(const LVecBase4f &color_scale, int num_components,
                qpGeomVertexDataType::NumericType numeric_type,
                qpGeomVertexDataType::Contents contents) const;
  CPT(qpGeomVertexData) 
    set_color(const Colorf &color, int num_components,
              qpGeomVertexDataType::NumericType numeric_type,
              qpGeomVertexDataType::Contents contents) const;

  INLINE CPT(qpGeomVertexData) animate_vertices() const;

  PT(qpGeomVertexData) 
    replace_data_type(const InternalName *name, int num_components,
                      qpGeomVertexDataType::NumericType numeric_type,
                      qpGeomVertexDataType::Contents contents,
                      qpGeomUsageHint::UsageHint usage_hint,
                      bool keep_animation) const;

  void output(ostream &out) const;
  void write(ostream &out, int indent_level = 0) const;

public:
  INLINE CPT(qpGeomVertexData) animate_vertices_cull() const;

  bool get_array_info(const InternalName *name, 
                      const qpGeomVertexArrayData *&array_data,
                      int &num_components,
                      qpGeomVertexDataType::NumericType &numeric_type, 
                      int &start, int &stride) const;

  static INLINE PN_uint32 pack_8888(unsigned int a, unsigned int b,
                                    unsigned int c, unsigned int d);
  static INLINE unsigned int unpack_8888_a(PN_uint32 data);
  static INLINE unsigned int unpack_8888_b(PN_uint32 data);
  static INLINE unsigned int unpack_8888_c(PN_uint32 data);
  static INLINE unsigned int unpack_8888_d(PN_uint32 data);

private:
  CPT(qpGeomVertexData) do_animate_vertices(bool from_app) const;

  static void bytewise_copy(unsigned char *to, int to_stride,
                            const unsigned char *from, int from_stride,
                            const qpGeomVertexDataType *from_type,
                            int num_records);
  static void
  packed_argb_to_uint8_rgba(unsigned char *to, int to_stride,
                            const unsigned char *from, int from_stride,
                            int num_records);
  static void
  uint8_rgba_to_packed_argb(unsigned char *to, int to_stride,
                            const unsigned char *from, int from_stride,
                            int num_records);
  
private:
  string _name;
  CPT(qpGeomVertexFormat) _format;
  qpGeomUsageHint::UsageHint _usage_hint;

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

    Arrays _arrays;
    PT(TransformBlendPalette) _transform_blend_palette;
    PT(qpGeomVertexData) _animated_vertices;
    UpdateSeq _animated_vertices_modified;
    UpdateSeq _modified;
  };

  PipelineCycler<CData> _cycler;
  typedef CycleDataReader<CData> CDReader;
  typedef CycleDataWriter<CData> CDWriter;

private:
  bool do_set_num_vertices(int n, CDWriter &cdata);
  void make_animated_vertices(CDWriter &cdata);
  void update_animated_vertices(CDWriter &cdata, bool from_app);

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
