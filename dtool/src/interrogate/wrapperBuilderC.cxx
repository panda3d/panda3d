// Filename: wrapperBuilderC.cxx
// Created by:  drose (06Aug00)
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

#include "wrapperBuilderC.h"
#include "interrogate.h"
#include "parameterRemap.h"

#include <interrogateDatabase.h>
#include <cppInstance.h>
#include <cppFunctionType.h>
#include <cppParameterList.h>
#include <cppConstType.h>
#include <cppReferenceType.h>
#include <cppPointerType.h>
#include <cppSimpleType.h>
#include <cppStructType.h>
#include <notify.h>

////////////////////////////////////////////////////////////////////
//     Function: WrapperBuilderC::Constructor
//       Access: Public
//  Description:
////////////////////////////////////////////////////////////////////
WrapperBuilderC::
WrapperBuilderC() {
}

////////////////////////////////////////////////////////////////////
//     Function: WrapperBuilderC::write_prototype
//       Access: Public, Virtual
//  Description: Generates the prototype for the wrapper function(s).
////////////////////////////////////////////////////////////////////
void WrapperBuilderC::
write_prototype(ostream &out, const string &wrapper_name) const {
  for (int def_index = 0; def_index < (int)_def.size(); ++def_index) {
    const FunctionDef *def = _def[def_index];

    if (!output_function_names) {
      // If we're not saving the function names, don't export it from
      // the library.
      out << "static ";
    } else {
      out << "extern \"C\" ";
    }
    
    if (def->_void_return) {
      out << "void " << wrapper_name;
    } else {
      def->_return_type->get_new_type()->output_instance(out, wrapper_name, &parser);
    }
    
    out << "(";
    int pn = 0;
    if (pn < (int)def->_parameters.size()) {
      def->_parameters[pn]._remap->get_new_type()->
        output_instance(out, get_parameter_name(pn), &parser);
      pn++;
      while (pn < (int)def->_parameters.size()) {
        out << ", ";
        def->_parameters[pn]._remap->get_new_type()->
          output_instance(out, get_parameter_name(pn), &parser);
        pn++;
      }
    }
    out << ");\n";
  }
}

////////////////////////////////////////////////////////////////////
//     Function: WrapperBuilderC::write_wrapper
//       Access: Public, Virtual
//  Description: Generates a wrapper function to the indicated output
//               stream.
////////////////////////////////////////////////////////////////////
void WrapperBuilderC::
write_wrapper(ostream &out, const string &wrapper_name) const {
  for (int def_index = 0; def_index < (int)_def.size(); ++def_index) {
    const FunctionDef *def = _def[def_index];

    out << "/*\n"
        << " * C wrapper for\n"
        << " * " << def->_description << "\n"
        << " */\n";
    
    if (!output_function_names) {
      // If we're not saving the function names, don't export it from
      // the library.
      out << "static ";
    }
    
    if (def->_void_return) {
      out << "void\n";
    } else {
      out << def->_return_type->get_new_type()->get_local_name(&parser) << "\n";
    }
    
    out << wrapper_name << "(";
    int pn = 0;
    if (pn < (int)def->_parameters.size()) {
      def->_parameters[pn]._remap->get_new_type()->
        output_instance(out, get_parameter_name(pn), &parser);
      pn++;
      while (pn < (int)def->_parameters.size()) {
        out << ", ";
        def->_parameters[pn]._remap->get_new_type()->
          output_instance(out, get_parameter_name(pn), &parser);
        pn++;
      }
    }
    out << ") {\n";
    
    write_spam_message(def_index, out);
    
    string return_expr = call_function(def_index, out, 2, true, "param0");
    return_expr = manage_return_value(def_index, out, 2, return_expr);
    if (!return_expr.empty()) {
      out << "  return " << return_expr << ";\n";
    }
    
    out << "}\n\n";
  }
}

////////////////////////////////////////////////////////////////////
//     Function: WrapperBuilderC::get_wrapper_name
//       Access: Public, Virtual
//  Description: Returns the callable name for this wrapper function.
////////////////////////////////////////////////////////////////////
string WrapperBuilderC::
get_wrapper_name(const string &library_hash_name) const {
  return "_inC" + library_hash_name + _hash;
}

////////////////////////////////////////////////////////////////////
//     Function: WrapperBuilderC::supports_atomic_strings
//       Access: Public, Virtual
//  Description: Returns true if this kind of wrapper can support true
//               atomic string objects (and not have to fiddle with
//               char *).
////////////////////////////////////////////////////////////////////
bool WrapperBuilderC::
supports_atomic_strings() const {
  return false;
}

////////////////////////////////////////////////////////////////////
//     Function: WrapperBuilderC::get_calling_convention
//       Access: Public, Virtual
//  Description: Returns an indication of what kind of function we are
//               building.
////////////////////////////////////////////////////////////////////
WrapperBuilder::CallingConvention WrapperBuilderC::
get_calling_convention() const {
  return CC_c;
}

