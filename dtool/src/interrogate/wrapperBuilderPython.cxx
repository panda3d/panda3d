// Filename: wrapperBuilderPython.cxx
// Created by:  drose (07Aug00)
//
////////////////////////////////////////////////////////////////////
//
// PANDA 3D SOFTWARE
// Copyright (c) 2001, Disney Enterprises, Inc.  All rights reserved
//
// All use of this software is subject to the terms of the Panda 3d
// Software license.  You should have received a copy of this license
// along with this source code; you will also find a current copy of
// the license at http://www.panda3d.org/license.txt .
//
// To contact the maintainers of this program write to
// panda3d@yahoogroups.com .
//
////////////////////////////////////////////////////////////////////

#include "wrapperBuilderPython.h"
#include "interrogate.h"
#include "parameterRemap.h"
#include "typeManager.h"

#include <interrogateDatabase.h>
#include <cppInstance.h>
#include <cppFunctionType.h>
#include <cppParameterList.h>
#include <cppConstType.h>
#include <cppReferenceType.h>
#include <cppPointerType.h>
#include <cppSimpleType.h>
#include <cppStructType.h>
#include <cppExpression.h>
#include <notify.h>

////////////////////////////////////////////////////////////////////
//     Function: WrapperBuilderPython::Constructor
//       Access: Public
//  Description:
////////////////////////////////////////////////////////////////////
WrapperBuilderPython::
WrapperBuilderPython() {
}

////////////////////////////////////////////////////////////////////
//     Function: WrapperBuilderPython::write_wrapper
//       Access: Public, Virtual
//  Description: Generates a wrapper function to the indicated output
//               stream.
////////////////////////////////////////////////////////////////////
void WrapperBuilderPython::
write_wrapper(ostream &out, const string &wrapper_name) const {
  out << "/*\n"
      << " * Python wrapper for\n"
      << " * " << _description << "\n"
      << " */\n";

  if (!output_function_names) {
    // If we're not saving the function names, don't export it from
    // the library.
    out << "static ";
  }

  out << "PyObject *\n"
      << wrapper_name << "(PyObject *, PyObject *args) {\n";

  write_spam_message(out);

  int pn;

  string format_specifiers;
  string parameter_list;
  vector_string pexprs;

  // Make one pass through the parameter list.  We will output a
  // one-line temporary variable definition for each parameter, while
  // simultaneously building the ParseTuple() function call and also
  // the parameter expression list for call_function().

  for (pn = 0; pn < (int)_parameters.size(); pn++) {
    out << "  ";
    CPPType *orig_type = _parameters[pn]._remap->get_orig_type();
    CPPType *type = _parameters[pn]._remap->get_new_type();

    // This is the string to convert our local variable to the
    // appropriate C++ type.  Normally this is just a cast.
    string pexpr_string =
      "(" + type->get_local_name(&parser) + ")" + get_parameter_name(pn);

    if (_parameters[pn]._remap->new_type_is_atomic_string()) {
      if (TypeManager::is_char_pointer(orig_type)) {
        out << "char *" << get_parameter_name(pn);
        format_specifiers += "s";
        parameter_list += ", &" + get_parameter_name(pn);

      } else {
        out << "char *" << get_parameter_name(pn)
            << "_str; int " << get_parameter_name(pn) << "_len";
        format_specifiers += "s#";
        parameter_list += ", &" + get_parameter_name(pn)
          + "_str, &" + get_parameter_name(pn) + "_len";
        pexpr_string = "basic_string<char>(" +
          get_parameter_name(pn) + "_str, " +
          get_parameter_name(pn) + "_len)";
      }

    } else if (TypeManager::is_bool(type)) {
      out << "PyObject *" << get_parameter_name(pn);
      format_specifiers += "O";
      parameter_list += ", &" + get_parameter_name(pn);
      pexpr_string = "(PyObject_IsTrue(" + get_parameter_name(pn) + ")!=0)";

    } else if (TypeManager::is_integer(type)) {
      out << "int " << get_parameter_name(pn);
      format_specifiers += "i";
      parameter_list += ", &" + get_parameter_name(pn);

    } else if (TypeManager::is_float(type)) {
      out << "double " << get_parameter_name(pn);
      format_specifiers += "d";
      parameter_list += ", &" + get_parameter_name(pn);

    } else if (TypeManager::is_char_pointer(type)) {
      out << "char *" << get_parameter_name(pn);
      format_specifiers += "s";
      parameter_list += ", &" + get_parameter_name(pn);

    } else if (TypeManager::is_pointer(type)) {
      out << "int " << get_parameter_name(pn);
      format_specifiers += "i";
      parameter_list += ", &" + get_parameter_name(pn);

    } else {
      // Ignore a parameter.
      out << "PyObject *" << get_parameter_name(pn);
      format_specifiers += "O";
      parameter_list += ", &" + get_parameter_name(pn);
    }

    out << ";\n";
    pexprs.push_back(pexpr_string);
  }

  out << "  if (PyArg_ParseTuple(args, \"" << format_specifiers
      << "\"" << parameter_list << ")) {\n";

  if (_return_type->new_type_is_atomic_string()) {
    // Treat strings as a special case.  We don't want to format the
    // return expression.
    string return_expr = call_function(out, 4, false, pexprs);

    CPPType *type = _return_type->get_orig_type();
    out << "    ";
    type->output_instance(out, "return_value", &parser);
    out << " = " << return_expr << ";\n";
    return_expr = manage_return_value(out, 4, "return_value");
    test_assert(out, 4);
    pack_return_value(out, return_expr);

  } else {
    string return_expr = call_function(out, 4, true, pexprs);
    if (return_expr.empty()) {
      test_assert(out, 4);
      out << "    return Py_BuildValue(\"\");\n";

    } else {
      CPPType *type = _return_type->get_temporary_type();
      out << "    ";
      type->output_instance(out, "return_value", &parser);
      out << " = " << return_expr << ";\n";

      return_expr = manage_return_value(out, 4, "return_value");
      test_assert(out, 4);
      pack_return_value(out, _return_type->temporary_to_return(return_expr));
    }
  }

  out << "  }\n";

  out << "  return (PyObject *)NULL;\n";
  out << "}\n\n";
}

////////////////////////////////////////////////////////////////////
//     Function: WrapperBuilderPython::get_wrapper_name
//       Access: Public, Virtual
//  Description: Returns the callable name for this wrapper function.
////////////////////////////////////////////////////////////////////
string WrapperBuilderPython::
get_wrapper_name(const string &library_hash_name) const {
  return "_inP" + library_hash_name + _hash;
}

////////////////////////////////////////////////////////////////////
//     Function: WrapperBuilderPython::supports_atomic_strings
//       Access: Public, Virtual
//  Description: Returns true if this kind of wrapper can support true
//               atomic string objects (and not have to fiddle with
//               char *).
////////////////////////////////////////////////////////////////////
bool WrapperBuilderPython::
supports_atomic_strings() const {
  return true;
}

////////////////////////////////////////////////////////////////////
//     Function: WrapperBuilderPython::get_calling_convention
//       Access: Public, Virtual
//  Description: Returns an indication of what kind of function we are
//               building.
////////////////////////////////////////////////////////////////////
WrapperBuilder::CallingConvention WrapperBuilderPython::
get_calling_convention() const {
  return CC_python;
}

////////////////////////////////////////////////////////////////////
//     Function: WrapperBuilderPython::test_assert
//       Access: Protected
//  Description: Outputs code to check to see if an assertion has
//               failed while the C++ code was executing, and report
//               this failure back to Python.
////////////////////////////////////////////////////////////////////
void WrapperBuilderPython::
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
    out << "#endif\n";
  }
}

////////////////////////////////////////////////////////////////////
//     Function: WrapperBuilderPython::pack_return_value
//       Access: Protected
//  Description: Outputs a command to pack the indicated expression,
//               of the return_type type, as a Python return value.
////////////////////////////////////////////////////////////////////
void WrapperBuilderPython::
pack_return_value(ostream &out, string return_expr) const {
  CPPType *orig_type = _return_type->get_orig_type();
  CPPType *type = _return_type->get_new_type();

  out << "    return Py_BuildValue(";

  if (_return_type->new_type_is_atomic_string()) {
    if (TypeManager::is_char_pointer(orig_type)) {
      out << "\"s\", " << return_expr;

    } else {
      out << "\"s#\", " << return_expr << ".data(), "
          << return_expr << ".length()";
    }

  } else if (TypeManager::is_integer(type)) {
    out << "\"i\", (int)(" << return_expr << ")";

  } else if (TypeManager::is_float(type)) {
    out << "\"d\", (double)(" << return_expr << ")";

  } else if (TypeManager::is_char_pointer(type)) {
    out << "\"s\", " << return_expr;

  } else if (TypeManager::is_pointer(type)) {
    out << "\"i\", (int)" << return_expr;

  } else {
    // Return None.
    out << "\"\"";
  }
  out << ");\n";
}
