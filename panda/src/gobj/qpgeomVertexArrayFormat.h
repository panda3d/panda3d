// Filename: qpgeomVertexArrayFormat.h
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

#ifndef qpGEOMVERTEXARRAYFORMAT_H
#define qpGEOMVERTEXARRAYFORMAT_H

#include "pandabase.h"
#include "typedWritableReferenceCount.h"
#include "qpgeomVertexDataType.h"
#include "pvector.h"
#include "pmap.h"

class qpGeomVertexFormat;
class qpGeomVertexData;
class InternalName;
class FactoryParams;

////////////////////////////////////////////////////////////////////
//       Class : qpGeomVertexArrayFormat
// Description : This describes the structure of a single array within
//               a Geom data.  See GeomVertexFormat for the parent
//               class which collects together all of the individual
//               GeomVertexArrayFormat objects.
//
//               A particular array may include any number of standard
//               or user-defined data types.  All data types consist
//               of a sequence of one or more floating-point numbers;
//               the semantic meaning of the data type is implicit
//               from its name.  The standard array types are named
//               "vertex", "normal", "texcoord", "color", "tangent",
//               and "binormal"; other data may be stored simply by
//               choosing a different name.
//
//               This is part of the experimental Geom rewrite.
////////////////////////////////////////////////////////////////////
class EXPCL_PANDA qpGeomVertexArrayFormat : public TypedWritableReferenceCount {
PUBLISHED:
  qpGeomVertexArrayFormat();
  qpGeomVertexArrayFormat(const qpGeomVertexArrayFormat &copy);
  qpGeomVertexArrayFormat(const InternalName *name0, int num_components0,
                          qpGeomVertexDataType::NumericType numeric_type0,
                          qpGeomVertexDataType::Contents contents0);
  qpGeomVertexArrayFormat(const InternalName *name0, int num_components0,
                          qpGeomVertexDataType::NumericType numeric_type0,
                          qpGeomVertexDataType::Contents contents0,
                          const InternalName *name1, int num_components1,
                          qpGeomVertexDataType::NumericType numeric_type1,
                          qpGeomVertexDataType::Contents contents1);
  qpGeomVertexArrayFormat(const InternalName *name0, int num_components0,
                          qpGeomVertexDataType::NumericType numeric_type0,
                          qpGeomVertexDataType::Contents contents0,
                          const InternalName *name1, int num_components1,
                          qpGeomVertexDataType::NumericType numeric_type1,
                          qpGeomVertexDataType::Contents contents1,
                          const InternalName *name2, int num_components2,
                          qpGeomVertexDataType::NumericType numeric_type2,
                          qpGeomVertexDataType::Contents contents2);
  qpGeomVertexArrayFormat(const InternalName *name0, int num_components0,
                          qpGeomVertexDataType::NumericType numeric_type0,
                          qpGeomVertexDataType::Contents contents0,
                          const InternalName *name1, int num_components1,
                          qpGeomVertexDataType::NumericType numeric_type1,
                          qpGeomVertexDataType::Contents contents1,
                          const InternalName *name2, int num_components2,
                          qpGeomVertexDataType::NumericType numeric_type2,
                          qpGeomVertexDataType::Contents contents2,
                          const InternalName *name3, int num_components3,
                          qpGeomVertexDataType::NumericType numeric_type3,
                          qpGeomVertexDataType::Contents contents3);
  void operator = (const qpGeomVertexArrayFormat &copy);
  ~qpGeomVertexArrayFormat();

  INLINE bool is_registered() const;

  INLINE int get_stride() const;
  INLINE void set_stride(int stride);

  INLINE int get_total_bytes() const;
  INLINE int get_pad_to() const;

  int add_data_type(const InternalName *name, int num_components,
                    qpGeomVertexDataType::NumericType numeric_type,
                    qpGeomVertexDataType::Contents contents,
                    int start = -1);
  int add_data_type(const qpGeomVertexDataType &data_type);
  void remove_data_type(const InternalName *name);
  void clear_data_types();

  INLINE int get_num_data_types() const;
  INLINE const qpGeomVertexDataType *get_data_type(int i) const;

  const qpGeomVertexDataType *get_data_type(const InternalName *name) const;
  const qpGeomVertexDataType *get_data_type(int start_byte, int num_bytes) const;
  INLINE bool has_data_type(const InternalName *name) const;

  bool is_data_subset_of(const qpGeomVertexArrayFormat &other) const;

  void output(ostream &out) const;
  void write(ostream &out, int indent_level = 0) const;
  void write_with_data(ostream &out, int indent_level, 
                       const qpGeomVertexData *data, int array_index) const;

public:
  int compare_to(const qpGeomVertexArrayFormat &other) const;

private:
  INLINE void consider_sort_data_types() const;
  void sort_data_types();
  void do_register();

  bool _is_registered;
  const qpGeomVertexFormat *_root_format;
  const qpGeomVertexFormat *_parent_format;

  int _stride;
  int _total_bytes;
  int _pad_to;

  typedef pvector<qpGeomVertexDataType *> DataTypes;
  DataTypes _data_types;
  bool _data_types_unsorted;

  typedef pmap<const InternalName *, qpGeomVertexDataType *> DataTypesByName;
  DataTypesByName _data_types_by_name;

public:
  static void register_with_read_factory();
  virtual void write_datagram(BamWriter *manager, Datagram &dg);
  virtual int complete_pointers(TypedWritable **plist, BamReader *manager);

protected:
  static TypedWritable *make_from_bam(const FactoryParams &params);
  void fillin(DatagramIterator &scan, BamReader *manager);

public:
  static TypeHandle get_class_type() {
    return _type_handle;
  }
  static void init_type() {
    TypedWritableReferenceCount::init_type();
    register_type(_type_handle, "qpGeomVertexArrayFormat",
                  TypedWritableReferenceCount::get_class_type());
  }
  virtual TypeHandle get_type() const {
    return get_class_type();
  }
  virtual TypeHandle force_init_type() {init_type(); return get_class_type();}

private:
  static TypeHandle _type_handle;

  friend class qpGeomVertexFormat;
};

INLINE ostream &operator << (ostream &out, const qpGeomVertexArrayFormat &obj);

#include "qpgeomVertexArrayFormat.I"

#endif
