/**
 * PANDA 3D SOFTWARE
 * Copyright (c) Carnegie Mellon University.  All rights reserved.
 *
 * All use of this software is subject to the terms of the revised BSD
 * license.  You should have received a copy of this license along
 * with this source code in a file named "LICENSE."
 *
 * @file xFileDataDef.cxx
 * @author drose
 * @date 2004-10-03
 */

#include "xFileDataDef.h"
#include "indent.h"
#include "xLexerDefs.h"
#include "xFileParseData.h"
#include "xFileDataObjectInteger.h"
#include "xFileDataObjectDouble.h"
#include "xFileDataObjectString.h"
#include "xFileDataNodeTemplate.h"
#include "xFileDataObjectArray.h"
#include "string_utils.h"

TypeHandle XFileDataDef::_type_handle;

/**
 *
 */
XFileDataDef::
~XFileDataDef() {
  clear();
}

/**
 *
 */
void XFileDataDef::
clear() {
  XFileNode::clear();
  _array_def.clear();
}

/**
 * Adds an additional array dimension to the data description.
 */
void XFileDataDef::
add_array_def(const XFileArrayDef &array_def) {
  _array_def.push_back(array_def);
}

/**
 * Writes a suitable representation of this node to an .x file in text mode.
 */
void XFileDataDef::
write_text(std::ostream &out, int indent_level) const {
  indent(out, indent_level);

  if (!_array_def.empty()) {
    out << "array ";
  }

  switch (_type) {
  case T_word:
    out << "WORD";
    break;

  case T_dword:
    out << "DWORD";
    break;

  case T_float:
    out << "FLOAT";
    break;

  case T_double:
    out << "DOUBLE";
    break;

  case T_char:
    out << "CHAR";
    break;

  case T_uchar:
    out << "UCHAR";
    break;

  case T_sword:
    out << "SWORD";
    break;

  case T_sdword:
    out << "SDWORD";
    break;

  case T_string:
    out << "STRING";
    break;

  case T_cstring:
    out << "CSTRING";
    break;

  case T_unicode:
    out << "UNICODE";
    break;

  case T_template:
    out << _template->get_name();
    break;
  }

  if (has_name()) {
    out << " " << get_name();
  }

  ArrayDef::const_iterator ai;
  for (ai = _array_def.begin(); ai != _array_def.end(); ++ai) {
    (*ai).output(out);
  }

  out << ";\n";
}

/**
 * This is called on the template that defines an object, once the data for
 * the object has been parsed.  It is responsible for identifying which
 * component of the template owns each data element, and packing the data
 * elements appropriately back into the object.
 *
 * It returns true on success, or false on an error (e.g.  not enough data
 * elements, mismatched data type).
 */
bool XFileDataDef::
repack_data(XFileDataObject *object,
            const XFileParseDataList &parse_data_list,
            XFileDataDef::PrevData &prev_data,
            size_t &index, size_t &sub_index) const {
  // We'll fill this in with the data value we pack, if any.
  PT(XFileDataObject) data_value;

  // What kind of data element are we expecting?
  switch (_type) {
  case T_word:
  case T_dword:
  case T_char:
  case T_uchar:
  case T_sword:
  case T_sdword:
    // Expected integer data.
    data_value = unpack_value(parse_data_list, 0,
                              prev_data, index, sub_index,
                              &XFileDataDef::unpack_integer_value);
    break;

  case T_float:
  case T_double:
    data_value = unpack_value(parse_data_list, 0,
                              prev_data, index, sub_index,
                              &XFileDataDef::unpack_double_value);
    break;

  case T_string:
  case T_cstring:
  case T_unicode:
    data_value = unpack_value(parse_data_list, 0,
                              prev_data, index, sub_index,
                              &XFileDataDef::unpack_string_value);
    break;

  case T_template:
    data_value = unpack_value(parse_data_list, 0,
                              prev_data, index, sub_index,
                              &XFileDataDef::unpack_template_value);
    break;
  }

  if (data_value != nullptr) {
    object->add_element(data_value);
    prev_data[this] = data_value;
  }

  return XFileNode::repack_data(object, parse_data_list,
                                prev_data, index, sub_index);
}

/**
 * This is similar to repack_data(), except it is used to fill the initial
 * values for a newly-created template object to zero.
 */
bool XFileDataDef::
fill_zero_data(XFileDataObject *object) const {
  PT(XFileDataObject) data_value;

  // What kind of data element are we expecting?
  switch (_type) {
  case T_word:
  case T_dword:
  case T_char:
  case T_uchar:
  case T_sword:
  case T_sdword:
    data_value = zero_fill_value(0, &XFileDataDef::zero_fill_integer_value);
    break;

  case T_float:
  case T_double:
    data_value = zero_fill_value(0, &XFileDataDef::zero_fill_double_value);
    break;

  case T_string:
  case T_cstring:
  case T_unicode:
    data_value = zero_fill_value(0, &XFileDataDef::zero_fill_string_value);
    break;

  case T_template:
    data_value = zero_fill_value(0, &XFileDataDef::zero_fill_template_value);
    break;
  }

  if (data_value != nullptr) {
    object->add_element(data_value);
  }

  return XFileNode::fill_zero_data(object);
}

/**
 * Returns true if the node, particularly a template node, is structurally
 * equivalent to the other node (which must be of the same type).  This checks
 * data element types, but does not compare data element names.
 */
bool XFileDataDef::
matches(const XFileNode *other) const {
  if (!XFileNode::matches(other)) {
    return false;
  }

  const XFileDataDef *data_def = DCAST(XFileDataDef, other);
  if (data_def->get_data_type() != get_data_type()) {
    return false;
  }

  if (get_data_type() == T_template &&
      !get_template()->matches(data_def->get_template())) {
    return false;
  }

  if (data_def->get_num_array_defs() != get_num_array_defs()) {
    return false;
  }

  for (int i = 0; i < get_num_array_defs(); i++) {
    if (!get_array_def(i).matches(data_def->get_array_def(i),
                                  this, data_def)) {
      return false;
    }
  }

  return true;
}


/**
 * Unpacks and returns the next sequential integer value from the
 * parse_data_list.
 */
PT(XFileDataObject) XFileDataDef::
unpack_integer_value(const XFileParseDataList &parse_data_list,
                     const XFileDataDef::PrevData &prev_data,
                     size_t &index, size_t &sub_index) const {
  nassertr(index < parse_data_list._list.size(), nullptr);
  const XFileParseData &parse_data = parse_data_list._list[index];

  PT(XFileDataObject) data_value;

  if ((parse_data._parse_flags & XFileParseData::PF_int) != 0) {
    nassertr(sub_index < parse_data._int_list.size(), nullptr);
    int value = parse_data._int_list[sub_index];
    data_value = new XFileDataObjectInteger(this, value);

    sub_index++;
    if (sub_index >= parse_data._int_list.size()) {
      index++;
      sub_index = 0;
    }

  } else {
    parse_data.yyerror("Expected integer data for " + get_name());
  }

  return data_value;
}

/**
 * Unpacks and returns the next sequential double value from the
 * parse_data_list.
 */
PT(XFileDataObject) XFileDataDef::
unpack_double_value(const XFileParseDataList &parse_data_list,
                    const XFileDataDef::PrevData &prev_data,
                    size_t &index, size_t &sub_index) const {
  nassertr(index < parse_data_list._list.size(), nullptr);
  const XFileParseData &parse_data = parse_data_list._list[index];

  PT(XFileDataObject) data_value;

  if ((parse_data._parse_flags & XFileParseData::PF_double) != 0) {
    nassertr(sub_index < parse_data._double_list.size(), nullptr);
    double value = parse_data._double_list[sub_index];
    data_value = new XFileDataObjectDouble(this, value);

    sub_index++;
    if (sub_index >= parse_data._double_list.size()) {
      index++;
      sub_index = 0;
    }

  } else if ((parse_data._parse_flags & XFileParseData::PF_int) != 0) {
    nassertr(sub_index < parse_data._int_list.size(), nullptr);
    int value = parse_data._int_list[sub_index];
    data_value = new XFileDataObjectDouble(this, value);

    sub_index++;
    if (sub_index >= parse_data._int_list.size()) {
      index++;
      sub_index = 0;
    }

  } else {
    parse_data.yyerror("Expected floating-point data for " + get_name());
  }

  return data_value;
}

/**
 * Unpacks and returns the next sequential string value from the
 * parse_data_list.
 */
PT(XFileDataObject) XFileDataDef::
unpack_string_value(const XFileParseDataList &parse_data_list,
                    const XFileDataDef::PrevData &prev_data,
                    size_t &index, size_t &sub_index) const {
  nassertr(index < parse_data_list._list.size(), nullptr);
  const XFileParseData &parse_data = parse_data_list._list[index];

  PT(XFileDataObject) data_value;

  if ((parse_data._parse_flags & XFileParseData::PF_string) != 0) {
    data_value = new XFileDataObjectString(this, parse_data._string);
    index++;
    sub_index = 0;

  } else {
    parse_data.yyerror("Expected string data for " + get_name());
  }

  return data_value;
}

/**
 * Unpacks a nested template object's data.
 */
PT(XFileDataObject) XFileDataDef::
unpack_template_value(const XFileParseDataList &parse_data_list,
                      const XFileDataDef::PrevData &prev_data,
                      size_t &index, size_t &sub_index) const {
  PT(XFileDataNodeTemplate) data_value =
    new XFileDataNodeTemplate(get_x_file(), get_name(), _template);

  PrevData nested_prev_data(prev_data);
  if (!_template->repack_data(data_value, parse_data_list,
                              nested_prev_data, index, sub_index)) {
    return nullptr;
  }

  return data_value;
}

/**
 * Unpacks and returns the next sequential value, of the type supported by the
 * unpack_method.  If the value is an array type, unpacks all the elements of
 * the array.
 */
PT(XFileDataObject) XFileDataDef::
unpack_value(const XFileParseDataList &parse_data_list, int array_index,
             const XFileDataDef::PrevData &prev_data,
             size_t &index, size_t &sub_index,
             XFileDataDef::UnpackMethod unpack_method) const {
  PT(XFileDataObject) data_value;

  if (array_index == (int)_array_def.size()) {
    if (index >= parse_data_list._list.size()) {
      xyyerror("Not enough data elements in structure at " + get_name());
      return nullptr;
    }
    data_value = (this->*unpack_method)(parse_data_list, prev_data,
                                        index, sub_index);

  } else {
    data_value = new XFileDataObjectArray(this);
    int array_size = _array_def[array_index].get_size(prev_data);

    for (int i = 0; i < array_size; i++) {
      if (index >= parse_data_list._list.size()) {
        xyyerror(std::string("Expected ") + format_string(array_size)
                 + " array elements, found " + format_string(i));
        return data_value;
      }

      PT(XFileDataObject) array_element =
        unpack_value(parse_data_list, array_index + 1,
                     prev_data, index, sub_index,
                     unpack_method);
      if (array_element == nullptr) {
        return data_value;
      }
      data_value->add_element(array_element);
    }
  }

  return data_value;
}

/**
 * Returns a newly-allocated zero integer value.
 */
PT(XFileDataObject) XFileDataDef::
zero_fill_integer_value() const {
  return new XFileDataObjectInteger(this, 0);
}

/**
 * Returns a newly-allocated zero floating-point value.
 */
PT(XFileDataObject) XFileDataDef::
zero_fill_double_value() const {
  return new XFileDataObjectDouble(this, 0.0);
}

/**
 * Returns a newly-allocated empty string value.
 */
PT(XFileDataObject) XFileDataDef::
zero_fill_string_value() const {
  return new XFileDataObjectString(this, "");
}

/**
 * Returns a newly-allocated zero-filled nested template value.
 */
PT(XFileDataObject) XFileDataDef::
zero_fill_template_value() const {
  PT(XFileDataObject) data_value =
    new XFileDataNodeTemplate(get_x_file(), get_name(), _template);
  if (!_template->fill_zero_data(data_value)) {
    return nullptr;
  }

  return data_value;
}

/**
 * Creates a zero-valued element for the next sequential value, of the type
 * returned by the zero_fill_method.  If the value is a fixed-size array type,
 * zero-fills all the elements of the array.
 */
PT(XFileDataObject) XFileDataDef::
zero_fill_value(int array_index,
                XFileDataDef::ZeroFillMethod zero_fill_method) const {
  PT(XFileDataObject) data_value;

  if (array_index == (int)_array_def.size()) {
    data_value = (this->*zero_fill_method)();

  } else {
    data_value = new XFileDataObjectArray(this);
    int array_size = 0;
    if (_array_def[array_index].is_fixed_size()) {
      array_size = _array_def[array_index].get_fixed_size();
    }

    for (int i = 0; i < array_size; i++) {
      PT(XFileDataObject) array_element =
        zero_fill_value(array_index + 1, zero_fill_method);
      if (array_element == nullptr) {
        return nullptr;
      }
      data_value->add_element(array_element);
    }
  }

  return data_value;
}
