// Filename: geomVertexArrayFormat.h
// Created by:  drose (06Mar05)
//
////////////////////////////////////////////////////////////////////
//
// PANDA 3D SOFTWARE
// Copyright (c) Carnegie Mellon University.  All rights reserved.
//
// All use of this software is subject to the terms of the revised BSD
// license.  You should have received a copy of this license along
// with this source code in a file named "LICENSE."
//
////////////////////////////////////////////////////////////////////

#ifndef GEOMVERTEXARRAYFORMAT_H
#define GEOMVERTEXARRAYFORMAT_H

#include "pandabase.h"
#include "typedWritableReferenceCount.h"
#include "geomVertexColumn.h"
#include "geomEnums.h"
#include "indirectCompareTo.h"
#include "pvector.h"
#include "pmap.h"
#include "lightMutex.h"

class GeomVertexFormat;
class GeomVertexData;
class GeomVertexArrayData;
class InternalName;
class FactoryParams;
class BamWriter;
class BamReader;

////////////////////////////////////////////////////////////////////
//       Class : GeomVertexArrayFormat
// Description : This describes the structure of a single array within
//               a Geom data.  See GeomVertexFormat for the parent
//               class which collects together all of the individual
//               GeomVertexArrayFormat objects.
//
//               A particular array may include any number of standard
//               or user-defined columns.  All columns consist of a
//               sequence of one or more numeric values, packed in any
//               of a variety of formats; the semantic meaning of each
//               column is defined in general with its contents
//               member, and in particular by its name.  The standard
//               array types used most often are named "vertex",
//               "normal", "texcoord", and "color"; other kinds of
//               data may be piggybacked into the data record simply
//               by choosing a unique name.
////////////////////////////////////////////////////////////////////
class EXPCL_PANDA_GOBJ GeomVertexArrayFormat FINAL : public TypedWritableReferenceCount, public GeomEnums {
PUBLISHED:
  GeomVertexArrayFormat();
  GeomVertexArrayFormat(const GeomVertexArrayFormat &copy);
  GeomVertexArrayFormat(CPT_InternalName name0, int num_components0,
                        NumericType numeric_type0, Contents contents0);
  GeomVertexArrayFormat(CPT_InternalName name0, int num_components0,
                        NumericType numeric_type0, Contents contents0,
                        CPT_InternalName name1, int num_components1,
                        NumericType numeric_type1, Contents contents1);
  GeomVertexArrayFormat(CPT_InternalName name0, int num_components0,
                        NumericType numeric_type0, Contents contents0,
                        CPT_InternalName name1, int num_components1,
                        NumericType numeric_type1, Contents contents1,
                        CPT_InternalName name2, int num_components2,
                        NumericType numeric_type2, Contents contents2);
  GeomVertexArrayFormat(CPT_InternalName name0, int num_components0,
                        NumericType numeric_type0, Contents contents0,
                        CPT_InternalName name1, int num_components1,
                        NumericType numeric_type1, Contents contents1,
                        CPT_InternalName name2, int num_components2,
                        NumericType numeric_type2, Contents contents2,
                        CPT_InternalName name3, int num_components3,
                        NumericType numeric_type3, Contents contents3);
  void operator = (const GeomVertexArrayFormat &copy);
  ~GeomVertexArrayFormat();

  virtual bool unref() const;

  INLINE bool is_registered() const;
  INLINE static CPT(GeomVertexArrayFormat) register_format(const GeomVertexArrayFormat *format);

  INLINE int get_stride() const;
  INLINE void set_stride(int stride);

  INLINE int get_pad_to() const;
  INLINE void set_pad_to(int pad_to);

  INLINE int get_divisor() const;
  INLINE void set_divisor(int divisor);

  INLINE int get_total_bytes() const;

  int add_column(CPT_InternalName name, int num_components,
                 NumericType numeric_type, Contents contents,
                 int start = -1, int column_alignment = 0);
  int add_column(const GeomVertexColumn &column);
  void remove_column(const InternalName *name);
  void clear_columns();
  void pack_columns();
  void align_columns_for_animation();

  INLINE int get_num_columns() const;
  INLINE const GeomVertexColumn *get_column(int i) const;
  MAKE_SEQ(get_columns, get_num_columns, get_column);

  const GeomVertexColumn *get_column(const InternalName *name) const;
  const GeomVertexColumn *get_column(int start_byte, int num_bytes) const;
  INLINE bool has_column(const InternalName *name) const;

  bool is_data_subset_of(const GeomVertexArrayFormat &other) const;
  int count_unused_space() const;

  void output(ostream &out) const;
  void write(ostream &out, int indent_level = 0) const;
  void write_with_data(ostream &out, int indent_level,
                       const GeomVertexArrayData *array_data) const;

  string get_format_string(bool pad = true) const;

public:
  int compare_to(const GeomVertexArrayFormat &other) const;

private:
  class Registry;
  INLINE static Registry *get_registry();
  static void make_registry();

  void do_register();
  void do_unregister();

  INLINE void consider_sort_columns() const;
  void sort_columns();

  bool _is_registered;
  int _stride;
  int _total_bytes;
  int _pad_to;
  int _divisor;

  typedef pvector<GeomVertexColumn *> Columns;
  Columns _columns;
  bool _columns_unsorted;

  typedef pmap<const InternalName *, GeomVertexColumn *> ColumnsByName;
  ColumnsByName _columns_by_name;

  // This is the global registry of all currently-in-use array formats.
  typedef pset<GeomVertexArrayFormat *, IndirectCompareTo<GeomVertexArrayFormat> > ArrayFormats;
  class EXPCL_PANDA_GOBJ Registry {
  public:
    Registry();
    CPT(GeomVertexArrayFormat) register_format(GeomVertexArrayFormat *format);
    void unregister_format(GeomVertexArrayFormat *format);

    ArrayFormats _formats;
    LightMutex _lock;
  };

  static Registry *_registry;

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
    register_type(_type_handle, "GeomVertexArrayFormat",
                  TypedWritableReferenceCount::get_class_type());
  }
  virtual TypeHandle get_type() const {
    return get_class_type();
  }
  virtual TypeHandle force_init_type() {init_type(); return get_class_type();}

private:
  static TypeHandle _type_handle;

  friend class GeomVertexFormat;
};

INLINE ostream &operator << (ostream &out, const GeomVertexArrayFormat &obj);

#include "geomVertexArrayFormat.I"

#endif
