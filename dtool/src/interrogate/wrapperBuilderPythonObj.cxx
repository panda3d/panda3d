// Filename: wrapperBuilderPythonObj.cxx
// Created by:  drose (11Sep01)
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

#include "wrapperBuilderPythonObj.h"
#include "interrogate.h"
#include "parameterRemap.h"
#include "typeManager.h"
#include "functionWriterPtrFromPython.h"
#include "functionWriterPtrToPython.h"
#include "functionWriters.h"

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
//     Function: WrapperBuilderPythonObj::Constructor
//       Access: Public
//  Description:
////////////////////////////////////////////////////////////////////
WrapperBuilderPythonObj::
WrapperBuilderPythonObj() {
}

////////////////////////////////////////////////////////////////////
//     Function: WrapperBuilderPythonObj::get_function_writers
//       Access: Public, Virtual
//  Description: Adds whatever list of FunctionWriters might be needed
//               for this particular WrapperBuilder.  These will be
//               generated to the output source file before
//               write_wrapper() is called.
////////////////////////////////////////////////////////////////////
void WrapperBuilderPythonObj::
get_function_writers(FunctionWriters &writers) {
  for (int def_index = 0; def_index < (int)_def.size(); ++def_index) {
    const FunctionDef *def = _def[def_index];

    if (def->_is_method) {
      FunctionWriterPtrFromPython *writer = 
        new FunctionWriterPtrFromPython(def->_struct_type);
      writers.add_writer(writer);
    }

    int pn;
    for (pn = 0; pn < (int)def->_parameters.size(); pn++) {
      CPPType *type = def->_parameters[pn]._remap->get_new_type();

      if (def->_parameters[pn]._remap->new_type_is_atomic_string()) {
      } else if (TypeManager::is_bool(type)) {
      } else if (TypeManager::is_integer(type)) {
      } else if (TypeManager::is_float(type)) {
      } else if (TypeManager::is_char_pointer(type)) {

      } else if (TypeManager::is_pointer(type)) {
        FunctionWriterPtrFromPython *writer = 
          new FunctionWriterPtrFromPython(type);
        writers.add_writer(writer);
      }
    }

    CPPType *type = def->_return_type->get_new_type();
    if (def->_return_type->new_type_is_atomic_string()) {
    } else if (TypeManager::is_integer(type)) {
    } else if (TypeManager::is_float(type)) {
    } else if (TypeManager::is_char_pointer(type)) {
      
    } else if (TypeManager::is_pointer(type)) {
      FunctionWriterPtrToPython *writer = 
        new FunctionWriterPtrToPython(type);
      writers.add_writer(writer);
    }
  }
}

////////////////////////////////////////////////////////////////////
//     Function: WrapperBuilderPythonObj::write_prototype
//       Access: Public, Virtual
//  Description: Generates the prototype for the wrapper function(s).
////////////////////////////////////////////////////////////////////
void WrapperBuilderPythonObj::
write_prototype(ostream &out, const string &wrapper_name) const {
  if (!output_function_names) {
    // If we're not saving the function names, don't export it from
    // the library.
    out << "static ";
  } else {
    out << "extern \"C\" ";
  }

  out << "PyObject *"
      << wrapper_name << "(PyObject *self, PyObject *args);\n";
}

////////////////////////////////////////////////////////////////////
//     Function: WrapperBuilderPythonObj::write_wrapper
//       Access: Public, Virtual
//  Description: Generates a wrapper function to the indicated output
//               stream.
////////////////////////////////////////////////////////////////////
void WrapperBuilderPythonObj::
write_wrapper(ostream &out, const string &wrapper_name) const {
  bool any_is_method = false;

  out << "/*\n"
      << " * Python object wrapper for\n";

  for (int def_index = 0; def_index < (int)_def.size(); ++def_index) {
    const FunctionDef *def = _def[def_index];
    out << " * " << def->_description << "\n";
    if (def->_is_method) {
      any_is_method = true;
    }
  }
  out << " */\n";

  if (!output_function_names) {
    // If we're not saving the function names, don't export it from
    // the library.
    out << "static ";
  }

  if (any_is_method) {
    out << "PyObject *\n"
        << wrapper_name << "(PyObject *self, PyObject *args) {\n";
  } else {
    out << "PyObject *\n"
        << wrapper_name << "(PyObject *, PyObject *args) {\n";
  }
  
  write_spam_message(0, out);
  string expected_params = "Arguments must match one of:";

  for (int def_index = 0; def_index < (int)_def.size(); ++def_index) {
    const FunctionDef *def = _def[def_index];
    out << "  {\n"
        << "    /* " << def->_description << " */\n"
        << "\n";

    string thisptr;
    if (def->_is_method) {
      // Declare a "thisptr" variable.
      thisptr = "thisptr";
      FunctionWriterPtrFromPython *writer = 
        new FunctionWriterPtrFromPython(def->_struct_type);
      out << "    ";
      writer->get_pointer_type()->output_instance(out, thisptr, &parser);
      out << ";\n"
          << "    if (!" << writer->get_name() << "(self, &thisptr)) {\n"
          << "      return (PyObject *)NULL;\n"
          << "    }\n";
      delete writer;
    }

    string format_specifiers;
    string parameter_list;
    vector_string pexprs;

    // Make one pass through the parameter list.  We will output a
    // one-line temporary variable definition for each parameter, while
    // simultaneously building the ParseTuple() function call and also
    // the parameter expression list for call_function().

    expected_params += "\\n  ";
    expected_params += _def[0]->_function->get_simple_name();
    expected_params += "(";
    
    int pn;
    for (pn = 0; pn < (int)def->_parameters.size(); pn++) {
      if (pn != 0) {
        expected_params += ", ";
      }

      out << "    ";
      CPPType *orig_type = def->_parameters[pn]._remap->get_orig_type();
      CPPType *type = def->_parameters[pn]._remap->get_new_type();

      // This is the string to convert our local variable to the
      // appropriate C++ type.  Normally this is just a cast.
      string pexpr_string =
        "(" + type->get_local_name(&parser) + ")" + get_parameter_name(pn);

      if (def->_parameters[pn]._remap->new_type_is_atomic_string()) {
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
        expected_params += "string";

      } else if (TypeManager::is_bool(type)) {
        out << "PyObject *" << get_parameter_name(pn);
        format_specifiers += "O";
        parameter_list += ", &" + get_parameter_name(pn);
        pexpr_string = "(PyObject_IsTrue(" + get_parameter_name(pn) + ")!=0)";
        expected_params += "bool";

      } else if (TypeManager::is_integer(type)) {
        out << "int " << get_parameter_name(pn);
        format_specifiers += "i";
        parameter_list += ", &" + get_parameter_name(pn);
        expected_params += "integer";

      } else if (TypeManager::is_float(type)) {
        out << "double " << get_parameter_name(pn);
        format_specifiers += "d";
        parameter_list += ", &" + get_parameter_name(pn);
        expected_params += "float";

      } else if (TypeManager::is_char_pointer(type)) {
        out << "char *" << get_parameter_name(pn);
        format_specifiers += "s";
        parameter_list += ", &" + get_parameter_name(pn);
        expected_params += "string";

      } else if (TypeManager::is_pointer(type)) {
        out << "int " << get_parameter_name(pn);
        format_specifiers += "i";
        parameter_list += ", &" + get_parameter_name(pn);
        expected_params += "pointer";

      } else {
        // Ignore a parameter.
        out << "PyObject *" << get_parameter_name(pn);
        format_specifiers += "O";
        parameter_list += ", &" + get_parameter_name(pn);
        expected_params += "ignored";
      }

      if (def->_parameters[pn]._has_name) {
        expected_params += " " + def->_parameters[pn]._name;
      }

      out << ";\n";
      pexprs.push_back(pexpr_string);
    }
    expected_params += ")";

    out << "    if (PyArg_ParseTuple(args, \"" << format_specifiers
        << "\"" << parameter_list << ")) {\n";

    if (track_interpreter) {
      out << "      in_interpreter = 0;\n";
    }

    if (def->_return_type->new_type_is_atomic_string()) {
      // Treat strings as a special case.  We don't want to format the
      // return expression.
      string return_expr = call_function(def_index, 
                                         out, 6, false, thisptr, pexprs);

      CPPType *type = def->_return_type->get_orig_type();
      out << "      ";
      type->output_instance(out, "return_value", &parser);
      out << " = " << return_expr << ";\n";

      if (track_interpreter) {
        out << "      in_interpreter = 1;\n";
      }

      return_expr = manage_return_value(def_index, out, 6, "return_value");
      test_assert(out, 6);
      pack_return_value(def_index, out, return_expr);

    } else {
      string return_expr = call_function(def_index,
                                         out, 6, true, thisptr, pexprs);
      if (return_expr.empty()) {
        if (track_interpreter) {
          out << "      in_interpreter = 1;\n";
        }
        test_assert(out, 6);
        out << "      return Py_BuildValue(\"\");\n";

      } else {
        CPPType *type = def->_return_type->get_temporary_type();
        out << "      ";
        type->output_instance(out, "return_value", &parser);
        out << " = " << return_expr << ";\n";
        if (track_interpreter) {
          out << "      in_interpreter = 1;\n";
        }

        return_expr = manage_return_value(def_index, out, 6, "return_value");
        test_assert(out, 6);
        pack_return_value(def_index, out, def->_return_type->temporary_to_return(return_expr));
      }
    }

    out << "    }\n"
        << "    PyErr_Clear();\n"  // Clear the error generated by ParseTuple()
        << "  }\n";
  }

  // Invalid parameters.  Generate an error exception.  (We don't rely
  // on the error already generated by ParseTuple(), because it only
  // reports the error for one flavor of the function, whereas we
  // might accept multiple flavors for the different overloaded
  // C++ function signatures.

  out << "  PyErr_SetString(PyExc_TypeError, \"" << expected_params << "\");\n"
      << "  return (PyObject *)NULL;\n";
  out << "}\n\n";
}

////////////////////////////////////////////////////////////////////
//     Function: WrapperBuilderPythonObj::get_wrapper_name
//       Access: Public, Virtual
//  Description: Returns the callable name for this wrapper function.
////////////////////////////////////////////////////////////////////
string WrapperBuilderPythonObj::
get_wrapper_name(const string &library_hash_name) const {
  return "_inM" + library_hash_name + _hash;
}

////////////////////////////////////////////////////////////////////
//     Function: WrapperBuilderPythonObj::supports_atomic_strings
//       Access: Public, Virtual
//  Description: Returns true if this kind of wrapper can support true
//               atomic string objects (and not have to fiddle with
//               char *).
////////////////////////////////////////////////////////////////////
bool WrapperBuilderPythonObj::
supports_atomic_strings() const {
  return true;
}

////////////////////////////////////////////////////////////////////
//     Function: WrapperBuilderPythonObj::synthesize_this_parameter
//       Access: Public, Virtual
//  Description: Returns true if this particular wrapper type requires
//               an explicit "this" parameter to be added to the
//               function parameter list when appropriate, or false if
//               the "this" pointer will come through a different
//               channel.
////////////////////////////////////////////////////////////////////
bool WrapperBuilderPythonObj::
synthesize_this_parameter() const {
  return false;
}

////////////////////////////////////////////////////////////////////
//     Function: WrapperBuilderPythonObj::get_calling_convention
//       Access: Public, Virtual
//  Description: Returns an indication of what kind of function we are
//               building.
////////////////////////////////////////////////////////////////////
WrapperBuilder::CallingConvention WrapperBuilderPythonObj::
get_calling_convention() const {
  return CC_python_obj;
}

////////////////////////////////////////////////////////////////////
//     Function: WrapperBuilderPythonObj::test_assert
//       Access: Protected
//  Description: Outputs code to check to see if an assertion has
//               failed while the C++ code was executing, and report
//               this failure back to Python.
////////////////////////////////////////////////////////////////////
void WrapperBuilderPythonObj::
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
//     Function: WrapperBuilderPythonObj::pack_return_value
//       Access: Protected
//  Description: Outputs a command to pack the indicated expression,
//               of the return_type type, as a Python return value.
////////////////////////////////////////////////////////////////////
void WrapperBuilderPythonObj::
pack_return_value(int def_index, ostream &out, string return_expr) const {
  assert(def_index >= 0 && def_index < (int)_def.size());
  const FunctionDef *def = _def[def_index];

  CPPType *orig_type = def->_return_type->get_orig_type();
  CPPType *type = def->_return_type->get_new_type();

  if (def->_return_type->new_type_is_atomic_string()) {
    if (TypeManager::is_char_pointer(orig_type)) {
      out << "      return PyString_FromString(" << return_expr << ");\n";

    } else {
      out << "      return PyString_FromStringAndSize("
          << return_expr << ".data(), " << return_expr << ".length());\n";
    }

  } else if (TypeManager::is_integer(type)) {
    out << "      return PyInt_FromLong(" 
        << return_expr << ");\n";

  } else if (TypeManager::is_float(type)) {
    out << "      return PyFloat_FromDouble("
        << return_expr << ");\n";

  } else if (TypeManager::is_char_pointer(type)) {
    out << "      return PyString_FromString(" << return_expr << ");\n";

  } else if (TypeManager::is_pointer(type)) {
    bool caller_manages = def->_return_value_needs_management;
    FunctionWriterPtrToPython *writer = 
      new FunctionWriterPtrToPython(type);
    out << "      return " << writer->get_name() << "(" << return_expr
        << ", " << caller_manages << ");\n";

  } else {
    // Return None.
    out << "      return Py_BuildValue(\"\");\n";
  }
}
