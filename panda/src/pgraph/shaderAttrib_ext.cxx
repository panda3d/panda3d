/**
 * PANDA 3D SOFTWARE
 * Copyright (c) Carnegie Mellon University.  All rights reserved.
 *
 * All use of this software is subject to the terms of the revised BSD
 * license.  You should have received a copy of this license along
 * with this source code in a file named "LICENSE."
 *
 * @file shaderAttrib_ext.cxx
 * @author rdb
 * @date 2017-10-08
 */

#include "shaderAttrib_ext.h"
#include "shaderInput_ext.h"

#ifdef HAVE_PYTHON

/**
 * Returns a new ShaderAttrib with the given shader input set.
 */
CPT(RenderAttrib) Extension<ShaderAttrib>::
set_shader_input(CPT_InternalName name, PyObject *value, int priority) const {
  ShaderAttrib *attrib = new ShaderAttrib(*_this);

  ShaderInput &input = attrib->_inputs[name];
  invoke_extension(&input).__init__(std::move(name), value);

  return ShaderAttrib::return_new(attrib);
}

/**
 * Returns a new ShaderAttrib with the given shader inputs set.  This is a
 * more efficient way to set multiple shader inputs than calling
 * set_shader_input multiple times.
 */
CPT(RenderAttrib) Extension<ShaderAttrib>::
set_shader_inputs(PyObject *args, PyObject *kwargs) const {
  if (PyObject_Size(args) > 0) {
    Dtool_Raise_TypeError("ShaderAttrib.set_shader_inputs takes only keyword arguments");
    return nullptr;
  }

  ShaderAttrib *attrib = new ShaderAttrib(*_this);

  PyObject *key, *value;
  Py_ssize_t pos = 0;

  while (PyDict_Next(kwargs, &pos, &key, &value)) {
    char *buffer;
    Py_ssize_t length;
#if PY_MAJOR_VERSION >= 3
    buffer = (char *)PyUnicode_AsUTF8AndSize(key, &length);
    if (buffer == nullptr) {
#else
    if (PyString_AsStringAndSize(key, &buffer, &length) == -1) {
#endif
      Dtool_Raise_TypeError("ShaderAttrib.set_shader_inputs accepts only string keywords");
      delete attrib;
      return nullptr;
    }

    CPT_InternalName name(std::string(buffer, length));
    ShaderInput &input = attrib->_inputs[name];
    invoke_extension(&input).__init__(std::move(name), value);
  }

  return ShaderAttrib::return_new(attrib);
}

#endif  // HAVE_PYTHON
