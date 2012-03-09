// Filename: functionWriterPtrToPython.cxx
// Created by:  drose (14Sep01)
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

#include "functionWriterPtrToPython.h"
#include "typeManager.h"
#include "interrogateBuilder.h"
#include "interrogate.h"
#include "interfaceMakerPythonObj.h"

#include "cppPointerType.h"

////////////////////////////////////////////////////////////////////
//     Function: FunctionWriterPtrToPython::Constructor
//       Access: Public
//  Description:
////////////////////////////////////////////////////////////////////
FunctionWriterPtrToPython::
FunctionWriterPtrToPython(CPPType *type) {
  _type = TypeManager::unwrap_const(TypeManager::unwrap_pointer(type));
  _name = 
    "to_python_" + 
    InterrogateBuilder::clean_identifier(_type->get_local_name(&parser));

  _pointer_type = new CPPPointerType(_type);
}

////////////////////////////////////////////////////////////////////
//     Function: FunctionWriterPtrToPython::Destructor
//       Access: Public
//  Description:
////////////////////////////////////////////////////////////////////
FunctionWriterPtrToPython::
~FunctionWriterPtrToPython() {
  delete _pointer_type;
}

////////////////////////////////////////////////////////////////////
//     Function: FunctionWriterPtrToPython::write_prototype
//       Access: Public, Virtual
//  Description: Outputs the prototype for the function.
////////////////////////////////////////////////////////////////////
void FunctionWriterPtrToPython::
write_prototype(ostream &out) {
  out << "static PyObject *" << _name << "(";
  _pointer_type->output_instance(out, "addr", &parser);
  out << ", int caller_manages);\n";
}

////////////////////////////////////////////////////////////////////
//     Function: FunctionWriterPtrToPython::write_code
//       Access: Public, Virtual
//  Description: Outputs the code for the function.
////////////////////////////////////////////////////////////////////
void FunctionWriterPtrToPython::
write_code(ostream &out) {
  string classobj_func = InterfaceMakerPythonObj::get_builder_name(_type);
  out << "static PyObject *\n"
      << _name << "(";
  _pointer_type->output_instance(out, "addr", &parser);
  out << ", int caller_manages) {\n"
      << "  PyObject *" << classobj_func << "();\n"
      << "  PyObject *classobj = " << classobj_func << "();\n"
      << "  PyInstanceObject *instance = (PyInstanceObject *)PyInstance_New(classobj, (PyObject *)NULL, (PyObject *)NULL);\n"
      << "  if (instance != (PyInstanceObject *)NULL) {\n"
      << "    PyObject *thisptr = PyLong_FromVoidPtr((void*)addr);\n"
      << "    PyDict_SetItemString(instance->in_dict, \"this\", thisptr);\n"
      << "  }\n"
      << "  return (PyObject *)instance;\n"
      << "}\n\n";
}

////////////////////////////////////////////////////////////////////
//     Function: FunctionWriterPtrToPython::get_pointer_type
//       Access: Public
//  Description: Returns the type that represents a pointer to the
//               data type.
////////////////////////////////////////////////////////////////////
CPPType *FunctionWriterPtrToPython::
get_pointer_type() const {
  return _pointer_type;
}
