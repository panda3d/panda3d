// Filename: wrapperBuilderC.C
// Created by:  drose (06Aug00)
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
//     Function: WrapperBuilderC::write_wrapper
//       Access: Public, Virtual
//  Description: Generates a wrapper function to the indicated output
//               stream.
////////////////////////////////////////////////////////////////////
void WrapperBuilderC::
write_wrapper(ostream &out, const string &wrapper_name) const {
  out << "/*\n"
      << " * C wrapper for\n"
      << " * " << _description << "\n"
      << " */\n";

  if (!output_function_names) {
    // If we're not saving the function names, don't export it from
    // the library.
    out << "static ";
  }

  if (_void_return) {
    out << "void\n";
  } else {
    out << _return_type->get_new_type()->get_local_name(&parser) << "\n";
  }

  out << wrapper_name << "(";
  int pn = 0;
  if (pn < (int)_parameters.size()) {
    _parameters[pn]._remap->get_new_type()->
      output_instance(out, get_parameter_name(pn), &parser);
    pn++;
    while (pn < (int)_parameters.size()) {
      out << ", ";
      _parameters[pn]._remap->get_new_type()->
	output_instance(out, get_parameter_name(pn), &parser);
      pn++;
    }
  }
  out << ") {\n";

  write_spam_message(out);

  string return_expr = call_function(out, 2);
  return_expr = manage_return_value(out, 2, return_expr);
  if (!return_expr.empty()) {
    out << "  return " << return_expr << ";\n";
  }

  out << "}\n\n";
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

