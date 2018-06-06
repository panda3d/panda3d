/**
 * PANDA 3D SOFTWARE
 * Copyright (c) Carnegie Mellon University.  All rights reserved.
 *
 * All use of this software is subject to the terms of the revised BSD
 * license.  You should have received a copy of this license along
 * with this source code in a file named "LICENSE."
 *
 * @file xFileDataDef.h
 * @author drose
 * @date 2004-10-03
 */

#ifndef XFILEDATADEF_H
#define XFILEDATADEF_H

#include "pandatoolbase.h"
#include "namable.h"
#include "xFileNode.h"
#include "xFileArrayDef.h"
#include "xFileTemplate.h"
#include "xFileDataObject.h"
#include "pvector.h"
#include "pointerTo.h"

/**
 * A definition of a single data element appearing within a template record.
 * This class represents the *definition* of the data element (e.g.  DWORD
 * nVertices); see XFileDataObject for its *value* (e.g.  12).
 */
class XFileDataDef : public XFileNode {
public:
  enum Type {
    T_word,
    T_dword,
    T_float,
    T_double,
    T_char,
    T_uchar,
    T_sword,
    T_sdword,
    T_string,
    T_cstring,
    T_unicode,
    T_template,
  };

  INLINE XFileDataDef(XFile *x_file, const std::string &name,
                      Type type, XFileTemplate *xtemplate = nullptr);
  virtual ~XFileDataDef();

  virtual void clear();
  void add_array_def(const XFileArrayDef &array_def);

  INLINE Type get_data_type() const;
  INLINE XFileTemplate *get_template() const;

  INLINE int get_num_array_defs() const;
  INLINE const XFileArrayDef &get_array_def(int i) const;

  virtual void write_text(std::ostream &out, int indent_level) const;

  virtual bool repack_data(XFileDataObject *object,
                           const XFileParseDataList &parse_data_list,
                           PrevData &prev_data,
                           size_t &index, size_t &sub_index) const;

  virtual bool fill_zero_data(XFileDataObject *object) const;

  virtual bool matches(const XFileNode *other) const;

private:
  typedef PT(XFileDataObject)
    (XFileDataDef::*UnpackMethod)(const XFileParseDataList &parse_data_list,
                                  const PrevData &prev_data,
                                  size_t &index, size_t &sub_index) const;
  typedef PT(XFileDataObject)
    (XFileDataDef::*ZeroFillMethod)() const;

  PT(XFileDataObject)
    unpack_integer_value(const XFileParseDataList &parse_data_list,
                         const PrevData &prev_data,
                         size_t &index, size_t &sub_index) const;
  PT(XFileDataObject)
    unpack_double_value(const XFileParseDataList &parse_data_list,
                        const PrevData &prev_data,
                        size_t &index, size_t &sub_index) const;
  PT(XFileDataObject)
    unpack_string_value(const XFileParseDataList &parse_data_list,
                        const PrevData &prev_data,
                        size_t &index, size_t &sub_index) const;
  PT(XFileDataObject)
    unpack_template_value(const XFileParseDataList &parse_data_list,
                          const PrevData &prev_data,
                          size_t &index, size_t &sub_index) const;

  PT(XFileDataObject)
    unpack_value(const XFileParseDataList &parse_data_list, int array_index,
                 const PrevData &prev_data,
                 size_t &index, size_t &sub_index,
                 UnpackMethod unpack_method) const;

  PT(XFileDataObject) zero_fill_integer_value() const;
  PT(XFileDataObject) zero_fill_double_value() const;
  PT(XFileDataObject) zero_fill_string_value() const;
  PT(XFileDataObject) zero_fill_template_value() const;
  PT(XFileDataObject)
    zero_fill_value(int array_index, ZeroFillMethod zero_fill_method) const;

private:
  Type _type;
  PT(XFileTemplate) _template;

  typedef pvector<XFileArrayDef> ArrayDef;
  ArrayDef _array_def;

public:
  static TypeHandle get_class_type() {
    return _type_handle;
  }
  static void init_type() {
    XFileNode::init_type();
    register_type(_type_handle, "XFileDataDef",
                  XFileNode::get_class_type());
  }
  virtual TypeHandle get_type() const {
    return get_class_type();
  }
  virtual TypeHandle force_init_type() {init_type(); return get_class_type();}

private:
  static TypeHandle _type_handle;
};

#include "xFileDataDef.I"

#endif
