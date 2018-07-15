/**
 * PANDA 3D SOFTWARE
 * Copyright (c) Carnegie Mellon University.  All rights reserved.
 *
 * All use of this software is subject to the terms of the revised BSD
 * license.  You should have received a copy of this license along
 * with this source code in a file named "LICENSE."
 *
 * @file physxObject.I
 * @author enn0x
 * @date 2009-09-11
 */

/**
 *
 */
INLINE PhysxObject::
PhysxObject() {

  _error_type = ET_empty;
}

/**
 *
 */
INLINE PhysxObject::
~PhysxObject() {

#ifdef HAVE_PYTHON
  // Decrement the reference count of all held Python objects.
  PythonTagData::const_iterator ti;
  for (ti = _python_tag_data.begin(); ti != _python_tag_data.end(); ++ti) {
    PyObject *value = (*ti).second;
    Py_XDECREF(value);
  }
#endif // HAVE_PYTHON
}

#ifdef HAVE_PYTHON
/**
 *
 */
INLINE bool PhysxObject::
has_python_tags() const {

  return _python_tag_data.empty() ? false : true;
}

/**
 *
 */
INLINE void PhysxObject::
set_python_tag(const std::string &key, PyObject *value) {

  Py_XINCREF(value);

  std::pair<PythonTagData::iterator, bool> result;
  result = _python_tag_data.insert(PythonTagData::value_type(key, value));

  if (!result.second) {
    // The insert was unsuccessful; that means the key was already present in
    // the map.  In this case, we should decrement the original value's
    // reference count and replace it with the new object.
    PythonTagData::iterator ti = result.first;
    PyObject *old_value = (*ti).second;
    Py_XDECREF(old_value);
    (*ti).second = value;
  }
}

/**
 *
 */
INLINE PyObject *PhysxObject::
get_python_tag(const std::string &key) const {

  PythonTagData::const_iterator ti;
  ti = _python_tag_data.find(key);

  if (ti != _python_tag_data.end()) {
    PyObject *result = (*ti).second;
    Py_XINCREF(result);
    return result;
  }

  Py_INCREF(Py_None);
  return Py_None;
}

/**
 *
 */
INLINE bool PhysxObject::
has_python_tag(const std::string &key) const {

  PythonTagData::const_iterator ti;
  ti = _python_tag_data.find(key);
  return (ti != _python_tag_data.end());
}

/**
 *
 */
INLINE void PhysxObject::
clear_python_tag(const std::string &key) {

  PythonTagData::iterator ti;
  ti = _python_tag_data.find(key);

  if (ti != _python_tag_data.end()) {
    PyObject *value = (*ti).second;
    Py_XDECREF(value);
    _python_tag_data.erase(ti);
  }
}
#endif // HAVE_PYTHON
