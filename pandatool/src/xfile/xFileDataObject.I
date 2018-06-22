/**
 * PANDA 3D SOFTWARE
 * Copyright (c) Carnegie Mellon University.  All rights reserved.
 *
 * All use of this software is subject to the terms of the revised BSD
 * license.  You should have received a copy of this license along
 * with this source code in a file named "LICENSE."
 *
 * @file xFileDataObject.I
 * @author drose
 * @date 2004-10-03
 */

/**
 *
 */
INLINE XFileDataObject::
XFileDataObject(const XFileDataDef *data_def) :
  _data_def(data_def)
{
}

/**
 * Returns the data object that this object is represented by, if any, or NULL
 * if there is none.
 */
INLINE const XFileDataDef *XFileDataObject::
get_data_def() const {
  return _data_def;
}

/**
 * Stores the indicated integer value into the object, if it makes sense to do
 * so.  It is an error to call this on an object that cannot accept an integer
 * value.
 */
INLINE void XFileDataObject::
operator = (int int_value) {
  set(int_value);
}

/**
 * Stores the indicated floating-point value into the object, if it makes
 * sense to do so.  It is an error to call this on an object that cannot
 * accept a floating-point value.
 */
INLINE void XFileDataObject::
operator = (double double_value) {
  set(double_value);
}

/**
 * Stores the indicated string value into the object, if it makes sense to do
 * so.  It is an error to call this on an object that cannot accept a string
 * value.
 */
INLINE void XFileDataObject::
operator = (const std::string &string_value) {
  set(string_value);
}

/**
 * Stores the indicated Vec2 value into the object, if it makes sense to do
 * so.  It is an error to call this on an object that does not store two
 * floating-point values.
 */
INLINE void XFileDataObject::
operator = (const LVecBase2d &vec) {
  set(vec);
}

/**
 * Stores the indicated Vec3 value into the object, if it makes sense to do
 * so.  It is an error to call this on an object that does not store three
 * floating-point values.
 */
INLINE void XFileDataObject::
operator = (const LVecBase3d &vec) {
  set(vec);
}

/**
 * Stores the indicated Vec4 value into the object, if it makes sense to do
 * so.  It is an error to call this on an object that does not store four
 * floating-point values.
 */
INLINE void XFileDataObject::
operator = (const LVecBase4d &vec) {
  set(vec);
}

/**
 * Stores the indicated Matrix value into the object, if it makes sense to do
 * so.  It is an error to call this on an object that does not store sixteen
 * floating-point values.
 */
INLINE void XFileDataObject::
operator = (const LMatrix4d &mat) {
  set(mat);
}

/**
 * Stores the indicated integer value into the object, if it makes sense to do
 * so.  It is an error to call this on an object that cannot accept an integer
 * value.
 */
INLINE void XFileDataObject::
set(int int_value) {
  set_int_value(int_value);
}

/**
 * Stores the indicated floating-point value into the object, if it makes
 * sense to do so.  It is an error to call this on an object that cannot
 * accept a floating-point value.
 */
INLINE void XFileDataObject::
set(double double_value) {
  set_double_value(double_value);
}

/**
 * Stores the indicated string value into the object, if it makes sense to do
 * so.  It is an error to call this on an object that cannot accept a string
 * value.
 */
INLINE void XFileDataObject::
set(const std::string &string_value) {
  set_string_value(string_value);
}

/**
 * Stores the indicated Vec2 value into the object, if it makes sense to do
 * so.  It is an error to call this on an object that does not store two
 * floating-point values.
 */
INLINE void XFileDataObject::
set(const LVecBase2d &vec) {
  store_double_array(2, vec.get_data());
}

/**
 * Stores the indicated Vec3 value into the object, if it makes sense to do
 * so.  It is an error to call this on an object that does not store three
 * floating-point values.
 */
INLINE void XFileDataObject::
set(const LVecBase3d &vec) {
  store_double_array(3, vec.get_data());
}

/**
 * Stores the indicated Vec4 value into the object, if it makes sense to do
 * so.  It is an error to call this on an object that does not store four
 * floating-point values.
 */
INLINE void XFileDataObject::
set(const LVecBase4d &vec) {
  store_double_array(4, vec.get_data());
}

/**
 * Stores the indicated Matrix value into the object, if it makes sense to do
 * so.  It is an error to call this on an object that does not store sixteen
 * floating-point values.
 */
INLINE void XFileDataObject::
set(const LMatrix4d &mat) {
  store_double_array(16, mat.get_data());
}

/**
 * Unambiguously returns the object's representation as an integer, or 0 if
 * the object has no integer representation.  See also get_data_def() to
 * determine what kind of representation this object has.
 */
INLINE int XFileDataObject::
i() const {
  return get_int_value();
}

/**
 * Unambiguously returns the object's representation as a double, or 0.0 if
 * the object has no double representation.  See also get_data_def() to
 * determine what kind of representation this object has.
 */
INLINE double XFileDataObject::
d() const {
  return get_double_value();
}

/**
 * Unambiguously returns the object's representation as a string, or empty
 * string if the object has no string representation.  See also get_data_def()
 * to determine what kind of representation this object has.
 */
INLINE std::string XFileDataObject::
s() const {
  return get_string_value();
}

/**
 * Returns the object's representation as an LVecBase2d.  It is an error if
 * the object does not have two nested objects that store a double value.
 */
INLINE LVecBase2d XFileDataObject::
vec2() const {
  LVecBase2d vec;
  get_double_array(2, &vec[0]);
  return vec;
}

/**
 * Returns the object's representation as an LVecBase3d.  It is an error if
 * the object does not have three nested objects that store a double value.
 */
INLINE LVecBase3d XFileDataObject::
vec3() const {
  LVecBase3d vec;
  get_double_array(3, &vec[0]);
  return vec;
}

/**
 * Returns the object's representation as an LVecBase4d.  It is an error if
 * the object does not have four nested objects that store a double value.
 */
INLINE LVecBase4d XFileDataObject::
vec4() const {
  LVecBase4d vec;
  get_double_array(4, &vec[0]);
  return vec;
}

/**
 * Returns the object's representation as an LMatrix4d.  It is an error if the
 * object does not have sixteen nested objects that store a double value.
 */
INLINE LMatrix4d XFileDataObject::
mat4() const {
  LMatrix4d mat;
  get_double_array(16, &mat(0, 0));
  return mat;
}

/**
 * Returns the number of nested data objects within this object.
 */
INLINE int XFileDataObject::
size() const {
  return get_num_elements();
}

/**
 * Returns the nth nested object within this object.  Call get_num_children()
 * to determine the number of nested objects.
 */
INLINE const XFileDataObject &XFileDataObject::
operator [] (int n) const {
  const XFileDataObject *element = ((XFileDataObject *)this)->get_element(n);
  nassertr(element != nullptr, *this);
  return *element;
}

/**
 * Returns the named nested object within this object.  It is an error if the
 * named object does not exist.  Call find_child() instead if there is any
 * doubt.
 */
INLINE const XFileDataObject &XFileDataObject::
operator [] (const std::string &name) const {
  const XFileDataObject *element = ((XFileDataObject *)this)->get_element(name);
  nassertr(element != nullptr, *this);
  return *element;
}

/**
 * Returns the nth nested object within this object.  Call get_num_children()
 * to determine the number of nested objects.
 */
INLINE XFileDataObject &XFileDataObject::
operator [] (int n) {
  XFileDataObject *element = get_element(n);
  nassertr(element != nullptr, *this);
  return *element;
}

/**
 * Returns the named nested object within this object.  It is an error if the
 * named object does not exist.  Call find_child() instead if there is any
 * doubt.
 */
INLINE XFileDataObject &XFileDataObject::
operator [] (const std::string &name) {
  XFileDataObject *element = get_element(name);
  nassertr(element != nullptr, *this);
  return *element;
}

INLINE std::ostream &
operator << (std::ostream &out, const XFileDataObject &data_object) {
  data_object.output_data(out);
  return out;
}
