// Filename: interfaceMakerPython.cxx
// Created by:  drose (21Sep01)
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

#include "interfaceMakerPython.h"
#include "interrogate.h"

////////////////////////////////////////////////////////////////////
//     Function: InterfaceMakerPython::Constructor
//       Access: Public
//  Description:
////////////////////////////////////////////////////////////////////
InterfaceMakerPython::
InterfaceMakerPython(InterrogateModuleDef *def) :
  InterfaceMaker(def)
{
}

////////////////////////////////////////////////////////////////////
//     Function: InterfaceMakerPython::write_includes
//       Access: Public, Virtual
//  Description: Generates the list of #include ... whatever that's
//               required by this particular interface to the
//               indicated output stream.
////////////////////////////////////////////////////////////////////
void InterfaceMakerPython::
write_includes(ostream &out) {
  InterfaceMaker::write_includes(out);
  out << "#undef _POSIX_C_SOURCE\n"
      << "#define PY_SSIZE_T_CLEAN 1\n\n"
      << "#if PYTHON_FRAMEWORK\n"
      << "  #include \"Python/Python.h\"\n"
      << "#else\n"
      << "  #include \"Python.h\"\n"
      << "#endif\n";
}

////////////////////////////////////////////////////////////////////
//     Function: InterfaceMakerPython::test_assert
//       Access: Protected
//  Description: Outputs code to check to see if an assertion has
//               failed while the C++ code was executing, and report
//               this failure back to Python.
////////////////////////////////////////////////////////////////////
void InterfaceMakerPython::
test_assert(ostream &out, int indent_level) const {
  if (watch_asserts) {
    out << "#ifndef NDEBUG\n";
    indent(out, indent_level)
      << "Notify *notify = Notify::ptr();\n";
    indent(out, indent_level)
      << "if (notify->has_assert_failed()) {\n";
    indent(out, indent_level + 2)
      << "PyErr_SetString(PyExc_AssertionError, notify->get_assert_error_message().c_str());\n";
    indent(out, indent_level + 2)
      << "notify->clear_assert_failed();\n";
    indent(out, indent_level + 2)
      << "return (PyObject *)NULL;\n";
    indent(out, indent_level)
      << "}\n";
    indent(out, indent_level)
      << "if (PyErr_Occurred()) {\n";
    indent(out, indent_level + 2)
      << "return (PyObject *)NULL;\n";
    indent(out, indent_level)
      << "}\n";
    out << "#endif\n";
  }
}
