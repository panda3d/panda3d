/**
 * PANDA 3D SOFTWARE
 * Copyright (c) Carnegie Mellon University.  All rights reserved.
 *
 * All use of this software is subject to the terms of the revised BSD
 * license.  You should have received a copy of this license along
 * with this source code in a file named "LICENSE."
 *
 * @file xFileDataObject.cxx
 * @author drose
 * @date 2004-10-03
 */

#include "xFileDataObject.h"
#include "xFileTemplate.h"
#include "xFile.h"
#include "xFileDataNodeTemplate.h"
#include "xFileDataObjectInteger.h"
#include "xFileDataObjectDouble.h"
#include "xFileDataObjectString.h"
#include "config_xfile.h"
#include "indent.h"

using std::string;

TypeHandle XFileDataObject::_type_handle;

/**
 *
 */
XFileDataObject::
~XFileDataObject() {
}

/**
 * Returns true if this kind of data object is a complex object that can hold
 * nested data elements, false otherwise.
 */
bool XFileDataObject::
is_complex_object() const {
  return false;
}

/**
 * Returns a string that represents the type of object this data object
 * represents.
 */
string XFileDataObject::
get_type_name() const {
  return get_type().get_name();
}

/**
 * Appends a new integer value to the data object, if it makes sense to do so.
 * Normally, this is valid only for a DataObjectArray, or in certain special
 * cases for a DataNodeTemplate.
 */
XFileDataObject &XFileDataObject::
add_int(int int_value) {
  XFileDataObject *object =
    new XFileDataObjectInteger(get_data_def(), int_value);
  add_element(object);
  return *object;
}

/**
 * Appends a new floating-point value to the data object, if it makes sense to
 * do so.  Normally, this is valid only for a DataObjectArray, or in certain
 * special cases for a DataNodeTemplate.
 */
XFileDataObject &XFileDataObject::
add_double(double double_value) {
  XFileDataObject *object =
    new XFileDataObjectDouble(get_data_def(), double_value);
  add_element(object);
  return *object;
}

/**
 * Appends a new string value to the data object, if it makes sense to do so.
 * Normally, this is valid only for a DataObjectArray, or in certain special
 * cases for a DataNodeTemplate.
 */
XFileDataObject &XFileDataObject::
add_string(const string &string_value) {
  XFileDataObject *object =
    new XFileDataObjectString(get_data_def(), string_value);
  add_element(object);
  return *object;
}

/**
 * Appends a new Vector instance.
 */
XFileDataObject &XFileDataObject::
add_Vector(XFile *x_file, const LVecBase3d &vector) {
  XFileTemplate *xtemplate = XFile::find_standard_template("Vector");
  nassertr(xtemplate != nullptr, *this);
  XFileDataNodeTemplate *node =
    new XFileDataNodeTemplate(x_file, "", xtemplate);
  add_element(node);
  node->zero_fill();

  node->set(vector);

  return *node;
}

/**
 * Appends a new MeshFace instance.
 */
XFileDataObject &XFileDataObject::
add_MeshFace(XFile *x_file) {
  XFileTemplate *xtemplate = XFile::find_standard_template("MeshFace");
  nassertr(xtemplate != nullptr, *this);
  XFileDataNodeTemplate *node =
    new XFileDataNodeTemplate(x_file, "", xtemplate);
  add_element(node);
  node->zero_fill();

  return *node;
}

/**
 * Appends a new IndexedColor instance.
 */
XFileDataObject &XFileDataObject::
add_IndexedColor(XFile *x_file, int index, const LColor &color) {
  XFileTemplate *xtemplate = XFile::find_standard_template("IndexedColor");
  nassertr(xtemplate != nullptr, *this);
  XFileDataNodeTemplate *node =
    new XFileDataNodeTemplate(x_file, "", xtemplate);
  add_element(node);
  node->zero_fill();

  (*node)["index"] = index;
  (*node)["indexColor"] = LCAST(double, color);

  return *node;
}

/**
 * Appends a new Coords2d instance.
 */
XFileDataObject &XFileDataObject::
add_Coords2d(XFile *x_file, const LVecBase2d &coords) {
  XFileTemplate *xtemplate = XFile::find_standard_template("Coords2d");
  nassertr(xtemplate != nullptr, *this);
  XFileDataNodeTemplate *node =
    new XFileDataNodeTemplate(x_file, "", xtemplate);
  add_element(node);
  node->zero_fill();

  node->set(coords);

  return *node;
}

/**
 * Adds the indicated element as a nested data element, if this data object
 * type supports it.  Returns true if added successfully, false if the data
 * object type does not support nested data elements.
 */
bool XFileDataObject::
add_element(XFileDataObject *element) {
  return false;
}

/**
 * Writes a suitable representation of this node to an .x file in text mode.
 */
void XFileDataObject::
output_data(std::ostream &out) const {
  out << "(" << get_type() << "::output_data() not implemented.)";
}

/**
 * Writes a suitable representation of this node to an .x file in text mode.
 */
void XFileDataObject::
write_data(std::ostream &out, int indent_level, const char *) const {
  indent(out, indent_level)
    << "(" << get_type() << "::write_data() not implemented.)\n";
}

/**
 * Sets the object's value as an integer, if this is legal.
 */
void XFileDataObject::
set_int_value(int int_value) {
  xfile_cat.error()
    << get_type_name() << " does not support integer values.\n";
}

/**
 * Sets the object's value as a floating-point number, if this is legal.
 */
void XFileDataObject::
set_double_value(double double_value) {
  xfile_cat.error()
    << get_type_name() << " does not support floating-point values.\n";
}

/**
 * Sets the object's value as a string, if this is legal.
 */
void XFileDataObject::
set_string_value(const string &string_value) {
  xfile_cat.error()
    << get_type_name() << " does not support string values.\n";
}

/**
 * Stores the indicated array of doubles in the nested elements within this
 * object.  There must be exactly the indicated number of nested values, and
 * they must all accept a double.
 */
void XFileDataObject::
store_double_array(int num_elements, const double *values) {
  if (get_num_elements() != num_elements) {
    xfile_cat.error()
      << get_type_name() << " does not accept "
      << num_elements << " values.\n";
    return;
  }

  for (int i = 0; i < num_elements; i++) {
    get_element(i)->set_double_value(values[i]);
  }
}


/**
 * Returns the object's representation as an integer, if it has one.
 */
int XFileDataObject::
get_int_value() const {
  return 0;
}

/**
 * Returns the object's representation as a double, if it has one.
 */
double XFileDataObject::
get_double_value() const {
  return 0.0;
}

/**
 * Returns the object's representation as a string, if it has one.
 */
string XFileDataObject::
get_string_value() const {
  return string();
}

/**
 * Fills the indicated array of doubles with the values from the nested
 * elements within this object.  There must be exactly the indicated number of
 * nested values, and they must all return a double.
 */
void XFileDataObject::
get_double_array(int num_elements, double *values) const {
  if (get_num_elements() != num_elements) {
    xfile_cat.error()
      << get_type_name() << " does not contain "
      << num_elements << " values.\n";
    return;
  }

  for (int i = 0; i < num_elements; i++) {
    values[i] = ((XFileDataObject *)this)->get_element(i)->get_double_value();
  }
}

/**
 * Returns the number of nested data elements within the object.  This may be,
 * e.g.  the size of the array, if it is an array.
 */
int XFileDataObject::
get_num_elements() const {
  return 0;
}

/**
 * Returns the nth nested data element within the object.
 */
XFileDataObject *XFileDataObject::
get_element(int n) {
  xfile_cat.warning()
    << "Looking for [" << n << "] within data object of type "
    << get_type_name() << ", does not support nested objects.\n";
  return nullptr;
}

/**
 * Returns the nested data element within the object that has the indicated
 * name.
 */
XFileDataObject *XFileDataObject::
get_element(const string &name) {
  xfile_cat.warning()
    << "Looking for [\"" << name << "\"] within data object of type "
    << get_type_name() << ", does not support nested objects.\n";
  return nullptr;
}
