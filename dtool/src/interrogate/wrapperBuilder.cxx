// Filename: wrapperBuilder.cxx
// Created by:  drose (01Aug00)
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

#include "wrapperBuilder.h"
#include "interrogate.h"
#include "parameterRemap.h"
#include "parameterRemapThis.h"
#include "parameterRemapUnchanged.h"
#include "parameterRemapReferenceToPointer.h"
#include "parameterRemapConcreteToPointer.h"
#include "parameterRemapEnumToInt.h"
#include "parameterRemapConstToNonConst.h"
#include "parameterRemapReferenceToConcrete.h"
#include "parameterRemapCharStarToString.h"
#include "parameterRemapBasicStringToString.h"
#include "parameterRemapBasicStringRefToString.h"
#include "parameterRemapPTToPointer.h"
#include "interrogateBuilder.h"
#include "typeManager.h"

#include <interrogateDatabase.h>
#include <cppInstance.h>
#include <cppFunctionType.h>
#include <cppParameterList.h>
#include <cppStructType.h>
#include <cppReferenceType.h>
#include <notify.h>

////////////////////////////////////////////////////////////////////
//     Function: WrapperBuilder::FunctionDef::Constructor
//       Access: Public
//  Description: 
////////////////////////////////////////////////////////////////////
WrapperBuilder::FunctionDef::
FunctionDef() {
  _return_type = (ParameterRemap *)NULL;
  _void_return = true;
  _has_this = false;
  _is_method = false;
  _type = T_normal;

  _function = (CPPInstance *)NULL;
  _struct_type = (CPPStructType *)NULL;
  _scope = (CPPScope *)NULL;
  _ftype = (CPPFunctionType *)NULL;
  _num_default_parameters = 0;

  _return_value_needs_management = false;
  _return_value_destructor = 0;
  _manage_reference_count = false;
}

////////////////////////////////////////////////////////////////////
//     Function: WrapperBuilder::FunctionDef::Destructor
//       Access: Public
//  Description: 
////////////////////////////////////////////////////////////////////
WrapperBuilder::FunctionDef::
~FunctionDef() {
  Parameters::iterator pi;
  for (pi = _parameters.begin(); pi != _parameters.end(); ++pi) {
    delete (*pi)._remap;
  }
  _parameters.clear();

  if (_return_type != (ParameterRemap *)NULL) {
    delete _return_type;
    _return_type = (ParameterRemap *)NULL;
  }
}

////////////////////////////////////////////////////////////////////
//     Function: WrapperBuilder::FunctionDef::Copy constructor
//       Access: Private
//  Description: These are not designed to be copied.
////////////////////////////////////////////////////////////////////
WrapperBuilder::FunctionDef::
FunctionDef(const FunctionDef &) {
  assert(false);
}

////////////////////////////////////////////////////////////////////
//     Function: WrapperBuilder::FunctionDef::Copy assignment operator
//       Access: Private
//  Description: These are not designed to be copied.
////////////////////////////////////////////////////////////////////
void WrapperBuilder::FunctionDef::
operator = (const FunctionDef &) {
  assert(false);
}


////////////////////////////////////////////////////////////////////
//     Function: WrapperBuilder::Constructor
//       Access: Public
//  Description:
////////////////////////////////////////////////////////////////////
WrapperBuilder::
WrapperBuilder() {
  _wrapper_index = 0;
  _is_valid = false;
}

////////////////////////////////////////////////////////////////////
//     Function: WrapperBuilder::Destructor
//       Access: Public, Virtual
//  Description:
////////////////////////////////////////////////////////////////////
WrapperBuilder::
~WrapperBuilder() {
  Def::iterator di;
  for (di = _def.begin(); di != _def.end(); ++di) {
    delete (*di);
  }
}

////////////////////////////////////////////////////////////////////
//     Function: WrapperBuilder::add_function
//       Access: Public
//  Description: Sets up the builder according to the indicated
//               function.  The value of num_default_parameters
//               represents the number of parameters on the end to
//               assign to their default values for this particular
//               wrapper.
//
//               If this is called only once, then the WrapperBuilder
//               will generate a wrapper that only calls the
//               particular C++ function.  Otherwise, if this is
//               called multiple times (and the WrapperBuilder type
//               supports this), it will generate a wrapper that can
//               handle overloading, and will call the appropriate
//               C++ function based on the input parameters.
//
//               Returns the definition index if successful, or -1 if
//               the function cannot be wrapped for some reason.
////////////////////////////////////////////////////////////////////
int WrapperBuilder::
add_function(CPPInstance *function, const string &description,
             CPPStructType *struct_type,
             CPPScope *scope, const string &function_signature,
             WrapperBuilder::Type type,
             const string &expression,
             int num_default_parameters) {
  _is_valid = true;
  int def_index = (int)_def.size();
  FunctionDef *def = new FunctionDef;
  _def.push_back(def);

  def->_function = function;
  def->_description = description;
  def->_struct_type = struct_type;
  def->_scope = scope;
  def->_function_signature = function_signature;
  def->_expression = expression;
  def->_num_default_parameters = num_default_parameters;
  def->_parameters.clear();

  def->_ftype = def->_function->_type->resolve_type(scope, &parser)->as_function_type();
  assert(def->_ftype != (CPPFunctionType *)NULL);

  def->_has_this = false;
  def->_is_method = false;
  def->_type = type;

  if ((def->_ftype->_flags & CPPFunctionType::F_constructor) != 0) {
    def->_type = T_constructor;

  } else if ((def->_ftype->_flags & CPPFunctionType::F_destructor) != 0) {
    def->_type = T_destructor;

  } else if ((def->_ftype->_flags & CPPFunctionType::F_operator_typecast) != 0) {
    def->_type = T_typecast_method;
  }

  if (def->_struct_type != (CPPStructType *)NULL &&
      ((function->_storage_class & CPPInstance::SC_static) == 0) &&
      def->_type != T_constructor) {

    // If this is a method, but not a static method, and not a
    // constructor, then we may need to synthesize a "this" parameter.
    def->_is_method = true;

    if (synthesize_this_parameter()) {
      Parameter param;
      param._name = "this";
      param._has_name = true;
      bool is_const = (def->_ftype->_flags & CPPFunctionType::F_const_method) != 0;
      param._remap = new ParameterRemapThis(def->_struct_type, is_const);
      def->_parameters.push_back(param);
      def->_has_this = true;
    }
      
    // Also check the name of the function.  If it's one of the
    // assignment-style operators, flag it as such.
    string fname = def->_function->get_simple_name();
    if (fname == "operator =" ||
        fname == "operator *=" ||
        fname == "operator /=" ||
        fname == "operator %=" ||
        fname == "operator +=" ||
        fname == "operator -=" ||
        fname == "operator |=" ||
        fname == "operator &=" ||
        fname == "operator ^=" ||
        fname == "operator <<=" ||
        fname == "operator >>=") {
      def->_type = T_assignment_method;
    }
  }

  const CPPParameterList::Parameters &params =
    def->_ftype->_parameters->_parameters;
  for (int i = 0; i < (int)params.size() - num_default_parameters; i++) {
    CPPType *type = params[i]->_type->resolve_type(&parser, def->_scope);
    Parameter param;
    param._has_name = true;
    param._name = params[i]->get_simple_name();

    if (param._name.empty()) {
      // If the parameter has no name, record it as being nameless,
      // but also synthesize one in case someone asks anyway.
      param._has_name = false;
      ostringstream param_name;
      param_name << "param" << i;
      param._name = param_name.str();
    }

    param._remap = make_remap(def_index, type);
    param._remap->set_default_value(params[i]->_initializer);

    if (!param._remap->is_valid()) {
      _is_valid = false;
    }

    def->_parameters.push_back(param);
  }

  if (def->_type == T_constructor) {
    // Constructors are a special case.  These appear to return void
    // as seen by the parser, but we know they actually return a new
    // concrete instance.

    if (def->_struct_type == (CPPStructType *)NULL) {
      nout << "Method " << *def->_function << " has no struct type\n";
      _is_valid = false;
    } else {
      def->_return_type = make_remap(def_index, def->_struct_type);
      def->_void_return = false;
    }

  } else if (def->_type == T_assignment_method) {
    // Assignment-type methods are also a special case.  We munge
    // these to return *this, which is a semi-standard C++ convention
    // anyway.  We just enforce it.

    if (def->_struct_type == (CPPStructType *)NULL) {
      nout << "Method " << *def->_function << " has no struct type\n";
      _is_valid = false;
    } else {
      CPPType *ref_type = CPPType::new_type(new CPPReferenceType(def->_struct_type));
      def->_return_type = make_remap(def_index, ref_type);
      def->_void_return = false;
    }

  } else {
    // The normal case.
    CPPType *rtype = def->_ftype->_return_type->resolve_type(&parser, def->_scope);
    def->_return_type = make_remap(def_index, rtype);
    def->_void_return = TypeManager::is_void(rtype);
  }

  if (!def->_return_type->is_valid()) {
    _is_valid = false;
  }

  // Do we need to manage the return value?
  def->_return_value_needs_management = 
    def->_return_type->return_value_needs_management();
  def->_return_value_destructor = 
    def->_return_type->get_return_value_destructor();

  // Should we manage a reference count?
  CPPType *return_type = def->_return_type->get_new_type();
  CPPType *return_meat_type = TypeManager::unwrap_pointer(return_type);

  if (manage_reference_counts &&
      TypeManager::is_reference_count_pointer(return_type) &&
      !TypeManager::has_protected_destructor(return_meat_type)) {
    // Yes!
    def->_manage_reference_count = true;
    def->_return_value_needs_management = true;

    // This is problematic, because we might not have the class in
    // question fully defined here, particularly if the class is
    // defined in some other library.
    def->_return_value_destructor = builder.get_destructor_for(return_meat_type);
  }

  if (!_is_valid) {
    // Invalid function wrapper.  Too bad.
    _def.pop_back();
    delete def;
    return -1;
  }

  return def_index;
}

////////////////////////////////////////////////////////////////////
//     Function: WrapperBuilder::get_function_writers
//       Access: Public, Virtual
//  Description: Adds whatever list of FunctionWriters might be needed
//               for this particular WrapperBuilder.  These will be
//               generated to the output source file before
//               write_wrapper() is called.
////////////////////////////////////////////////////////////////////
void WrapperBuilder::
get_function_writers(FunctionWriters &) {
}

////////////////////////////////////////////////////////////////////
//     Function: WrapperBuilder::synthesize_this_parameter
//       Access: Public, Virtual
//  Description: Returns true if this particular wrapper type requires
//               an explicit "this" parameter to be added to the
//               function parameter list when appropriate, or false if
//               the "this" pointer will come through a different
//               channel.
////////////////////////////////////////////////////////////////////
bool WrapperBuilder::
synthesize_this_parameter() const {
  return true;
}


////////////////////////////////////////////////////////////////////
//     Function: WrapperBuilder::make_remap
//       Access: Protected, Virtual
//  Description: Allocates a new ParameterRemap object suitable to the
//               indicated type.
////////////////////////////////////////////////////////////////////
ParameterRemap *WrapperBuilder::
make_remap(int def_index, CPPType *orig_type) {
  assert(def_index >= 0 && def_index < (int)_def.size());
  const FunctionDef *def = _def[def_index];

  if (convert_strings) {
    if (TypeManager::is_char_pointer(orig_type)) {
      return new ParameterRemapCharStarToString(orig_type);
    }

    // If we're exporting a method of basic_string<char> itself, don't
    // convert basic_string<char>'s to atomic strings.

    if (def->_struct_type == (CPPStructType *)NULL ||
        !TypeManager::is_basic_string_char(def->_struct_type)) {
      if (TypeManager::is_basic_string_char(orig_type)) {
        return new ParameterRemapBasicStringToString(orig_type);

      } else if (TypeManager::is_const_ref_to_basic_string_char(orig_type)) {
        return new ParameterRemapBasicStringRefToString(orig_type);
      }
    }
  }

  if (manage_reference_counts) {
    if (TypeManager::is_pointer_to_base(orig_type) ||
        TypeManager::is_const_ref_to_pointer_to_base(orig_type)) {
      CPPType *pt_type = TypeManager::unwrap_reference(orig_type);

      // Don't convert PointerTo<>'s to pointers for methods of the
      // PointerTo itself!
      if (def->_struct_type == (CPPStructType *)NULL ||
          !(pt_type->get_local_name(&parser) == def->_struct_type->get_local_name(&parser))) {
        return new ParameterRemapPTToPointer(orig_type);
      }
    }
  }

  if (TypeManager::is_reference(orig_type)) {
    return new ParameterRemapReferenceToPointer(orig_type);

  } else if (TypeManager::is_struct(orig_type)) {
    return new ParameterRemapConcreteToPointer(orig_type);

    /*
  } else if (TypeManager::is_enum(orig_type) || TypeManager::is_const_ref_to_enum(orig_type)) {
    return new ParameterRemapEnumToInt(orig_type);
    */

  } else if (TypeManager::is_const_simple(orig_type)) {
    return new ParameterRemapConstToNonConst(orig_type);

  } else if (TypeManager::is_const_ref_to_simple(orig_type)) {
    return new ParameterRemapReferenceToConcrete(orig_type);

  } else if (TypeManager::is_pointer(orig_type) || TypeManager::is_simple(orig_type)) {
    return new ParameterRemapUnchanged(orig_type);

  } else {
    // Here's something we have a problem with.
    _is_valid = false;
    return new ParameterRemapUnchanged(orig_type);
  }
}

////////////////////////////////////////////////////////////////////
//     Function: WrapperBuilder::manage_return_value
//       Access: Protected
//  Description: Does any additional processing that we might want to
//               do on the return value for the function, just before
//               we return it.  Returns the string representing the
//               new return value after processing.
////////////////////////////////////////////////////////////////////
string WrapperBuilder::
manage_return_value(int def_index, 
                    ostream &out, int indent_level,
                    const string &return_expr) const {
  assert(def_index >= 0 && def_index < (int)_def.size());
  const FunctionDef *def = _def[def_index];
  if (def->_manage_reference_count) {
    // If we're managing reference counts, and we're about to return a
    // reference countable object, then increment its count.
    if (return_expr == "return_value") {
      // If the expression is just a variable name, we can just ref it
      // directly.
      output_ref(def_index, out, indent_level, return_expr);
      return return_expr;

    } else {
      // Otherwise, we should probably assign it to a temporary first,
      // so we don't invoke the function twice or something.
      CPPType *type = def->_return_type->get_temporary_type();
      indent(out, indent_level);
      type->output_instance(out, "refcount", &parser);
      out << " = " << return_expr << ";\n";

      indent(out, indent_level)
        << "if (" << return_expr << " != ("
        << def->_return_type->get_new_type()->get_local_name(&parser) << ")0) {\n";
      indent(out, indent_level + 2)
        << return_expr << "->ref();\n";
      indent(out, indent_level)
        << "}\n";
      output_ref(def_index, out, indent_level, "refcount");
      return def->_return_type->temporary_to_return("refcount");
    }
  }

  // Otherwise, just return the expression unchanged.
  return return_expr;
}

////////////////////////////////////////////////////////////////////
//     Function: WrapperBuilder::output_ref
//       Access: Protected
//  Description: Outputs the code to increment the reference count for
//               the indicated variable name.
////////////////////////////////////////////////////////////////////
void WrapperBuilder::
output_ref(int def_index, ostream &out, int indent_level, const string &varname) const {
  assert(def_index >= 0 && def_index < (int)_def.size());
  const FunctionDef *def = _def[def_index];
  if (def->_type == T_constructor || def->_type == T_typecast) {
    // In either of these cases, we can safely assume the pointer will
    // never be NULL.
    indent(out, indent_level)
      << varname << "->ref();\n";

  } else {
    // However, in the general case, we have to check for that before
    // we attempt to ref it.

    indent(out, indent_level)
      << "if (" << varname << " != ("
      << def->_return_type->get_new_type()->get_local_name(&parser) << ")0) {\n";
    indent(out, indent_level + 2)
      << varname << "->ref();\n";
    indent(out, indent_level)
      << "}\n";
  }
}

////////////////////////////////////////////////////////////////////
//     Function: WrapperBuilder::get_parameter_name
//       Access: Protected
//  Description: Returns a suitable name for the nth parameter.
////////////////////////////////////////////////////////////////////
string WrapperBuilder::
get_parameter_name(int n) const {
  /*
  if (_has_this) {
    if (n == 0) {
      return "container";
    }
    n--;
  }
  */

  ostringstream str;
  str << "param" << n;
  return str.str();
}

////////////////////////////////////////////////////////////////////
//     Function: WrapperBuilder::get_parameter_expr
//       Access: Protected
//  Description: Returns a string that represents the expression
//               associated with the nth parameter.  This is just the
//               nth element of pexprs if it is nonempty, or the name
//               of the nth parameter is it is empty.
////////////////////////////////////////////////////////////////////
string WrapperBuilder::
get_parameter_expr(int n, const vector_string &pexprs) const {
  if (n < (int)pexprs.size()) {
    return pexprs[n];
  }
  return get_parameter_name(n);
}

////////////////////////////////////////////////////////////////////
//     Function: WrapperBuilder::get_call_str
//       Access: Protected
//  Description: Returns a string suitable for calling the wrapped
//               function.  If pexprs is nonempty, it represents
//               the list of expressions that will evaluate to each
//               parameter value.
////////////////////////////////////////////////////////////////////
string WrapperBuilder::
get_call_str(int def_index,
             const string &container, const vector_string &pexprs) const {
  assert(def_index >= 0 && def_index < (int)_def.size());
  const FunctionDef *def = _def[def_index];

  // Build up the call to the actual function.
  ostringstream call;

  // Getters and setters are a special case.
  if (def->_type == T_getter) {
    if (!container.empty()) {
      call << "(" << container << ")->" << def->_expression;
    } else {
      call << def->_expression;
    }
    assert(def->_parameters.empty());

  } else if (def->_type == T_setter) {
    if (!container.empty()) {
      call << "(" << container << ")->" << def->_expression;
    } else {
      call << def->_expression;
    }

    call << " = ";
    assert(def->_parameters.size() == 1);
    def->_parameters[0]._remap->pass_parameter(call, get_parameter_expr(0, pexprs));

  } else {
    int pn = 0;
    if (def->_type == T_constructor) {
      // Constructors are called differently.
      call << def->_struct_type->get_local_name(&parser);

    } else if (!container.empty()) {
      // If we have a "this" parameter, the calling convention is also
      // a bit different.
      call << "(" << container << ")->" << def->_function->get_local_name();
      if (def->_has_this) {
        pn++;
      }
      
    } else {
      call << def->_function->get_local_name(&parser);
    }

    call << "(";
    if (pn < (int)def->_parameters.size()) {
      def->_parameters[pn]._remap->pass_parameter(call, get_parameter_expr(pn, pexprs));
      pn++;
      while (pn < (int)def->_parameters.size()) {
        call << ", ";
        def->_parameters[pn]._remap->pass_parameter(call, get_parameter_expr(pn, pexprs));
        pn++;
      }
    }
    call << ")";
  }

  return call.str();
}


////////////////////////////////////////////////////////////////////
//     Function: WrapperBuilder::call_function
//       Access: Protected
//  Description: Writes a sequence of commands to the given output
//               stream to call the wrapped function.  The parameter
//               values are taken from pexprs, if it is nonempty, or
//               are assumed to be simply the names of the parameters,
//               if it is empty.
//
//               The return value is the expression to return, if we
//               are returning a value, or the empty string if we
//               return nothing.
////////////////////////////////////////////////////////////////////
string WrapperBuilder::
call_function(int def_index,
              ostream &out, int indent_level, bool convert_result,
              const string &container, const vector_string &pexprs) const {
  assert(def_index >= 0 && def_index < (int)_def.size());
  const FunctionDef *def = _def[def_index];

  string return_expr;

  if (def->_type == T_destructor) {
    // A destructor wrapper is just a wrapper around the delete operator.
    assert(!container.empty());
    assert(def->_struct_type != (CPPStructType *)NULL);

    if (TypeManager::is_reference_count(def->_struct_type)) {
      // Except for a reference-count type object, in which case the
      // destructor is a wrapper around unref_delete().
      indent(out, indent_level)
        << "unref_delete(" << container << ");\n";
    } else {
      indent(out, indent_level)
        << "delete " << container << ";\n";
    }

  } else if (def->_type == T_typecast_method) {
    // A typecast method can be invoked implicitly.
    string cast_expr =
      "(" + def->_return_type->get_orig_type()->get_local_name(&parser) +
      ")(*" + container + ")";

    if (!convert_result) {
      return_expr = cast_expr;
    } else {
      string new_str =
        def->_return_type->prepare_return_expr(out, indent_level, cast_expr);
      return_expr = def->_return_type->get_return_expr(new_str);
    }

  } else if (def->_type == T_typecast) {
    // A regular typecast converts from a pointer type to another
    // pointer type.  (This is different from the typecast method,
    // above, which converts from the concrete type to some other
    // type.)
    assert(!container.empty());
    string cast_expr =
      "(" + def->_return_type->get_orig_type()->get_local_name(&parser) +
      ")" + container;

    if (!convert_result) {
      return_expr = cast_expr;
    } else {
      string new_str =
        def->_return_type->prepare_return_expr(out, indent_level, cast_expr);
      return_expr = def->_return_type->get_return_expr(new_str);
    }

  } else if (def->_type == T_constructor) {
    // A special case for constructors.
    return_expr = "new " + get_call_str(def_index, container, pexprs);

  } else if (def->_type == T_assignment_method) {
    // Another special case for assignment operators.
    assert(!container.empty());
    indent(out, indent_level)
      << get_call_str(def_index, container, pexprs) << ";\n";

    string this_expr = container;
    string ref_expr = "*" + this_expr;

    if (!convert_result) {
      return_expr = ref_expr;
    } else {
      string new_str =
        def->_return_type->prepare_return_expr(out, indent_level, ref_expr);
      return_expr = def->_return_type->get_return_expr(new_str);

      // Now a simple special-case test.  Often, we will have converted
      // the reference-returning assignment operator to a pointer.  In
      // this case, we might inadventent generate code like "return
      // &(*this)", when "return this" would do.  We check for this here
      // and undo it as a special case.

      // There's no real good reason to do this, other than that it
      // feels more satisfying to a casual perusal of the generated
      // code.  It *is* conceivable that some broken compilers wouldn't
      // like "&(*this)", though.

      if (return_expr == "&(" + ref_expr + ")" ||
          return_expr == "&" + ref_expr) {
        return_expr = this_expr;
      }
    }

  } else if (def->_void_return) {
    indent(out, indent_level)
      << get_call_str(def_index, container, pexprs) << ";\n";

  } else {
    string call = get_call_str(def_index, container, pexprs);

    if (!convert_result) {
      return_expr = get_call_str(def_index, container, pexprs);

    } else {
      if (def->_return_type->return_value_should_be_simple()) {
        // We have to assign the result to a temporary first; this makes
        // it a bit easier on poor old VC++.
        indent(out, indent_level);
        def->_return_type->get_orig_type()->output_instance(out, "result",
                                                           &parser);
        out << " = " << call << ";\n";

        string new_str =
          def->_return_type->prepare_return_expr(out, indent_level, "result");
        return_expr = def->_return_type->get_return_expr(new_str);

      } else {
        // This should be simple enough that we can return it directly.
        string new_str =
          def->_return_type->prepare_return_expr(out, indent_level, call);
        return_expr = def->_return_type->get_return_expr(new_str);
      }
    }
  }

  return return_expr;
}

////////////////////////////////////////////////////////////////////
//     Function: WrapperBuilder::write_spam_message
//       Access: Protected
//  Description: Generates the lines of code at the beginning of the
//               wrapper function that output a message when the
//               wrapper is called.  This is output only if -spam is
//               specified on the command line.
////////////////////////////////////////////////////////////////////
void WrapperBuilder::
write_spam_message(int def_index, ostream &out) const {
  assert(def_index >= 0 && def_index < (int)_def.size());
  const FunctionDef *def = _def[def_index];

  if (generate_spam) {
    out << "#ifndef NDEBUG\n"
        << "  if (in_" << library_name << "_cat.is_spam()) {\n"
        << "    in_" << library_name << "_cat.spam()\n"
        << "      << \"";
    write_quoted_string(out, def->_description);
    out << "\\n\";\n"
        << "  }\n"
        << "#endif\n";
  }
}

////////////////////////////////////////////////////////////////////
//     Function: WrapperBuilder::write_quoted_string
//       Access: Protected
//  Description: Writes the string to the given output stream, as if
//               it were quoted within double quotes in a C program.
//               Specifically, this escapes characters that need to be
//               escaped, and otherwise leaves the string unchanged.
////////////////////////////////////////////////////////////////////
void WrapperBuilder::
write_quoted_string(ostream &out, const string &str) const {
  string::const_iterator si;
  for (si = str.begin(); si != str.end(); ++si) {
    switch (*si) {
    case '\n':
      out << "\\n";
      break;

    case '\t':
      out << "\\t";
      break;

    case '\\':
    case '"':
      out << '\\';
      // fall through

    default:
      out << *si;
    }
  }
}

////////////////////////////////////////////////////////////////////
//     Function: WrapperBuilder::indent
//       Access: Public, Static
//  Description:
////////////////////////////////////////////////////////////////////
ostream &WrapperBuilder::
indent(ostream &out, int indent_level) {
  for (int i = 0; i < indent_level; i++) {
    out << ' ';
  }
  return out;
}
