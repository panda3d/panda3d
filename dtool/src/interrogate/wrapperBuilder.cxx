// Filename: wrapperBuilder.C
// Created by:  drose (01Aug00)
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
//     Function: WrapperBuilder::Constructor
//       Access: Public
//  Description: 
////////////////////////////////////////////////////////////////////
WrapperBuilder::
WrapperBuilder() {
  _return_type = (ParameterRemap *)NULL;
  _void_return = true;
  _has_this = false;
  _type = T_normal;

  _function = (CPPInstance *)NULL;
  _struct_type = (CPPStructType *)NULL;
  _scope = (CPPScope *)NULL;
  _ftype = (CPPFunctionType *)NULL;
  _num_default_parameters = 0;
  _wrapper_index = 0;
  
  _is_valid = false;
  _return_value_needs_management = false;
  _return_value_destructor = 0;
  _manage_reference_count = false;
}
 
////////////////////////////////////////////////////////////////////
//     Function: WrapperBuilder::Destructor
//       Access: Public
//  Description: 
////////////////////////////////////////////////////////////////////
WrapperBuilder::
~WrapperBuilder() {
  clear();
}

////////////////////////////////////////////////////////////////////
//     Function: WrapperBuilder::clear
//       Access: Public
//  Description: Empties the builder and prepares it to receive a new
//               function.
////////////////////////////////////////////////////////////////////
void WrapperBuilder::
clear() {
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
//     Function: WrapperBuilder::set_function
//       Access: Public
//  Description: Sets up the builder according to the indicated
//               function.  The value of num_default_parameters
//               represents the number of parameters on the end to
//               assign to their default values for this particular
//               wrapper.
//
//               Returns true if successful, false if the function
//               cannot be wrapped for some reason.
////////////////////////////////////////////////////////////////////
bool WrapperBuilder::
set_function(CPPInstance *function, const string &description,
             CPPStructType *struct_type,
             CPPScope *scope, const string &function_signature,
             WrapperBuilder::Type type, 
             const string &expression,
             int num_default_parameters) {
  clear();

  _function = function;
  _description = description;
  _struct_type = struct_type;
  _scope = scope;
  _function_signature = function_signature;
  _expression = expression;
  _num_default_parameters = num_default_parameters;
  _parameters.clear();

  _ftype = _function->_type->resolve_type(scope, &parser)->as_function_type();
  assert(_ftype != (CPPFunctionType *)NULL);
  
  _is_valid = true;
  _has_this = false;
  _type = type;
  _wrapper_index = 0;

  if ((_ftype->_flags & CPPFunctionType::F_constructor) != 0) {
    _type = T_constructor;

  } else if ((_ftype->_flags & CPPFunctionType::F_destructor) != 0) {
    _type = T_destructor;

  } else if ((_ftype->_flags & CPPFunctionType::F_operator_typecast) != 0) {
    _type = T_typecast_method;
  }

  if (_struct_type != (CPPStructType *)NULL &&
      ((function->_storage_class & CPPInstance::SC_static) == 0) &&
      _type != T_constructor) {

    // If this is a method, but not a static method, and not a
    // constructor, then we need to synthesize a "this" parameter.

    Parameter param;
    param._name = "this";
    param._has_name = true;
    bool is_const = (_ftype->_flags & CPPFunctionType::F_const_method) != 0;
    param._remap = new ParameterRemapThis(_struct_type, is_const);
    _parameters.push_back(param);
    _has_this = true;

    // Also check the name of the function.  If it's one of the
    // assignment-style operators, flag it as such.
    string fname = _function->get_simple_name();
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
      _type = T_assignment_method;
    }
  }

  const CPPParameterList::Parameters &params = 
    _ftype->_parameters->_parameters;
  for (int i = 0; i < (int)params.size() - num_default_parameters; i++) {
    CPPType *type = params[i]->_type->resolve_type(&parser, _scope);
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

    param._remap = make_remap(type);
    param._remap->set_default_value(params[i]->_initializer);

    if (!param._remap->is_valid()) {
      _is_valid = false;
    }

    _parameters.push_back(param);
  }

  if (_type == T_constructor) {
    // Constructors are a special case.  These appear to return void
    // as seen by the parser, but we know they actually return a new
    // concrete instance.

    if (_struct_type == (CPPStructType *)NULL) {
      nout << "Method " << *_function << " has no struct type\n";
      _is_valid = false;
    } else {
      _return_type = make_remap(_struct_type);
      _void_return = false;
    }

  } else if (_type == T_assignment_method) {
    // Assignment-type methods are also a special case.  We munge
    // these to return *this, which is a semi-standard C++ convention
    // anyway.  We just enforce it.

    if (_struct_type == (CPPStructType *)NULL) {
      nout << "Method " << *_function << " has no struct type\n";
      _is_valid = false;
    } else {
      CPPType *ref_type = CPPType::new_type(new CPPReferenceType(_struct_type));
      _return_type = make_remap(ref_type);
      _void_return = false;
    }

  } else {
    // The normal case.
    CPPType *rtype = _ftype->_return_type->resolve_type(&parser, _scope);
    _return_type = make_remap(rtype);
    _void_return = TypeManager::is_void(rtype);
  }

  if (!_return_type->is_valid()) {
    _is_valid = false;
  }

  // Do we need to manage the return value?
  _return_value_needs_management = _return_type->return_value_needs_management();
  _return_value_destructor = _return_type->get_return_value_destructor();

  // Should we manage a reference count?
  CPPType *return_type = _return_type->get_new_type();
  CPPType *return_meat_type = TypeManager::unwrap_pointer(return_type);

  if (manage_reference_counts && 
      TypeManager::is_reference_count_pointer(return_type) &&
      !TypeManager::has_protected_destructor(return_meat_type)) {
    // Yes!
    _manage_reference_count = true;
    _return_value_needs_management = true;

    // This is problematic, because we might not have the class in
    // question fully defined here, particularly if the class is
    // defined in some other library.
    _return_value_destructor = builder.get_destructor_for(return_meat_type);
  }

  return _is_valid;
}
 
////////////////////////////////////////////////////////////////////
//     Function: WrapperBuilder::is_valid
//       Access: Public
//  Description: Returns true if the function was correctly mapped, or
//               false if some parameter type is not supported and the
//               remapped function is thus invalid.
////////////////////////////////////////////////////////////////////
bool WrapperBuilder::
is_valid() const {
  return _is_valid;
}

////////////////////////////////////////////////////////////////////
//     Function: WrapperBuilder::return_value_needs_management
//       Access: Public
//  Description: Returns true if the return value represents a value
//               that was newly allocated, and hence must be
//               explicitly deallocated later by the caller.
////////////////////////////////////////////////////////////////////
bool WrapperBuilder::
return_value_needs_management() const {
  return _return_value_needs_management;
}

////////////////////////////////////////////////////////////////////
//     Function: WrapperBuilder::get_return_value_destructor
//       Access: Public
//  Description: If return_value_needs_management() returns true, this
//               should return the index of the function that should
//               be called when it is time to destruct the return
//               value.  It will generally be the same as the
//               destructor for the class we just returned a pointer
//               to.
////////////////////////////////////////////////////////////////////
FunctionIndex WrapperBuilder::
get_return_value_destructor() const {
  return _return_value_destructor;
}


////////////////////////////////////////////////////////////////////
//     Function: WrapperBuilder::make_remap
//       Access: Protected, Virtual
//  Description: Allocates a new ParameterRemap object suitable to the
//               indicated type.
////////////////////////////////////////////////////////////////////
ParameterRemap *WrapperBuilder::
make_remap(CPPType *orig_type) {
  if (convert_strings) {
    if (TypeManager::is_char_pointer(orig_type)) {
      return new ParameterRemapCharStarToString(orig_type);
    }
     
    // If we're exporting a method of basic_string<char> itself, don't
    // convert basic_string<char>'s to atomic strings.

    if (_struct_type == (CPPStructType *)NULL ||
        !TypeManager::is_basic_string_char(_struct_type)) {
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
      if (_struct_type == (CPPStructType *)NULL ||
          !(pt_type->get_local_name(&parser) == _struct_type->get_local_name(&parser))) {
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
manage_return_value(ostream &out, int indent_level,
                    const string &return_expr) const {
  if (_manage_reference_count) {
    // If we're managing reference counts, and we're about to return a
    // reference countable object, then increment its count.
    if (return_expr == "return_value") {
      // If the expression is just a variable name, we can just ref it
      // directly.
      output_ref(out, indent_level, return_expr);
      return return_expr;

    } else {
      // Otherwise, we should probably assign it to a temporary first,
      // so we don't invoke the function twice or something.
      CPPType *type = _return_type->get_temporary_type();
      indent(out, indent_level);
      type->output_instance(out, "refcount", &parser);
      out << " = " << return_expr << ";\n";

      indent(out, indent_level)
        << "if (" << return_expr << " != ("
        << _return_type->get_new_type()->get_local_name(&parser) << ")0) {\n";
      indent(out, indent_level + 2)
        << return_expr << "->ref();\n";
      indent(out, indent_level)
        << "}\n";
      output_ref(out, indent_level, "refcount");
      return _return_type->temporary_to_return("refcount");
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
output_ref(ostream &out, int indent_level, const string &varname) const {
  if (_type == T_constructor || _type == T_typecast) {
    // In either of these cases, we can safely assume the pointer will
    // never be NULL.
    indent(out, indent_level)
      << varname << "->ref();\n";

  } else {
    // However, in the general case, we have to check for that before
    // we attempt to ref it.

    indent(out, indent_level)
      << "if (" << varname << " != ("
      << _return_type->get_new_type()->get_local_name(&parser) << ")0) {\n";
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
  // Just for the fun of it, we'll name the "this" parameter something
  // different.
  if (_has_this) {
    if (n == 0) {
      return "container";
    }
    n--;
  }

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
get_call_str(const vector_string &pexprs) const {
  // Build up the call to the actual function.
  ostringstream call;

  int pn = 0;

  // Getters and setters are a special case.
  if (_type == T_getter) {
    if (_has_this) {
      call << "(" << get_parameter_expr(pn, pexprs) << ")->"
           << _expression;
    } else {
      call << _expression;
    }

  } else if (_type == T_setter) {
    if (_has_this) {
      call << "(" << get_parameter_expr(pn, pexprs) << ")->"
           << _expression;
      pn++;
    } else {
      call << _expression;
    }

    call << " = ";
    assert(pn + 1 == (int)_parameters.size());
    _parameters[pn]._remap->pass_parameter(call, get_parameter_expr(pn, pexprs));

  } else {
    if (_has_this) {
      // If we have a synthesized "this" parameter, the calling
      // convention is a bit different.
      call << "(" << get_parameter_expr(pn, pexprs) << ")->"
           << _function->get_local_name();
      pn++;
      
    } else if (_type == T_constructor) {
      // Constructors are called differently too.
      call << _struct_type->get_local_name(&parser);
      
    } else {
      call << _function->get_local_name(&parser);
    }
    
    call << "(";
    if (pn < (int)_parameters.size()) {
      _parameters[pn]._remap->pass_parameter(call, get_parameter_expr(pn, pexprs));
      pn++;
      while (pn < (int)_parameters.size()) {
        call << ", ";
        _parameters[pn]._remap->pass_parameter(call, get_parameter_expr(pn, pexprs));
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
call_function(ostream &out, int indent_level, bool convert_result,
              const vector_string &pexprs) const {
  string return_expr;

  if (_type == T_destructor) {
    // A destructor wrapper is just a wrapper around the delete operator.
    assert(_parameters.size() == 1);
    assert(_has_this);
    assert(_struct_type != (CPPStructType *)NULL);

    if (TypeManager::is_reference_count(_struct_type)) {
      // Except for a reference-count type object, in which case the
      // destructor is a wrapper around unref_delete().
      indent(out, indent_level)
        << "unref_delete(" << get_parameter_expr(0, pexprs) << ");\n";
    } else {
      indent(out, indent_level)
        << "delete " << get_parameter_expr(0, pexprs) << ";\n";
    }

  } else if (_type == T_typecast_method) {
    // A typecast method can be invoked implicitly.
    assert(_parameters.size() == 1);
    string cast_expr = 
      "(" + _return_type->get_orig_type()->get_local_name(&parser) + 
      ")(*" + get_parameter_expr(0, pexprs) + ")";

    if (!convert_result) {
      return_expr = cast_expr;
    } else {
      string new_str =
        _return_type->prepare_return_expr(out, indent_level, cast_expr);
      return_expr = _return_type->get_return_expr(new_str);
    }

  } else if (_type == T_typecast) {
    // A regular typecast converts from a pointer type to another
    // pointer type.  (This is different from the typecast method,
    // above, which converts from the concrete type to some other
    // type.)
    assert(_parameters.size() == 1);
    string cast_expr = 
      "(" + _return_type->get_orig_type()->get_local_name(&parser) + 
      ")" + get_parameter_expr(0, pexprs);

    if (!convert_result) {
      return_expr = cast_expr;
    } else {
      string new_str =
        _return_type->prepare_return_expr(out, indent_level, cast_expr);
      return_expr = _return_type->get_return_expr(new_str);
    }

  } else if (_type == T_constructor) {
    // A special case for constructors.
    return_expr = "new " + get_call_str(pexprs);

  } else if (_type == T_assignment_method) {
    // Another special case for assignment operators.
    indent(out, indent_level)
      << get_call_str(pexprs) << ";\n";

    string this_expr = get_parameter_expr(0, pexprs);
    string ref_expr = "*" + this_expr;

    if (!convert_result) {
      return_expr = ref_expr;
    } else {
      string new_str =
        _return_type->prepare_return_expr(out, indent_level, ref_expr);
      return_expr = _return_type->get_return_expr(new_str);

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
          
  } else if (_void_return) {
    indent(out, indent_level)
      << get_call_str(pexprs) << ";\n";

  } else {
    string call = get_call_str(pexprs);

    if (!convert_result) {
      return_expr = get_call_str(pexprs);
      
    } else {
      if (_return_type->return_value_should_be_simple()) {
        // We have to assign the result to a temporary first; this makes
        // it a bit easier on poor old VC++.
        indent(out, indent_level);
        _return_type->get_orig_type()->output_instance(out, "result",
                                                       &parser);
        out << " = " << call << ";\n";

        string new_str =
          _return_type->prepare_return_expr(out, indent_level, "result");
        return_expr = _return_type->get_return_expr(new_str);

      } else {
        // This should be simple enough that we can return it directly.
        string new_str = 
          _return_type->prepare_return_expr(out, indent_level, call);
        return_expr = _return_type->get_return_expr(new_str);
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
write_spam_message(ostream &out) const {
  if (generate_spam) {
    out << "#ifndef NDEBUG\n"
        << "  if (in_" << library_name << "_cat.is_spam()) {\n"
        << "    in_" << library_name << "_cat.spam()\n"
        << "      << \"";
    write_quoted_string(out, _description);
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
