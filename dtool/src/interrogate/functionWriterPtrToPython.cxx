/**
 * PANDA 3D SOFTWARE
 * Copyright (c) Carnegie Mellon University.  All rights reserved.
 *
 * All use of this software is subject to the terms of the revised BSD
 * license.  You should have received a copy of this license along
 * with this source code in a file named "LICENSE."
 *
 * @file functionWriterPtrToPython.cxx
 * @author drose
 * @date 2001-09-14
 */

#include "functionWriterPtrToPython.h"
#include "typeManager.h"
#include "interrogateBuilder.h"
#include "interrogate.h"
#include "interfaceMakerPythonObj.h"

#include "cppPointerType.h"

/**
 *
 */
FunctionWriterPtrToPython::
FunctionWriterPtrToPython(CPPType *type) {
  _type = TypeManager::unwrap_const(TypeManager::unwrap_pointer(type));
  _name =
    "to_python_" +
    InterrogateBuilder::clean_identifier(_type->get_local_name(&parser));

  _pointer_type = new CPPPointerType(_type);
}

/**
 *
 */
FunctionWriterPtrToPython::
~FunctionWriterPtrToPython() {
  delete _pointer_type;
}

/**
 * Outputs the prototype for the function.
 */
void FunctionWriterPtrToPython::
write_prototype(std::ostream &out) {
  out << "static PyObject *" << _name << "(";
  _pointer_type->output_instance(out, "addr", &parser);
  out << ", int caller_manages);\n";
}

/**
 * Outputs the code for the function.
 */
void FunctionWriterPtrToPython::
write_code(std::ostream &out) {
  std::string classobj_func = InterfaceMakerPythonObj::get_builder_name(_type);
  out << "static PyObject *\n"
      << _name << "(";
  _pointer_type->output_instance(out, "addr", &parser);
  out << ", int caller_manages) {\n"
      << "  PyObject *" << classobj_func << "();\n"
      << "  PyObject *classobj = " << classobj_func << "();\n"
      << "  PyInstanceObject *instance = (PyInstanceObject *)PyInstance_New(classobj, nullptr, nullptr);\n"
      << "  if (instance != nullptr) {\n"
      << "    PyObject *thisptr = PyLong_FromVoidPtr((void*)addr);\n"
      << "    PyDict_SetItemString(instance->in_dict, \"this\", thisptr);\n"
      << "  }\n"
      << "  return (PyObject *)instance;\n"
      << "}\n\n";
}

/**
 * Returns the type that represents a pointer to the data type.
 */
CPPType *FunctionWriterPtrToPython::
get_pointer_type() const {
  return _pointer_type;
}
