/**
 * PANDA 3D SOFTWARE
 * Copyright (c) Carnegie Mellon University.  All rights reserved.
 *
 * All use of this software is subject to the terms of the revised BSD
 * license.  You should have received a copy of this license along
 * with this source code in a file named "LICENSE."
 *
 * @file eggNode_ext.cxx
 * @author rdb
 * @date 2021-01-01
 */

#include "eggNode_ext.h"
#include "eggData.h"

#ifdef HAVE_PYTHON

/**
 * Implements pickle support.
 */
PyObject *Extension<EggNode>::
__reduce__() const {
  extern struct Dtool_PyTypedObject Dtool_EggNode;

  // Find the parse_egg_node function in this module.
  PyObject *module_name = PyObject_GetAttrString((PyObject *)&Dtool_EggNode, "__module__");
  nassertr_always(module_name != nullptr, nullptr);

  PyObject *module = PyImport_GetModule(module_name);
  Py_DECREF(module_name);
  nassertr_always(module != nullptr, nullptr);

  PyObject *func;
  if (_this->is_of_type(EggData::get_class_type())) {
    func = PyObject_GetAttrString(module, "parse_egg_data");
  } else {
    func = PyObject_GetAttrString(module, "parse_egg_node");
  }
  Py_DECREF(module);
  nassertr_always(func != nullptr, nullptr);

  // Get the egg syntax to pass to the parse_egg_node function.
  std::ostringstream stream;
  _this->write(stream, INT_MIN);
  std::string data = stream.str();
  size_t length = data.size();

  // Trim trailing whitespace.
  while (length > 0 && isspace(data[length - 1])) {
    --length;
  }

  return Py_BuildValue("N(s#)", func, data.data(), (Py_ssize_t)length);
}

/**
 * Parses an EggData from the raw egg syntax.
 */
PT(EggData) parse_egg_data(const std::string &egg_syntax) {
  PT(EggData) data = new EggData;
  data->set_auto_resolve_externals(false);

  std::istringstream in(egg_syntax);

  if (!data->read(in)) {
    PyErr_Format(PyExc_RuntimeError, "failed to parse egg data");
    return nullptr;
  }

  return data;
}

/**
 * Parses a single egg node from the raw egg syntax.
 */
PT(EggNode) parse_egg_node(const std::string &egg_syntax) {
  PT(EggData) data = parse_egg_data(egg_syntax);
  if (data == nullptr) {
    return nullptr;
  }

  if (data->size() != 1) {
    PyErr_Format(PyExc_RuntimeError, "expected exactly one node");
    return nullptr;
  }

  return data->remove_child(data->get_first_child());
} 

#endif
