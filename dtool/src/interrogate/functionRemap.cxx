/**
 * PANDA 3D SOFTWARE
 * Copyright (c) Carnegie Mellon University.  All rights reserved.
 *
 * All use of this software is subject to the terms of the revised BSD
 * license.  You should have received a copy of this license along
 * with this source code in a file named "LICENSE."
 *
 * @file functionRemap.cxx
 * @author drose
 * @date 2001-09-19
 */

#include "functionRemap.h"
#include "typeManager.h"
#include "interrogate.h"
#include "parameterRemap.h"
#include "parameterRemapThis.h"
#include "parameterRemapHandleToInt.h"
#include "parameterRemapUnchanged.h"
#include "interfaceMaker.h"
#include "interrogateBuilder.h"

#include "interrogateDatabase.h"
#include "cppExpression.h"
#include "cppInstance.h"
#include "cppArrayType.h"
#include "cppConstType.h"
#include "cppFunctionType.h"
#include "cppParameterList.h"
#include "cppReferenceType.h"
#include "interrogateType.h"
#include "pnotify.h"

using std::ostream;
using std::ostringstream;
using std::string;

/**
 *
 */
FunctionRemap::
FunctionRemap(const InterrogateType &itype, const InterrogateFunction &ifunc,
              CPPInstance *cppfunc, int num_default_parameters,
              InterfaceMaker *interface_maker) {
  _return_type = nullptr;
  _void_return = true;
  _ForcedVoidReturn = false;
  _has_this = false;
  _blocking = false;
  _extension = false;
  _const_method = false;
  _first_true_parameter = 0;
  _num_default_parameters = num_default_parameters;
  _type = T_normal;
  _flags = 0;
  _args_type = 0;
  _wrapper_index = 0;

  _return_value_needs_management = false;
  _return_value_destructor = 0;
  _manage_reference_count = false;

  _cppfunc = cppfunc;
  _ftype = _cppfunc->_type->as_function_type();
  _cpptype = itype._cpptype;
  _cppscope = itype._cppscope;

  _is_valid = setup_properties(ifunc, interface_maker);
}

/**
 *
 */
FunctionRemap::
~FunctionRemap() {
}

/**
 * Returns a string that will be a suitable name for the nth parameter in the
 * generated code.  This may not correspond to the name of the parameter in
 * the original code.
 */
string FunctionRemap::
get_parameter_name(int n) const {
  ostringstream str;
  str << "param" << n;
  return str.str();
}

/**
 * Writes a sequence of commands to the given output stream to call the
 * wrapped function.  The parameter values are assumed to be simply the names
 * of the parameters.
 *
 * The return value is the expression to return, if we are returning a value,
 * or the empty string if we return nothing.
 */
string FunctionRemap::
call_function(ostream &out, int indent_level, bool convert_result,
              const string &container) const {
  vector_string pexprs;
  for (size_t i = 0; i < _parameters.size(); ++i) {
    pexprs.push_back(get_parameter_name(i));
  }
  return call_function(out, indent_level, convert_result, container, pexprs);
}

/**
 * Writes a sequence of commands to the given output stream to call the
 * wrapped function.  The parameter values are taken from pexprs.
 *
 * The return value is the expression to return, if we are returning a value,
 * or the empty string if we return nothing.
 */
string FunctionRemap::
call_function(ostream &out, int indent_level, bool convert_result,
              const string &container, const vector_string &pexprs) const {
  string return_expr;

  if (_type == T_destructor) {
    // A destructor wrapper is just a wrapper around the delete operator.
    assert(!container.empty());
    assert(_cpptype != nullptr);

    if (TypeManager::is_reference_count(_cpptype)) {
      // Except for a reference-count type object, in which case the
      // destructor is a wrapper around unref_delete().
      InterfaceMaker::indent(out, indent_level)
        << "unref_delete(" << container << ");\n";
    } else {
      InterfaceMaker::indent(out, indent_level) << "delete " << container << ";\n";
    }

  } else if (_type == T_typecast_method) {
    // A typecast method can be invoked implicitly.
    ostringstream cast_expr;
    cast_expr << "("
      << _return_type->get_orig_type()->get_local_name(&parser) << ")";

    _parameters[0]._remap->pass_parameter(cast_expr, container);

    if (!convert_result) {
      return_expr = cast_expr.str();
    } else {
      string new_str =
        _return_type->prepare_return_expr(out, indent_level, cast_expr.str());
      return_expr = _return_type->get_return_expr(new_str);
    }

  } else if (_type == T_typecast) {
    // A regular typecast converts from a pointer type to another pointer
    // type.  (This is different from the typecast method, above, which
    // converts from the concrete type to some other type.)
    assert(!container.empty());
    string cast_expr =
      "(" + _return_type->get_orig_type()->get_local_name(&parser) +
      ")" + container;

    if (!convert_result) {
      return_expr = cast_expr;
    } else {
      string new_str =
        _return_type->prepare_return_expr(out, indent_level, cast_expr);
      return_expr = _return_type->get_return_expr(new_str);
    }

  } else if (_type == T_constructor) {
    // A special case for constructors.
    if (_extension) {
      // Extension constructors are a special case.  We assume there is a
      // default constructor for the class, and the actual construction is
      // done by an __init__ method.
      InterfaceMaker::indent(out, indent_level);
      _return_type->get_new_type()->output_instance(out, "result", &parser);
      out << " = new " << _cpptype->get_local_name(&parser) << ";\n";

      InterfaceMaker::indent(out, indent_level)
        << get_call_str("result", pexprs) << ";\n";

      return_expr = "result";

    } else {
      string defconstruct = builder.in_defconstruct(_cpptype->get_local_name(&parser));
      string call_expr;

      if (pexprs.empty() && !defconstruct.empty()) {
        call_expr = defconstruct;
      } else {
        call_expr = get_call_str(container, pexprs);
      }

      if (!_return_type->return_value_needs_management()) {
        return_expr = _return_type->get_return_expr(call_expr);
      } else {
        return_expr = "new " + call_expr;
      }
    }
    if (_void_return) {
      nout << "Error, constructor for " << *_cpptype << " returning void.\n";
      return_expr = "";
    }

  } else if (_type == T_assignment_method) {
    // Another special case for assignment operators.
    assert(!container.empty());
    InterfaceMaker::indent(out, indent_level)
      << get_call_str(container, pexprs) << ";\n";

    string this_expr = container;
    string ref_expr = "*" + this_expr;

    if (!convert_result) {
      return_expr = ref_expr;
    } else {
      string new_str =
        _return_type->prepare_return_expr(out, indent_level, ref_expr);
      return_expr = _return_type->get_return_expr(new_str);

      // Now a simple special-case test.  Often, we will have converted the
      // reference-returning assignment operator to a pointer.  In this case,
      // we might inadvertently generate code like "return &(*this)", when
      // "return this" would do.  We check for this here and undo it as a
      // special case.

      // There's no real good reason to do this, other than that it feels more
      // satisfying to a casual perusal of the generated code.  It *is*
      // conceivable that some broken compilers wouldn't like "&(*this)",
      // though.

      if (return_expr == "&(" + ref_expr + ")" ||
          return_expr == "&" + ref_expr) {
        return_expr = this_expr;
      }
    }

  } else if (_void_return) {
    InterfaceMaker::indent(out, indent_level)
      << get_call_str(container, pexprs) << ";\n";

  } else {
    string call = get_call_str(container, pexprs);

    if (!convert_result) {
      return_expr = call;

    } else {
      // if (_return_type->return_value_should_be_simple()) {
      if (false) {
        // We have to assign the result to a temporary first; this makes it a
        // bit easier on poor old VC++.
        InterfaceMaker::indent(out, indent_level);
        _return_type->get_orig_type()->output_instance(out, "result",
                                                           &parser);
        out << " = " << call << ";\n";

        // Use of the C++11 std::move function basically turns an lvalue into
        // an rvalue, allowing a move constructor to be called instead of a
        // copy constructor (since we won't be using the return value any
        // more), which is usually more efficient if it exists.  If it
        // doesn't, it shouldn't do any harm.
        string new_str =
          _return_type->prepare_return_expr(out, indent_level, "std::move(result)");
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

/**
 * Writes a line describing the original C++ method or function.  This is
 * generally useful only within a comment.
 */
void FunctionRemap::
write_orig_prototype(ostream &out, int indent_level, bool local, int num_default_args) const {
  if (local) {
    _cppfunc->output(out, indent_level, nullptr, false, num_default_args);
  } else {
    _cppfunc->output(out, indent_level, &parser, false, num_default_args);
  }
}

/**
 * Creates an InterrogateFunctionWrapper object corresponding to this callable
 * instance and stores it in the database.
 */
FunctionWrapperIndex FunctionRemap::
make_wrapper_entry(FunctionIndex function_index) {
  _wrapper_index =
    InterrogateDatabase::get_ptr()->get_next_index();

  InterrogateFunctionWrapper iwrapper;
  iwrapper._function = function_index;
  iwrapper._name = _wrapper_name;
  iwrapper._unique_name = _unique_name;

  if (_cppfunc->_leading_comment != nullptr) {
    iwrapper._comment = InterrogateBuilder::trim_blanks(_cppfunc->_leading_comment->_comment);
  }

  if (output_function_names) {
    // If we're keeping the function names, record that the wrapper is
    // callable.
    iwrapper._flags |= InterrogateFunctionWrapper::F_callable_by_name;
  }

  Parameters::const_iterator pi;
  for (pi = _parameters.begin();
       pi != _parameters.end();
       ++pi) {
    InterrogateFunctionWrapper::Parameter param;
    param._parameter_flags = 0;
    if ((*pi)._remap->new_type_is_atomic_string()) {
      param._type = builder.get_atomic_string_type();
    } else {
      param._type = builder.get_type((*pi)._remap->get_new_type(), false);
    }
    param._name = (*pi)._name;
    if ((*pi)._has_name) {
      param._parameter_flags |= InterrogateFunctionWrapper::PF_has_name;
    }
    iwrapper._parameters.push_back(param);
  }

  if (_has_this) {
    // If one of the parameters is "this", it must be the first one.
    assert(!iwrapper._parameters.empty());
    iwrapper._parameters.front()._parameter_flags |=
      InterrogateFunctionWrapper::PF_is_this;
  }

  if (!_void_return) {
    iwrapper._flags |= InterrogateFunctionWrapper::F_has_return;
  }

  if (_return_type->new_type_is_atomic_string()) {
    iwrapper._return_type = builder.get_atomic_string_type();
  } else {
    iwrapper._return_type =
      builder.get_type(_return_type->get_new_type(), false);
  }

  if (_return_value_needs_management) {
    iwrapper._flags |= InterrogateFunctionWrapper::F_caller_manages;
    FunctionIndex destructor = _return_value_destructor;

    if (destructor != 0) {
      iwrapper._return_value_destructor = destructor;

    } else {
      // We don't need to report this warning, since the FFI code understands
      // that if the destructor function is zero, it should use the regular
      // class destructor.

      // nout << "Warning!  Destructor for " << *_return_type->get_orig_type()
      // << " is unavailable.\n" << "  Cannot manage return value for:\n  " <<
      // description << "\n";
    }
  }

  InterrogateDatabase::get_ptr()->add_wrapper(_wrapper_index, iwrapper);
  return _wrapper_index;
}

/**
 * Returns a string suitable for calling the wrapped function.  If pexprs is
 * nonempty, it represents the list of expressions that will evaluate to each
 * parameter value.
 */
string FunctionRemap::
get_call_str(const string &container, const vector_string &pexprs) const {
  // Build up the call to the actual function.
  ostringstream call;

  // Getters and setters are a special case.
  if (_type == T_getter) {
    if (_has_this && !container.empty()) {
      call << "(" << container << ")->" << _expression;
    } else {
      call << _expression;
    }

  } else if (_type == T_setter) {
    string expr;
    if (_has_this && !container.empty()) {
      expr = "(" + container + ")->" + _expression;
    } else {
      expr = _expression;
    }

    // It's not possible to assign arrays in C++, we have to copy them.
    bool paren_close = false;
    CPPType *param_type = _parameters[_first_true_parameter]._remap->get_orig_type();
    CPPArrayType *array_type = param_type->as_array_type();
    if (array_type != nullptr) {
      call << "std::copy(" << expr << ", " << expr << " + " << *array_type->_bounds << ", ";
      paren_close = true;
    }
    else if (TypeManager::is_pointer_to_PyObject(param_type)) {
      call << "Dtool_Assign_PyObject(" << expr << ", ";
      paren_close = true;
    }
    else {
      call << expr << " = ";
    }

    _parameters[_first_true_parameter]._remap->pass_parameter(call,
                    get_parameter_expr(_first_true_parameter, pexprs));

    if (paren_close) {
      call << ')';
    }

  } else {
    const char *separator = "";

    // If this function is marked as having an extension function, call that
    // instead.
    if (_extension) {
      if (!container.empty()) {
        call << "invoke_extension(" << container << ").";
      } else {
        call << "Extension<" << _cpptype->get_local_name(&parser) << ">::";
      }

      if (_type == T_constructor) {
        // Constructor extensions are named __init__, by convention.
        call << "__init__";
      } else {
        call << _cppfunc->get_local_name();
      }
    } else {
      if (_type == T_constructor) {
        // Constructors are called differently.
        call << _cpptype->get_local_name(&parser);

      } else if (_has_this && !container.empty()) {
        // If we have a "this" parameter, the calling convention is also a bit
        // different.
        call << "((";
        _parameters[0]._remap->pass_parameter(call, container);
        call << ")." << _cppfunc->get_local_name() << ")";

      } else {
        call << "(";
        if (_cpptype != nullptr) {
          call << _cpptype->get_local_name(&parser);
        }
        call << "::" << _cppfunc->get_local_name() << ")";
      }
    }
    call << "(";

    if (_flags & F_explicit_self) {
      // Pass on the PyObject * that we stripped off above.
      call << separator << "self";
      separator = ", ";
    }

    size_t pn = _first_true_parameter;
    size_t num_parameters = pexprs.size();

    if (_type == T_item_assignment_operator) {
      // The last parameter is the value to set.
      --num_parameters;
    }

    for (pn = _first_true_parameter;
         pn < num_parameters; ++pn) {
      nassertd(pn < _parameters.size()) break;
      call << separator;
      _parameters[pn]._remap->pass_parameter(call, get_parameter_expr(pn, pexprs));
      separator = ", ";
    }
    call << ")";

    if (_type == T_item_assignment_operator) {
      call << " = ";
      _parameters[pn]._remap->pass_parameter(call, get_parameter_expr(pn, pexprs));
    }
  }

  return call.str();
}

/**
 * Returns the minimum number of arguments that needs to be passed to this
 * function.
 */
int FunctionRemap::
get_min_num_args() const {
  int min_num_args = 0;
  Parameters::const_iterator pi;
  pi = _parameters.begin();
  if (_has_this && pi != _parameters.end()) {
    ++pi;
  }
  for (; pi != _parameters.end(); ++pi) {
    ParameterRemap *param = (*pi)._remap;
    if (param->get_default_value() != nullptr) {
      // We've reached the first parameter that takes a default value.
      break;
    } else {
      ++min_num_args;
    }
  }
  return min_num_args;
}

/**
 * Returns the maximum number of arguments that can be passed to this
 * function.
 */
int FunctionRemap::
get_max_num_args() const {
  int max_num_args = _parameters.size();
  if (_has_this && _type != FunctionRemap::T_constructor) {
    --max_num_args;
  }
  return max_num_args;
}

/**
 * Returns a string that represents the expression associated with the nth
 * parameter.  This is just the nth element of pexprs if it is nonempty, or
 * the name of the nth parameter is it is empty.
 */
string FunctionRemap::
get_parameter_expr(size_t n, const vector_string &pexprs) const {
  if (n < pexprs.size()) {
    return pexprs[n];
  }
  return get_parameter_name(n);
}

/**
 * Sets up the properties of the function appropriately.  Returns true if
 * successful, or false if there is something unacceptable about the function.
 */
bool FunctionRemap::
setup_properties(const InterrogateFunction &ifunc, InterfaceMaker *interface_maker) {
  _function_signature =
    TypeManager::get_function_signature(_cppfunc, _num_default_parameters);
  _expression = ifunc._expression;

  if ((_ftype->_flags & CPPFunctionType::F_constructor) != 0) {
    _type = T_constructor;

  } else if ((_ftype->_flags & CPPFunctionType::F_destructor) != 0) {
    _type = T_destructor;

  } else if ((_ftype->_flags & CPPFunctionType::F_operator_typecast) != 0) {
    _type = T_typecast_method;

  } else if ((ifunc._flags & InterrogateFunction::F_typecast) != 0) {
    _type = T_typecast;

  } else if ((ifunc._flags & InterrogateFunction::F_getter) != 0) {
    _type = T_getter;

  } else if ((ifunc._flags & InterrogateFunction::F_setter) != 0) {
    _type = T_setter;
  }

  if ((_cppfunc->_storage_class & CPPInstance::SC_blocking) != 0) {
    // If it's marked as a "blocking" method or function, record that.
    _blocking = true;
  }
  if ((_cppfunc->_storage_class & CPPInstance::SC_extension) != 0) {
    // Same with functions or methods marked with "extension".
    _extension = true;
  }

  string fname = _cppfunc->get_simple_name();
  CPPType *rtype = _ftype->_return_type->resolve_type(&parser, _cppscope);

  if (_cpptype != nullptr &&
      ((_cppfunc->_storage_class & CPPInstance::SC_static) == 0) &&
      _type != T_constructor) {

    // If this is a method, but not a static method, and not a constructor,
    // then we need a "this" parameter.
    _has_this = true;
    _const_method = (_ftype->_flags & CPPFunctionType::F_const_method) != 0;

    if (interface_maker->synthesize_this_parameter()) {
      // If the interface_maker demands it, the "this" parameter is treated as
      // any other parameter, and inserted at the beginning of the parameter
      // list.
      Parameter param;
      param._name = "this";
      param._has_name = true;
      if (_const_method) {
        CPPType *const_type = CPPType::new_type(new CPPConstType(_cpptype));
        param._remap = interface_maker->remap_parameter(_cpptype, const_type);
      } else {
        param._remap = interface_maker->remap_parameter(_cpptype, _cpptype);
      }
      // param._remap = new ParameterRemapThis(_cpptype, _const_method);
      _parameters.push_back(param);
      _first_true_parameter = 1;
    }

    // Also check the name of the function.  If it's one of the assignment-
    // style operators, flag it as such.
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

    } else if (fname == "operator []" && !_const_method && rtype != nullptr) {
       // Check if this is an item-assignment operator.
      CPPReferenceType *reftype = rtype->as_reference_type();
      if (reftype != nullptr && reftype->_pointing_at->as_const_type() == nullptr) {
        // It returns a mutable reference.
        _type = T_item_assignment_operator;
      }
    }
  }

  const CPPParameterList::Parameters &params =
    _ftype->_parameters->_parameters;
  for (int i = 0; i < (int)params.size() - _num_default_parameters; i++) {
    // CPPType *type = params[i]->_type->resolve_type(&parser, _cppscope);
    CPPType *type = params[i]->_type;
    Parameter param;
    param._has_name = true;
    param._name = params[i]->get_simple_name();

    if (param._name.empty()) {
      // If the parameter has no name, record it as being nameless, but also
      // synthesize one in case someone asks anyway.
      param._has_name = false;
      ostringstream param_name;
      param_name << "param" << i;
      param._name = param_name.str();
    }

    param._remap = interface_maker->remap_parameter(_cpptype, type);
    if (param._remap == nullptr) {
      // If we can't handle one of the parameter types, we can't call the
      // function.
      if (fname == "__traverse__") {
        // Hack to record this even though we can't wrap visitproc.
        param._remap = new ParameterRemapUnchanged(type);
      } else {
        // nout << "Can't handle parameter " << i << " of method " <<
        // *_cppfunc << "\n";
        return false;
      }
    } else {
      param._remap->set_default_value(params[i]->_initializer);
    }

    if (!param._remap->is_valid()) {
      nout << "Invalid remap for parameter " << i << " of method " << *_cppfunc << "\n";
      return false;
    }

    _parameters.push_back(param);
  }

  if (_type == T_constructor) {
    // Constructors are a special case.  These appear to return void as seen
    // by the parser, but we know they actually return a new concrete
    // instance.

    if (_cpptype == nullptr) {
      nout << "Method " << *_cppfunc << " has no struct type\n";
      return false;
    }

    _return_type = interface_maker->remap_parameter(_cpptype, _cpptype);
    if (_return_type != nullptr) {
      _void_return = false;
    }

  } else if (_type == T_assignment_method) {
    // Assignment-type methods are also a special case.  We munge these to
    // return *this, which is a semi-standard C++ convention anyway.  We just
    // enforce it.

    if (_cpptype == nullptr) {
      nout << "Method " << *_cppfunc << " has no struct type\n";
      return false;
    } else {
      CPPType *ref_type = CPPType::new_type(new CPPReferenceType(_cpptype));
      _return_type = interface_maker->remap_parameter(_cpptype, ref_type);
      if (_return_type != nullptr) {
        _void_return = false;
      }
    }

  } else if (_type == T_item_assignment_operator) {
    // An item-assignment method isn't really a thing in C++, but it is in
    // scripting languages, so we use this to denote item-access operators
    // that return a non-const reference.

    if (_cpptype == nullptr) {
      nout << "Method " << *_cppfunc << " has no struct type\n";
      return false;
    } else {
      // Synthesize a const reference parameter for the assignment.
      CPPType *bare_type = TypeManager::unwrap_reference(rtype);
      CPPType *const_type = CPPType::new_type(new CPPConstType(bare_type));
      CPPType *ref_type = CPPType::new_type(new CPPReferenceType(const_type));

      Parameter param;
      param._has_name = true;
      param._name = "assign_val";
      param._remap = interface_maker->remap_parameter(_cpptype, ref_type);

      if (param._remap == nullptr || !param._remap->is_valid()) {
        nout << "Invalid remap for assignment type of method " << *_cppfunc << "\n";
        return false;
      }
      _parameters.push_back(param);

      // Pretend we don't return anything at all.
      CPPType *void_type = TypeManager::get_void_type();
      _return_type = interface_maker->remap_parameter(_cpptype, void_type);
      _void_return = true;
    }

  } else {
    // The normal case.
    _return_type = interface_maker->remap_parameter(_cpptype, rtype);
    if (_return_type != nullptr) {
      _void_return = TypeManager::is_void(rtype);
    }
  }

  if (_return_type == nullptr ||
      !_return_type->is_valid()) {
    // If our return type isn't something we can deal with, treat the function
    // as if it returns NULL.
    _void_return = true;
    _ForcedVoidReturn = true;
    CPPType *void_type = TypeManager::get_void_type();
    _return_type = interface_maker->remap_parameter(_cpptype, void_type);
    assert(_return_type != nullptr);
  }

  // Do we need to manage the return value?
  _return_value_needs_management =
    _return_type->return_value_needs_management();
  _return_value_destructor =
    _return_type->get_return_value_destructor();

  // Should we manage a reference count?
  CPPType *return_type = _return_type->get_new_type();
  return_type = TypeManager::resolve_type(return_type, _cppscope);
  CPPType *return_meat_type = TypeManager::unwrap_pointer(return_type);

  if (manage_reference_counts &&
      TypeManager::is_reference_count_pointer(return_type) &&
      !TypeManager::has_protected_destructor(return_meat_type)) {
    // Yes!
    _manage_reference_count = true;
    _return_value_needs_management = true;

    // This is problematic, because we might not have the class in question
    // fully defined here, particularly if the class is defined in some other
    // library.
    _return_value_destructor = builder.get_destructor_for(return_meat_type);
  }

  if (_type == T_getter && TypeManager::is_pointer_to_PyObject(return_type)) {
    _manage_reference_count = true;
    _return_value_needs_management = true;
  }

  // Check for a special meaning by name and signature.
  size_t first_param = 0;
  if (_has_this) {
    first_param = 1;
  }

  if (_parameters.size() > first_param && _parameters[first_param]._name == "self" &&
      TypeManager::is_pointer_to_PyObject(_parameters[first_param]._remap->get_orig_type())) {
    // Here's a special case.  If the first parameter of a nonstatic method
    // is a PyObject * called "self", then we will automatically fill it in
    // from the this pointer, and remove it from the generated parameter
    // list.
    _parameters.erase(_parameters.begin() + first_param);
    _flags |= F_explicit_self;
  }

  if (_parameters.size() == first_param) {
    _args_type = InterfaceMaker::AT_no_args;
  } else if (_parameters.size() == first_param + 1 &&
             _parameters[first_param]._remap->get_default_value() == nullptr) {
    _args_type = InterfaceMaker::AT_single_arg;
  } else {
    _args_type = InterfaceMaker::AT_varargs;

    // If the arguments are named "args" and "kwargs", we will be directly
    // passing the argument tuples to the function.
    if (_parameters.size() == first_param + 2 &&
        _parameters[first_param]._name == "args" &&
        (_parameters[first_param + 1]._name == "kwargs" ||
          _parameters[first_param + 1]._name == "kwds")) {
      _flags |= F_explicit_args;
      _args_type = InterfaceMaker::AT_keyword_args;
    }
  }

  switch (_type) {
  case T_normal:
    if (fname == "operator []" || fname == "__getitem__") {
      _flags |= F_getitem;
      if (_has_this && _parameters.size() == 2) {
        if (TypeManager::is_integer(_parameters[1]._remap->get_new_type())) {
          // It receives a single int parameter.
          _flags |= F_getitem_int;
        }
      }

    } else if (fname == "__setitem__") {
      if (_has_this && _parameters.size() > 2) {
        _flags |= F_setitem;
        if (TypeManager::is_integer(_parameters[1]._remap->get_new_type())) {
          // Its first parameter is an int parameter, presumably an index.
          _flags |= F_setitem_int;
          _args_type = InterfaceMaker::AT_varargs;
        }
      }

    } else if (fname == "__delitem__") {
      if (_has_this && _parameters.size() == 2) {
        _flags |= F_delitem;
        if (TypeManager::is_integer(_parameters[1]._remap->get_new_type())) {
          // Its first parameter is an int parameter, presumably an index.
          _flags |= F_delitem_int;
          _args_type = InterfaceMaker::AT_single_arg;
        }
      }

    } else if (fname == "size" || fname == "__len__") {
      if (_parameters.size() == first_param &&
          TypeManager::is_integer(_return_type->get_new_type())) {
        // It receives no parameters, and returns an integer.
        _flags |= F_size;
      }

    } else if (fname == "make_copy") {
      if (_has_this && _parameters.size() == 1 &&
          TypeManager::is_pointer(_return_type->get_new_type())) {
        // It receives no parameters, and returns a pointer.
        _flags |= F_make_copy;
      }

    } else if (fname == "__iter__") {
      if (_parameters.size() == first_param &&
          TypeManager::is_pointer(_return_type->get_new_type())) {
        // It receives no parameters, and returns a pointer.
        _flags |= F_iter;
      }

    } else if (fname == "compare_to") {
      if (_has_this && _parameters.size() == 2 &&
          TypeManager::is_integer(_return_type->get_new_type())) {
        // It receives one parameter, and returns an integer.
        _flags |= F_compare_to;
      }

    } else if (fname == "make") {
      if (!_has_this && _parameters.size() >= 1 &&
          TypeManager::is_pointer(_return_type->get_new_type())) {
        // We can use this for coercion.
        _flags |= F_coerce_constructor;
      }

      if (_args_type == InterfaceMaker::AT_varargs) {
        // Of course methods named "make" can still take kwargs, if they are
        // named.
        for (size_t i = first_param; i < _parameters.size(); ++i) {
          if (_parameters[i]._has_name) {
            _args_type = InterfaceMaker::AT_keyword_args;
            break;
          }
        }
      }

    } else if (fname == "operator /") {
      if (_has_this && _parameters.size() == 2 &&
          TypeManager::is_float(_parameters[1]._remap->get_new_type())) {
        // This division operator takes a single float argument.
        _flags |= F_divide_float;
      }

    } else if (fname == "get_key" || fname == "get_hash") {
      if (_has_this && _parameters.size() == 1 &&
          TypeManager::is_integer(_return_type->get_new_type())) {
        _flags |= F_hash;
      }

    } else if (fname == "operator ()" || fname == "__call__") {
      // Call operators always take keyword arguments.
      _args_type = InterfaceMaker::AT_keyword_args;

    } else if (fname == "__setattr__"
            || fname == "__getattr__"
            || fname == "__delattr__") {
      // Just to prevent these from getting keyword arguments.

    } else {
      if (_args_type == InterfaceMaker::AT_varargs) {
        // Every other method can take keyword arguments, if they take more
        // than one argument, and the arguments are named.
        for (size_t i = first_param; i < _parameters.size(); ++i) {
          if (_parameters[i]._has_name) {
            _args_type |= InterfaceMaker::AT_keyword_args;
            break;
          }
        }
      } else if (_args_type == InterfaceMaker::AT_single_arg) {
        // If it takes an argument named "args", we are directly passing the
        // "args" tuple to the function.
        if (_parameters[first_param]._name == "args") {
          _flags |= F_explicit_args;
          _args_type = InterfaceMaker::AT_varargs;
        }
      }
    }
    break;

  case T_assignment_method:
    if (fname == "operator /=") {
      if (_has_this && _parameters.size() == 2 &&
          TypeManager::is_float(_parameters[1]._remap->get_new_type())) {
        // This division operator takes a single float argument.
        _flags |= F_divide_float;
      }
    }
    break;

  case T_item_assignment_operator:
    // The concept of "item assignment operator" doesn't really exist in C++,
    // but it does in scripting languages, and this allows us to wrap cases
    // where the C++ getitem returns an assignable reference.
    _flags |= F_setitem;
    if (_has_this && _parameters.size() > 2) {
      if (TypeManager::is_integer(_parameters[1]._remap->get_new_type())) {
        // Its first parameter is an int parameter, presumably an index.
        _flags |= F_setitem_int;
      }
    }
    _args_type = InterfaceMaker::AT_varargs;
    break;

  case T_constructor:
    if (_ftype->_flags & CPPFunctionType::F_copy_constructor) {
      // It's a copy constructor.
      _flags |= F_copy_constructor;

    } else if (_ftype->_flags & CPPFunctionType::F_move_constructor) {

    } else if (!_has_this && _parameters.size() > 0 &&
               (_cppfunc->_storage_class & CPPInstance::SC_explicit) == 0) {
      // A non-explicit non-copy constructor might be eligible for coercion,
      // as long as it does not require explicit keyword args.
      if ((_flags & F_explicit_args) == 0 ||
          _args_type != InterfaceMaker::AT_keyword_args) {

        _flags |= F_coerce_constructor;
      }
    }

    // Constructors always take varargs, and possibly keyword args.
    _args_type = InterfaceMaker::AT_varargs;
    for (size_t i = first_param; i < _parameters.size(); ++i) {
      if (_parameters[i]._has_name) {
        _args_type = InterfaceMaker::AT_keyword_args;
        break;
      }
    }
    break;

  default:
    break;
  }

  return true;
}

std::string make_safe_name(const std::string &name) {
  return InterrogateBuilder::clean_identifier(name);
  /*
  static const char safe_chars2[] = "abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ0123456789_";
  std::string result = name;

  size_t pos = result.find_first_not_of(safe_chars2);
  while (pos != std::string::npos) {
    result[pos] = '_';
    pos = result.find_first_not_of(safe_chars2);
  }

  return result;
  */
}
