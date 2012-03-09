// Filename: functionWriterPtrFromPython.cxx
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

#include "functionWriterPtrFromPython.h"
#include "typeManager.h"
#include "interrogateBuilder.h"
#include "interrogate.h"

#include "cppPointerType.h"

////////////////////////////////////////////////////////////////////
//     Function: FunctionWriterPtrFromPython::Constructor
//       Access: Public
//  Description:
////////////////////////////////////////////////////////////////////
FunctionWriterPtrFromPython::
FunctionWriterPtrFromPython(CPPType *type) {
  _type = TypeManager::unwrap_const(TypeManager::unwrap_pointer(type));
  _name = 
    "from_python_" + 
    InterrogateBuilder::clean_identifier(_type->get_local_name(&parser));

  _pointer_type = new CPPPointerType(_type);
}

////////////////////////////////////////////////////////////////////
//     Function: FunctionWriterPtrFromPython::Destructor
//       Access: Public
//  Description:
////////////////////////////////////////////////////////////////////
FunctionWriterPtrFromPython::
~FunctionWriterPtrFromPython() {
  delete _pointer_type;
}

////////////////////////////////////////////////////////////////////
//     Function: FunctionWriterPtrFromPython::write_prototype
//       Access: Public, Virtual
//  Description: Outputs the prototype for the function.
////////////////////////////////////////////////////////////////////
void FunctionWriterPtrFromPython::
write_prototype(ostream &out) {
  CPPType *ppointer = new CPPPointerType(_pointer_type);

  out << "static int " << _name << "(PyObject *obj, ";
  ppointer->output_instance(out, "addr", &parser);
  out << ");\n";
  
  delete ppointer;
}

////////////////////////////////////////////////////////////////////
//     Function: FunctionWriterPtrFromPython::write_code
//       Access: Public, Virtual
//  Description: Outputs the code for the function.
////////////////////////////////////////////////////////////////////
void FunctionWriterPtrFromPython::
write_code(ostream &out) {
  CPPType *ppointer = new CPPPointerType(_pointer_type);

  out << "static int\n"
      << _name << "(PyObject *obj, ";
  ppointer->output_instance(out, "addr", &parser);
  out << ") {\n"
      << "  if (obj != (PyObject *)NULL && PyInstance_Check(obj)) {\n"
    //      << "    PyClassObject *in_class = ((PyInstanceObject *)obj)->in_class;\n"
      << "    PyObject *in_dict = ((PyInstanceObject *)obj)->in_dict;\n"
      << "    if (in_dict != (PyObject *)NULL && PyDict_Check(in_dict)) {\n"
      << "      PyObject *thisobj = PyDict_GetItemString(in_dict, \"this\");\n"
      << "      if (thisobj != (PyObject *)NULL && PyLong_Check(thisobj)) {\n"
      << "        (*addr) = ("
      << _pointer_type->get_local_name(&parser) << ")PyLong_AsVoidPtr(thisobj);\n"
      << "        return 1;\n"
      << "      }\n"
      << "    }\n"
      << "  }\n"
      << "  return 0;\n"
      << "}\n\n";
  
  delete ppointer;
}

////////////////////////////////////////////////////////////////////
//     Function: FunctionWriterPtrFromPython::get_type
//       Access: Public
//  Description: Returns the type that represents the actual data type.
////////////////////////////////////////////////////////////////////
CPPType *FunctionWriterPtrFromPython::
get_type() const {
  return _type;
}

////////////////////////////////////////////////////////////////////
//     Function: FunctionWriterPtrFromPython::get_pointer_type
//       Access: Public
//  Description: Returns the type that represents a pointer to the
//               data type.
////////////////////////////////////////////////////////////////////
CPPType *FunctionWriterPtrFromPython::
get_pointer_type() const {
  return _pointer_type;
}
