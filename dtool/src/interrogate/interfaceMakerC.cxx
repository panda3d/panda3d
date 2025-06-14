/**
 * PANDA 3D SOFTWARE
 * Copyright (c) Carnegie Mellon University.  All rights reserved.
 *
 * All use of this software is subject to the terms of the revised BSD
 * license.  You should have received a copy of this license along
 * with this source code in a file named "LICENSE."
 *
 * @file interfaceMakerC.cxx
 * @author drose
 * @date 2001-09-25
 */

#include "interfaceMakerC.h"
#include "interrogateBuilder.h"
#include "interrogate.h"
#include "functionRemap.h"
#include "parameterRemapHandleToInt.h"
#include "parameterRemapUnchanged.h"
#include "typeManager.h"

#include "interrogateDatabase.h"
#include "interrogateType.h"
#include "interrogateFunction.h"
#include "cppFunctionType.h"

using std::ostream;

/**
 *
 */
InterfaceMakerC::
InterfaceMakerC(InterrogateModuleDef *def) :
  InterfaceMaker(def)
{
}

/**
 *
 */
InterfaceMakerC::
~InterfaceMakerC() {
}

/**
 * Generates the list of function prototypes corresponding to the functions
 * that will be output in write_functions().
 */
void InterfaceMakerC::
write_prototypes(ostream &out,ostream *out_h) {
  // The 'used' attribute prevents emscripten from optimizing it out.
  out <<
    "#if __GNUC__ >= 4\n"
    "#define EXPORT_FUNC extern \"C\" __attribute__((used, visibility(\"default\")))\n"
    "#else\n"
    "#define EXPORT_FUNC extern \"C\"\n"
    "#endif\n\n";

  FunctionsByIndex::iterator fi;
  for (fi = _functions.begin(); fi != _functions.end(); ++fi) {
    Function *func = (*fi).second;
    write_prototype_for(out, func);
  }

  out << "\n";
  InterfaceMaker::write_prototypes(out,out_h);
}

/**
 * Generates the list of functions that are appropriate for this interface.
 * This function is called *before* write_prototypes(), above.
 */
void InterfaceMakerC::
write_functions(ostream &out) {
  FunctionsByIndex::iterator fi;
  for (fi = _functions.begin(); fi != _functions.end(); ++fi) {
    Function *func = (*fi).second;
    write_function_for(out, func);
  }

  InterfaceMaker::write_functions(out);
}

/**
 * Allocates a new ParameterRemap object suitable to the indicated parameter
 * type.  If struct_type is non-NULL, it is the type of the enclosing class
 * for the function (method) in question.
 *
 * The return value is a newly-allocated ParameterRemap object, if the
 * parameter type is acceptable, or NULL if the parameter type cannot be
 * handled.
 */
ParameterRemap *InterfaceMakerC::
remap_parameter(CPPType *struct_type, CPPType *param_type) {
  // Wrap TypeHandle and ButtonHandle, which are practically just ints, as an
  // integer instead of a pointer.  It makes things easier on the scripting
  // language, especially if there has to be a dynamic downcasting system on
  // the scripting language side.
   if (TypeManager::is_handle(param_type)) {
    return new ParameterRemapHandleToInt(param_type);

  } else {
    return InterfaceMaker::remap_parameter(struct_type, param_type);
  }
}

/**
 * This method should be overridden and redefined to return true for
 * interfaces that require the implicit "this" parameter, if present, to be
 * passed as the first parameter to any wrapper functions.
 */
bool InterfaceMakerC::
synthesize_this_parameter() {
  return true;
}

/**
 * Returns the prefix string used to generate wrapper function names.
 */
std::string InterfaceMakerC::
get_wrapper_prefix() {
  return "_inC";
}

/**
 * Returns the prefix string used to generate unique symbolic names, which are
 * not necessarily C-callable function names.
 */
std::string InterfaceMakerC::
get_unique_prefix() {
  return "c";
}

/**
 * Associates the function wrapper with its function in the appropriate
 * structures in the database.
 */
void InterfaceMakerC::
record_function_wrapper(InterrogateFunction &ifunc,
                        FunctionWrapperIndex wrapper_index) {
  ifunc._c_wrappers.push_back(wrapper_index);
}

/**
 * Writes the prototype for the indicated function.
 */
void InterfaceMakerC::
write_prototype_for(ostream &out, InterfaceMaker::Function *func) {
  Function::Remaps::const_iterator ri;

  for (ri = func->_remaps.begin(); ri != func->_remaps.end(); ++ri) {
    FunctionRemap *remap = (*ri);

    if (remap->_extension || (remap->_flags & FunctionRemap::F_explicit_self)) {
      continue;
    }

    if (output_function_names) {
      out << "EXPORT_FUNC ";
    }
    write_function_header(out, func, remap, false);
    out << ";\n";
  }
}

/**
 * Writes the definition for a function that will call the indicated C++
 * function or method.
 */
void InterfaceMakerC::
write_function_for(ostream &out, InterfaceMaker::Function *func) {
  Function::Remaps::const_iterator ri;

  for (ri = func->_remaps.begin(); ri != func->_remaps.end(); ++ri) {
    FunctionRemap *remap = (*ri);
    write_function_instance(out, func, remap);
  }
}

/**
 * Writes out the particular function that handles a single instance of an
 * overloaded function.
 */
void InterfaceMakerC::
write_function_instance(ostream &out, InterfaceMaker::Function *func,
                        FunctionRemap *remap) {
  if (remap->_extension || (remap->_flags & FunctionRemap::F_explicit_self)) {
    return;
  }

  out << "/*\n"
      << " * C wrapper for\n"
      << " * ";
  remap->write_orig_prototype(out, 0, false, remap->_num_default_parameters);
  out << "\n"
      << " */\n";

  if (!output_function_names) {
    // If we're not saving the function names, don't export it from the
    // library.
    out << "static ";
  }

  write_function_header(out, func, remap, true);
  out << " {\n";

  if (generate_spam) {
    write_spam_message(out, remap);
  }

  std::string return_expr =
    remap->call_function(out, 2, true, "param0");
  return_expr = manage_return_value(out, 2, remap, return_expr);
  if (!return_expr.empty()) {
    out << "  return " << return_expr << ";\n";
  }

  out << "}\n\n";
}

/**
 * Writes the first line of a function definition, either for a prototype or a
 * function body.
 */
void InterfaceMakerC::
write_function_header(ostream &out, InterfaceMaker::Function *func,
                      FunctionRemap *remap, bool newline) {
  if (remap->_void_return) {
    out << "void";
  } else {
    out << remap->_return_type->get_new_type()->get_local_name(&parser);
  }
  if (newline) {
    out << "\n";
  } else {
    out << " ";
  }

  out << remap->_wrapper_name << "(";
  int pn = 0;
  if (pn < (int)remap->_parameters.size()) {
    remap->_parameters[pn]._remap->get_new_type()->
      output_instance(out, remap->get_parameter_name(pn), &parser);
    pn++;
    while (pn < (int)remap->_parameters.size()) {
      out << ", ";
      remap->_parameters[pn]._remap->get_new_type()->
        output_instance(out, remap->get_parameter_name(pn), &parser);
      pn++;
    }
  }
  out << ")";
}
