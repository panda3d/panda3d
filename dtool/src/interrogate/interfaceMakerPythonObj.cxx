/**
 * PANDA 3D SOFTWARE
 * Copyright (c) Carnegie Mellon University.  All rights reserved.
 *
 * All use of this software is subject to the terms of the revised BSD
 * license.  You should have received a copy of this license along
 * with this source code in a file named "LICENSE."
 *
 * @file interfaceMakerPythonObj.cxx
 * @author drose
 * @date 2001-09-19
 */

#include "interfaceMakerPythonObj.h"
#include "interrogateBuilder.h"
#include "interrogate.h"
#include "functionRemap.h"
#include "parameterRemapUnchanged.h"
#include "typeManager.h"
#include "functionWriterPtrFromPython.h"
#include "functionWriterPtrToPython.h"

#include "interrogateDatabase.h"
#include "interrogateType.h"
#include "interrogateFunction.h"
#include "cppFunctionType.h"

using std::ostream;
using std::string;

/**
 *
 */
InterfaceMakerPythonObj::
InterfaceMakerPythonObj(InterrogateModuleDef *def) :
  InterfaceMakerPython(def)
{
}

/**
 *
 */
InterfaceMakerPythonObj::
~InterfaceMakerPythonObj() {
}

/**
 * Generates the list of function prototypes corresponding to the functions
 * that will be output in write_functions().
 */
void InterfaceMakerPythonObj::
write_prototypes(ostream &out, ostream *out_h) {
  FunctionsByIndex::iterator fi;
  for (fi = _functions.begin(); fi != _functions.end(); ++fi) {
    Function *func = (*fi).second;
    write_prototype_for(out, func);
  }

  out << "\n";
  InterfaceMakerPython::write_prototypes(out,out_h);
}

/**
 * Generates the list of functions that are appropriate for this interface.
 * This function is called *before* write_prototypes(), above.
 */
void InterfaceMakerPythonObj::
write_functions(ostream &out) {
  FunctionsByIndex::iterator fi;
  for (fi = _functions.begin(); fi != _functions.end(); ++fi) {
    Function *func = (*fi).second;
    write_function_for(out, func);
  }

  InterfaceMakerPython::write_functions(out);

  // Finally, generate all the make-class-wrapper functions.
  Objects::iterator oi;
  for (oi = _objects.begin(); oi != _objects.end(); ++oi) {
    Object *object = (*oi).second;

    write_class_wrapper(out, object);
  }
}

/**
 * Generates whatever additional code is required to support a module file.
 */
void InterfaceMakerPythonObj::
write_module(ostream &out,ostream *out_h, InterrogateModuleDef *def) {
  InterfaceMakerPython::write_module(out,out_h, def);

  out << "static PyMethodDef python_obj_funcs[] = {\n";

  Objects::iterator oi;
  for (oi = _objects.begin(); oi != _objects.end(); ++oi) {
    Object *object = (*oi).second;

    Functions::iterator fi;
    for (fi = object->_constructors.begin();
         fi != object->_constructors.end();
         ++fi) {
      Function *func = (*fi);
      out << "  { \"" << func->_ifunc.get_name() << "\", &" << func->_name
          << ", METH_VARARGS, nullptr },\n";
    }
  }
  out << "  { nullptr, nullptr, 0, nullptr }\n"
      << "};\n\n"

      << "#if PY_MAJOR_VERSION >= 3\n"
      << "static struct PyModuleDef python_obj_module = {\n"
      << "  PyModuleDef_HEAD_INIT,\n"
      << "  \"" << def->library_name << "\",\n"
      << "  nullptr,\n"
      << "  -1,\n"
      << "  python_obj_funcs,\n"
      << "  nullptr, nullptr, nullptr, nullptr\n"
      << "};\n\n"

      << "#define INIT_FUNC PyObject *PyInit_" << def->library_name << "\n"
      << "#else\n"
      << "#define INIT_FUNC void init" << def->library_name << "\n"
      << "#endif\n\n"

      << "#ifdef _WIN32\n"
      << "extern \"C\" __declspec(dllexport) INIT_FUNC();\n"
      << "#elif __GNUC__ >= 4\n"
      << "extern \"C\" __attribute__((visibility(\"default\"))) INIT_FUNC();\n"
      << "#else\n"
      << "extern \"C\" INIT_FUNC();\n"
      << "#endif\n\n"

      << "INIT_FUNC() {\n"
      << "#if PY_MAJOR_VERSION >= 3\n"
      << "  return PyModule_Create(&python_obj_module);\n"
      << "#else\n"
      << "  Py_InitModule(\"" << def->library_name << "\", python_obj_funcs);\n"
      << "#endif\n"
      << "}\n\n";
}

/**
 * This method should be overridden and redefined to return true for
 * interfaces that require the implicit "this" parameter, if present, to be
 * passed as the first parameter to any wrapper functions.
 */
bool InterfaceMakerPythonObj::
synthesize_this_parameter() {
  return true;
}

/**
 * Returns the name of the InterfaceMaker function generated to define the
 * Python class for the indicated struct type.
 */
string InterfaceMakerPythonObj::
get_builder_name(CPPType *struct_type) {
  return "get_python_class_" +
    InterrogateBuilder::clean_identifier(struct_type->get_local_name(&parser));
}

/**
 * Returns the prefix string used to generate wrapper function names.
 */
string InterfaceMakerPythonObj::
get_wrapper_prefix() {
  return "wpo_";
}

/**
 * Writes a function that will define the Python class.
 */
void InterfaceMakerPythonObj::
write_class_wrapper(ostream &out, InterfaceMaker::Object *object) {
  CPPType *struct_type = object->_itype._cpptype;
  if (struct_type == nullptr) {
    return;
  }

  string name = get_builder_name(struct_type);
  string python_name =
    InterrogateBuilder::clean_identifier(struct_type->get_simple_name());

  out << "/*\n"
      << " * Generate unique Python class for "
      << struct_type->get_local_name(&parser) << "\n"
      << " */\n"
      << "PyObject *\n"
      << name << "() {\n"
      << "  static PyObject *wrapper = nullptr;\n"
      << "  static PyMethodDef methods[] = {\n";

  int methods_size = 0;
  int class_methods_size = 0;

  Functions::iterator fi;
  for (fi = object->_methods.begin(); fi != object->_methods.end(); ++fi) {
    Function *func = (*fi);
    if (func->_has_this) {
      out << "  { \"" << func->_ifunc.get_name() << "\", &" << func->_name
          << ", METH_VARARGS },\n";
      methods_size++;
    }
  }

  out << "  };\n"
      << "  static const int methods_size = " << methods_size << ";\n\n"
      << "  static PyMethodDef class_methods[] = {\n";

  for (fi = object->_methods.begin(); fi != object->_methods.end(); ++fi) {
    Function *func = (*fi);
    if (!func->_has_this) {
      out << "  { \"" << func->_ifunc.get_name() << "\", &" << func->_name
          << ", METH_VARARGS },\n";
      class_methods_size++;
    }
  }

  out << "  };\n"
      << "  static const int class_methods_size = " << class_methods_size << ";\n\n"
      << "  if (wrapper == nullptr) {\n"
      << "    int i;\n"
      << "    PyObject *bases = PyTuple_New(0);\n"
      << "    PyObject *dict = PyDict_New();\n"
      << "#if PY_MAJOR_VERSION >= 3\n"
      << "    PyObject *name = PyUnicode_FromString(\"" << python_name << "\");\n"
      << "#else\n"
      << "    PyObject *name = PyString_FromString(\"" << python_name << "\");\n"
      << "#endif\n"
      << "    wrapper = PyClass_New(bases, dict, name);\n"
      << "    for (i = 0; i < methods_size; ++i) {\n"
      << "      PyObject *function, *method;\n"
      << "      function = PyCFunction_New(&methods[i], nullptr);\n"
      << "      method = PyMethod_New(function, nullptr, wrapper);\n"
      << "      PyDict_SetItemString(dict, methods[i].ml_name, method);\n"
      << "    }\n"
      << "    for (i = 0; i < class_methods_size; ++i) {\n"
      << "      PyObject *function;\n"
      << "      function = PyCFunction_New(&class_methods[i], nullptr);\n"
      << "      PyDict_SetItemString(dict, class_methods[i].ml_name, function);\n"
      << "    }\n"
      << "  }\n"
      << "  return wrapper;\n"
      << "}\n\n";
}

/**
 * Writes the prototype for the indicated function.
 */
void InterfaceMakerPythonObj::
write_prototype_for(ostream &out, InterfaceMaker::Function *func) {
  out << "static PyObject *"
      << func->_name << "(PyObject *self, PyObject *args);\n";
}

/**
 * Writes the definition for a function that will call the indicated C++
 * function or method.
 */
void InterfaceMakerPythonObj::
write_function_for(ostream &out, InterfaceMaker::Function *func) {
  Function::Remaps::const_iterator ri;
  out << "/*\n"
      << " * Python object wrapper for\n";
  for (ri = func->_remaps.begin(); ri != func->_remaps.end(); ++ri) {
    FunctionRemap *remap = (*ri);
    out << " * ";
    remap->write_orig_prototype(out, 0);
    out << "\n";
  }
  out << " */\n";

  out << "static PyObject *"
      << func->_name << "(PyObject *, PyObject *args) {\n";

  // Now write out each instance of the overloaded function.
  string expected_params = "Arguments must match one of:";

  for (ri = func->_remaps.begin(); ri != func->_remaps.end(); ++ri) {
    FunctionRemap *remap = (*ri);
    expected_params += "\\n  ";
    write_function_instance(out, 2, func, remap, expected_params);
  }

  // If we get here in the generated code, none of the parameters were valid.
  // Generate an error exception.  (We don't rely on the error already
  // generated by ParseTuple(), because it only reports the error for one
  // flavor of the function, whereas we might accept multiple flavors for the
  // different overloaded C++ function signatures.

  out << "  PyErr_SetString(PyExc_TypeError, \"" << expected_params << "\");\n"
      << "  return nullptr;\n";

  out << "}\n\n";
}

/**
 * Writes out the part of a function that handles a single instance of an
 * overloaded function.
 */
void InterfaceMakerPythonObj::
write_function_instance(ostream &out, int indent_level,
                        InterfaceMaker::Function *func,
                        FunctionRemap *remap, string &expected_params) {
  indent(out, indent_level) << "{\n";
  indent(out, indent_level + 2) << "/* ";
  remap->write_orig_prototype(out, 0);
  out << " */\n\n";

  string format_specifiers;
  string parameter_list;
  vector_string pexprs;
  string extra_convert;
  string extra_param_check;
  string extra_cleanup;

  // Make one pass through the parameter list.  We will output a one-line
  // temporary variable definition for each parameter, while simultaneously
  // building the ParseTuple() function call and also the parameter expression
  // list for call_function().

  expected_params += remap->_cppfunc->get_simple_name();
  expected_params += "(";

  int pn;
  for (pn = 0; pn < (int)remap->_parameters.size(); pn++) {
    if (pn != 0) {
      expected_params += ", ";
    }

    indent(out, indent_level + 2);
    CPPType *orig_type = remap->_parameters[pn]._remap->get_orig_type();
    CPPType *type = remap->_parameters[pn]._remap->get_new_type();
    string param_name = remap->get_parameter_name(pn);

    // This is the string to convert our local variable to the appropriate C++
    // type.  Normally this is just a cast.
    string pexpr_string =
      "(" + type->get_local_name(&parser) + ")" + param_name;

    if (remap->_parameters[pn]._remap->new_type_is_atomic_string()) {
      if (TypeManager::is_char_pointer(orig_type)) {
        out << "char *" << param_name;
        format_specifiers += "s";
        parameter_list += ", &" + param_name;

      } else {
        out << "char *" << param_name
            << "_str; Py_ssize_t " << param_name << "_len";
        format_specifiers += "s#";
        parameter_list += ", &" + param_name
          + "_str, &" + param_name + "_len";
        pexpr_string = "basic_string<char>(" +
          param_name + "_str, " +
          param_name + "_len)";
      }
      expected_params += "string";

    } else if (TypeManager::is_bool(type)) {
      out << "PyObject *" << param_name;
      format_specifiers += "O";
      parameter_list += ", &" + param_name;
      pexpr_string = "(PyObject_IsTrue(" + param_name + ")!=0)";
      expected_params += "bool";

    } else if (TypeManager::is_unsigned_longlong(type)) {
      out << "PyObject *" << param_name;
      format_specifiers += "O";
      parameter_list += ", &" + param_name;
      extra_convert += " PyObject *" + param_name + "_long = PyNumber_Long(" + param_name + ");";
      extra_param_check += "|| (" + param_name + "_long == nullptr)";
      pexpr_string = "PyLong_AsUnsignedLongLong(" + param_name + "_long)";
      extra_cleanup += " Py_XDECREF(" + param_name + "_long);";
      expected_params += "long";

    } else if (TypeManager::is_longlong(type)) {
      out << "PyObject *" << param_name;
      format_specifiers += "O";
      parameter_list += ", &" + param_name;
      extra_convert += " PyObject *" + param_name + "_long = PyNumber_Long(" + param_name + ");";
      extra_param_check += "|| (" + param_name + "_long == nullptr)";
      pexpr_string = "PyLong_AsLongLong(" + param_name + "_long)";
      extra_cleanup += " Py_XDECREF(" + param_name + "_long);";
      expected_params += "long";

    } else if (TypeManager::is_unsigned_integer(type)) {
      out << "PyObject *" << param_name;
      format_specifiers += "O";
      parameter_list += ", &" + param_name;
      extra_convert += " PyObject *" + param_name + "_uint = PyNumber_Long(" + param_name + ");";
      extra_param_check += "|| (" + param_name + "_uint == nullptr)";
      pexpr_string = "(unsigned int)PyLong_AsUnsignedLong(" + param_name + "_uint)";
      extra_cleanup += " Py_XDECREF(" + param_name + "_uint);";
      expected_params += "unsigned int";

    } else if (TypeManager::is_integer(type)) {
      out << "int " << param_name;
      format_specifiers += "i";
      parameter_list += ", &" + param_name;
      expected_params += "integer";

    } else if (TypeManager::is_float(type)) {
      out << "double " << param_name;
      format_specifiers += "d";
      parameter_list += ", &" + param_name;
      expected_params += "float";

    } else if (TypeManager::is_char_pointer(type)) {
      out << "char *" << param_name;
      format_specifiers += "s";
      parameter_list += ", &" + param_name;
      expected_params += "string";

    } else if (TypeManager::is_pointer(type)) {
      FunctionWriterPtrFromPython *writer = get_ptr_from_python(type);
      writer->get_pointer_type()->output_instance(out, param_name, &parser);
      format_specifiers += "O&";
      parameter_list += ", &" + writer->get_name() + ", &" + param_name;
      expected_params += writer->get_type()->get_preferred_name();

    } else {
      // Ignore a parameter.
      out << "PyObject *" << param_name;
      format_specifiers += "O";
      parameter_list += ", &" + param_name;
      expected_params += "any";
    }

    if (remap->_parameters[pn]._has_name) {
      expected_params += " " + remap->_parameters[pn]._name;
    }

    out << ";\n";
    pexprs.push_back(pexpr_string);
  }
  expected_params += ")";

  indent(out, indent_level + 2)
    << "if (PyArg_ParseTuple(args, \"" << format_specifiers
    << "\"" << parameter_list << ")) {\n";

  if (!extra_convert.empty()) {
    indent(out, indent_level + 3)
      << extra_convert << "\n";
  }

  if (!extra_param_check.empty()) {
    indent(out, indent_level + 4)
      << "if (" << extra_param_check.substr(3) << ") {\n";
    if (!extra_cleanup.empty()) {
      indent(out, indent_level + 5)
        << extra_cleanup << "\n";
    }
    indent(out, indent_level + 6)
      << "PyErr_SetString(PyExc_TypeError, \"Invalid parameters.\");\n";
    indent(out, indent_level + 6)
      << "return nullptr;\n";
    indent(out, indent_level + 4)
      << "}\n";
  }

  if (track_interpreter) {
    indent(out, indent_level + 4)
      << "in_interpreter = 0;\n";
  }

  if (!remap->_void_return &&
      remap->_return_type->new_type_is_atomic_string()) {
    // Treat strings as a special case.  We don't want to format the return
    // expression.
    string return_expr =
      remap->call_function(out, indent_level + 4, false, "param0", pexprs);

    CPPType *type = remap->_return_type->get_orig_type();
    indent(out, indent_level + 4);
    type->output_instance(out, "return_value", &parser);
    out << " = " << return_expr << ";\n";

    if (track_interpreter) {
      indent(out, indent_level + 4)
        << "in_interpreter = 1;\n";
    }
    if (!extra_cleanup.empty()) {
      indent(out, indent_level + 3)
        << extra_cleanup << "\n";
    }

    return_expr = manage_return_value(out, indent_level + 4, remap, "return_value");
    test_assert(out, indent_level + 4);
    pack_return_value(out, indent_level + 4, remap, return_expr);

  } else {
    string return_expr =
      remap->call_function(out, indent_level + 4, true, "param0", pexprs);
    if (return_expr.empty()) {
      if (track_interpreter) {
        indent(out, indent_level + 4)
          << "in_interpreter = 1;\n";
      }
      if (!extra_cleanup.empty()) {
        indent(out, indent_level + 3)
          << extra_cleanup << "\n";
      }
      test_assert(out, indent_level + 4);
      indent(out, indent_level + 4)
        << "return Py_BuildValue(\"\");\n";

    } else {
      CPPType *type = remap->_return_type->get_temporary_type();
      indent(out, indent_level + 4);
      type->output_instance(out, "return_value", &parser);
      out << " = " << return_expr << ";\n";
      if (track_interpreter) {
        indent(out, indent_level + 4)
          << "in_interpreter = 1;\n";
      }
      if (!extra_cleanup.empty()) {
        indent(out, indent_level + 3)
          << extra_cleanup << "\n";
      }

      return_expr = manage_return_value(out, indent_level + 4, remap, "return_value");
      test_assert(out, indent_level + 4);
      pack_return_value(out, indent_level + 4, remap, remap->_return_type->temporary_to_return(return_expr));
    }
  }

  indent(out, indent_level + 2) << "}\n";
  indent(out, indent_level + 2)
    << "PyErr_Clear();\n";  // Clear the error generated by ParseTuple()
  indent(out, indent_level)
    << "}\n";
}

/**
 * Outputs a command to pack the indicated expression, of the return_type
 * type, as a Python return value.
 */
void InterfaceMakerPythonObj::
pack_return_value(ostream &out, int indent_level,
                  FunctionRemap *remap, string return_expr) {
  CPPType *orig_type = remap->_return_type->get_orig_type();
  CPPType *type = remap->_return_type->get_new_type();

  if (remap->_return_type->new_type_is_atomic_string()) {
    if (TypeManager::is_char_pointer(orig_type)) {
      out << "#if PY_MAJOR_VERSION >= 3\n";
      indent(out, indent_level)
        << "return PyUnicode_FromString(" << return_expr << ");\n";
      out << "#else\n";
      indent(out, indent_level)
        << "return PyString_FromString(" << return_expr << ");\n";
      out << "#endif\n";

    } else {
      out << "#if PY_MAJOR_VERSION >= 3\n";
      indent(out, indent_level)
        << "return PyUnicode_FromStringAndSize("
        << return_expr << ".data(), (Py_ssize_t)" << return_expr << ".length());\n";
      out << "#else\n";
      indent(out, indent_level)
        << "return PyString_FromStringAndSize("
        << return_expr << ".data(), (Py_ssize_t)" << return_expr << ".length());\n";
      out << "#endif\n";
    }

  } else if (TypeManager::is_bool(type)) {
    indent(out, indent_level)
      << "return PyBool_FromLong(" << return_expr << ");\n";

  } else if (TypeManager::is_unsigned_longlong(type)) {
    indent(out, indent_level)
      << "return PyLong_FromUnsignedLongLong(" << return_expr << ");\n";

  } else if (TypeManager::is_longlong(type)) {
    indent(out, indent_level)
      << "return PyLong_FromLongLong(" << return_expr << ");\n";

  } else if (TypeManager::is_unsigned_integer(type)) {
    indent(out, indent_level)
      << "return PyLong_FromUnsignedLong(" << return_expr << ");\n";

  } else if (TypeManager::is_integer(type)) {
    out << "#if PY_MAJOR_VERSION >= 3\n";
    indent(out, indent_level)
      << "return PyLong_FromLong(" << return_expr << ");\n";
    out << "#else\n";
    indent(out, indent_level)
      << "return PyInt_FromLong(" << return_expr << ");\n";
    out << "#endif\n";

  } else if (TypeManager::is_float(type)) {
    indent(out, indent_level)
      << "return PyFloat_FromDouble(" << return_expr << ");\n";

  } else if (TypeManager::is_char_pointer(type)) {
    out << "#if PY_MAJOR_VERSION >= 3\n";
    indent(out, indent_level)
      << "return PyUnicode_FromString(" << return_expr << ");\n";
    out << "#else\n";
    indent(out, indent_level)
      << "return PyString_FromString(" << return_expr << ");\n";
    out << "#endif\n";

  } else if (TypeManager::is_pointer(type)) {
    bool caller_manages = remap->_return_value_needs_management;

    FunctionWriterPtrToPython *writer = get_ptr_to_python(type);
    indent(out, indent_level)
      << "return " << writer->get_name() << "(("
      << writer->get_pointer_type()->get_local_name(&parser) << ")"
      << return_expr << ", " << caller_manages << ");\n";

  } else {
    // Return None.
    indent(out, indent_level)
      << "return Py_BuildValue(\"\");\n";
  }
}

/**
 * Returns a FunctionWriter pointer suitable for converting from a Python
 * wrapper of the indicated type to the corresponding C++ pointer.
 */
FunctionWriterPtrFromPython *InterfaceMakerPythonObj::
get_ptr_from_python(CPPType *type) {
  PtrConverter::iterator ci;
  ci = _from_python.find(type);
  if (ci != _from_python.end()) {
    // We've previously used this type.
    return (FunctionWriterPtrFromPython *)(*ci).second;
  }

  FunctionWriter *writer =
    _function_writers.add_writer(new FunctionWriterPtrFromPython(type));
  _from_python.insert(PtrConverter::value_type(type, writer));
  return (FunctionWriterPtrFromPython *)writer;
}

/**
 * Returns a FunctionWriter pointer suitable for converting from a C++ pointer
 * of the indicated type to the corresponding Python wrapper.
 */
FunctionWriterPtrToPython *InterfaceMakerPythonObj::
get_ptr_to_python(CPPType *type) {
  PtrConverter::iterator ci;
  ci = _to_python.find(type);
  if (ci != _to_python.end()) {
    // We've previously used this type.
    return (FunctionWriterPtrToPython *)(*ci).second;
  }

  FunctionWriter *writer =
    _function_writers.add_writer(new FunctionWriterPtrToPython(type));
  _to_python.insert(PtrConverter::value_type(type, writer));
  return (FunctionWriterPtrToPython *)writer;
}
