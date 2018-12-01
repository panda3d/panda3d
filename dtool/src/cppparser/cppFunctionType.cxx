/**
 * PANDA 3D SOFTWARE
 * Copyright (c) Carnegie Mellon University.  All rights reserved.
 *
 * All use of this software is subject to the terms of the revised BSD
 * license.  You should have received a copy of this license along
 * with this source code in a file named "LICENSE."
 *
 * @file cppFunctionType.cxx
 * @author drose
 * @date 1999-10-21
 */

#include "cppFunctionType.h"
#include "cppParameterList.h"
#include "cppSimpleType.h"
#include "cppInstance.h"

using std::ostream;
using std::ostringstream;
using std::string;

/**
 *
 */
CPPFunctionType::
CPPFunctionType(CPPType *return_type, CPPParameterList *parameters,
                int flags) :
  CPPType(CPPFile()),
  _return_type(return_type),
  _parameters(parameters),
  _flags(flags)
{
  _class_owner = nullptr;

  // If the parameter list contains just the token "void", it means no
  // parameters.
  if (_parameters != nullptr &&
      _parameters->_parameters.size() == 1 &&
      _parameters->_parameters.front()->_type->as_simple_type() != nullptr &&
      _parameters->_parameters.front()->_type->as_simple_type()->_type ==
      CPPSimpleType::T_void &&
      _parameters->_parameters.front()->_ident == nullptr) {
    _parameters->_parameters.clear();
  }
}

/**
 *
 */
CPPFunctionType::
CPPFunctionType(const CPPFunctionType &copy) :
  CPPType(copy),
  _return_type(copy._return_type),
  _parameters(copy._parameters),
  _flags(copy._flags),
  _class_owner(copy._class_owner)
{
}

/**
 *
 */
void CPPFunctionType::
operator = (const CPPFunctionType &copy) {
  CPPType::operator = (copy);
  _return_type = copy._return_type;
  _parameters = copy._parameters;
  _flags = copy._flags;
  _class_owner = copy._class_owner;
}

/**
 * Returns true if the function accepts the given number of parameters.
 */
bool CPPFunctionType::
accepts_num_parameters(int num_parameters) {
  assert(num_parameters >= 0);
  if (_parameters == nullptr) {
    return (num_parameters == 0);
  }

  size_t actual_num_parameters = _parameters->_parameters.size();
  // If we passed too many parameters, it must have an ellipsis.
  if ((size_t)num_parameters > actual_num_parameters) {
    return _parameters->_includes_ellipsis;
  }

  // Make sure all superfluous parameters have a default value.
  for (size_t i = (size_t)num_parameters; i < actual_num_parameters; ++i) {
    CPPInstance *param = _parameters->_parameters[i];
    if (param->_initializer == nullptr) {
      return false;
    }
  }

  return true;
}

/**
 * Returns true if this declaration is an actual, factual declaration, or
 * false if some part of the declaration depends on a template parameter which
 * has not yet been instantiated.
 */
bool CPPFunctionType::
is_fully_specified() const {
  return CPPType::is_fully_specified() &&
    _return_type->is_fully_specified() &&
    _parameters->is_fully_specified();
}

/**
 *
 */
CPPDeclaration *CPPFunctionType::
substitute_decl(CPPDeclaration::SubstDecl &subst,
                CPPScope *current_scope, CPPScope *global_scope) {
  SubstDecl::const_iterator si = subst.find(this);
  if (si != subst.end()) {
    return (*si).second;
  }

  CPPFunctionType *rep = new CPPFunctionType(*this);
  if (_return_type != nullptr) {
    rep->_return_type =
      _return_type->substitute_decl(subst, current_scope, global_scope)
      ->as_type();
  }

  if (_parameters != nullptr) {
    rep->_parameters =
      _parameters->substitute_decl(subst, current_scope, global_scope);
  }

  if (rep->_return_type == _return_type &&
      rep->_parameters == _parameters) {
    delete rep;
    rep = this;
  }
  rep = CPPType::new_type(rep)->as_function_type();

  subst.insert(SubstDecl::value_type(this, rep));
  return rep;
}

/**
 * If this CPPType object is a forward reference or other nonspecified
 * reference to a type that might now be known a real type, returns the real
 * type.  Otherwise returns the type itself.
 */
CPPType *CPPFunctionType::
resolve_type(CPPScope *current_scope, CPPScope *global_scope) {
  CPPType *rtype = _return_type->resolve_type(current_scope, global_scope);
  CPPParameterList *params;
  if (_parameters == nullptr) {
    params = nullptr;
  } else {
    params = _parameters->resolve_type(current_scope, global_scope);
  }

  if (rtype != _return_type || params != _parameters) {
    CPPFunctionType *rep = new CPPFunctionType(*this);
    rep->_return_type = rtype;
    rep->_parameters = params;
    return CPPType::new_type(rep);
  }
  return this;
}

/**
 * Returns true if the type, or any nested type within the type, is a
 * CPPTBDType and thus isn't fully determined right now.  In this case,
 * calling resolve_type() may or may not resolve the type.
 */
bool CPPFunctionType::
is_tbd() const {
  if (_return_type->is_tbd()) {
    return true;
  }
  return _parameters == nullptr || _parameters->is_tbd();
}

/**
 * Returns true if the type is considered a Plain Old Data (POD) type.
 */
bool CPPFunctionType::
is_trivial() const {
  return false;
}

/**
 *
 */
void CPPFunctionType::
output(ostream &out, int indent_level, CPPScope *scope, bool complete) const {
  output(out, indent_level, scope, complete, -1);
}

/**
 * The additional parameter allows us to specify the number of parameters we
 * wish to show the default values for.  If num_default_parameters is >= 0, it
 * indicates the number of default parameter values to show on output.
 * Otherwise, all parameter values are shown.
 */
void CPPFunctionType::
output(ostream &out, int indent_level, CPPScope *scope, bool complete,
       int num_default_parameters) const {

  if (_flags & F_trailing_return_type) {
    // It was declared using trailing return type, so let's format it that
    // way.
    out << "auto(";
    _parameters->output(out, scope, true, num_default_parameters);
    out << ")";
    if (_flags & F_const_method) {
      out << " const";
    }
    if (_flags & F_noexcept) {
      out << " noexcept";
    }
    if (_flags & F_final) {
      out << " final";
    }
    if (_flags & F_override) {
      out << " override";
    }
    out << " -> ";
    _return_type->output(out, indent_level, scope, false);

  } else {
    _return_type->output(out, indent_level, scope, complete);
    out << "(";
    _parameters->output(out, scope, true, num_default_parameters);
    out << ")";
    if (_flags & F_const_method) {
      out << " const";
    }
    if (_flags & F_noexcept) {
      out << " noexcept";
    }
    if (_flags & F_final) {
      out << " final";
    }
    if (_flags & F_override) {
      out << " override";
    }
  }
}

/**
 * Formats a C++-looking line that defines an instance of the given type, with
 * the indicated name.  In most cases this will be "type name", but some types
 * have special exceptions.
 */
void CPPFunctionType::
output_instance(ostream &out, int indent_level, CPPScope *scope,
                bool complete, const string &prename,
                const string &name) const {
  output_instance(out, indent_level, scope, complete, prename, name, -1);
}

/**
 * The additional parameter allows us to specify the number of parameters we
 * wish to show the default values for.  If num_default_parameters is >= 0, it
 * indicates the number of default parameter values to show on output.
 * Otherwise, all parameter values are shown.
 */
void CPPFunctionType::
output_instance(ostream &out, int indent_level, CPPScope *scope,
                bool complete, const string &prename,
                const string &name, int num_default_parameters) const {
  ostringstream parm_string;
  parm_string << "(";
  _parameters->output(parm_string, scope, true, num_default_parameters);
  parm_string << ")";
  string str = parm_string.str();

  if (_flags & (F_constructor | F_destructor)) {
    // No return type for constructors and destructors.
    out << prename << name << str;

  } else if (_flags & F_trailing_return_type) {
    // It was declared using trailing return type, so let's format it that
    // way.
    out << "auto ";

    if (prename.empty()) {
      out << name;
    } else {
      out << "(" << prename << name << ")";
    }

    out << str;

  } else if (_flags & F_operator_typecast) {
    out << "operator ";
    _return_type->output_instance(out, indent_level, scope, complete, "", prename + str);

  } else {
    if (prename.empty()) {
      _return_type->output_instance(out, indent_level, scope, complete,
                                    "", prename + name + str);
    } else {
      _return_type->output_instance(out, indent_level, scope, complete,
                                    "", "(" + prename + name + ")" + str);
    }
  }

  if (_flags & F_const_method) {
    out << " const";
  }
  if (_flags & F_volatile_method) {
    out << " volatile";
  }
  if (_flags & F_noexcept) {
    out << " noexcept";
  }
  if (_flags & F_final) {
    out << " final";
  }
  if (_flags & F_override) {
    out << " override";
  }

  if (_flags & F_trailing_return_type) {
    out << " -> ";
    _return_type->output(out, indent_level, scope, false);
  }
}

/**
 * Returns the number of parameters in the list that may take default values.
 */
int CPPFunctionType::
get_num_default_parameters() const {
  // The trick is just to count, beginning from the end and working towards
  // the front, the number of parameters that have some initializer.

  if (_parameters == nullptr) {
    return 0;
  }

  const CPPParameterList::Parameters &params = _parameters->_parameters;
  CPPParameterList::Parameters::const_reverse_iterator pi;
  int count = 0;
  for (pi = params.rbegin();
       pi != params.rend() && (*pi)->_initializer != nullptr;
       ++pi) {
    count++;
  }

  return count;
}

/**
 *
 */
CPPDeclaration::SubType CPPFunctionType::
get_subtype() const {
  return ST_function;
}

/**
 *
 */
CPPFunctionType *CPPFunctionType::
as_function_type() {
  return this;
}

/**
 * This is similar to is_equal(), except it is more forgiving: it considers
 * the functions to be equivalent only if the return type and the types of all
 * parameters match.
 *
 * Note that this isn't symmetric to account for covariant return types.
 */
bool CPPFunctionType::
match_virtual_override(const CPPFunctionType &other) const {
  if (!_return_type->is_equivalent(*other._return_type) &&
      !_return_type->is_convertible_to(other._return_type)) {
    return false;
  }

  if (((_flags ^ other._flags) & ~(F_override | F_final)) != 0) {
    return false;
  }

  if (!_parameters->is_equivalent(*other._parameters)) {
    return false;
  }

  return true;
}

/**
 * Called by CPPDeclaration() to determine whether this type is equivalent to
 * another type of the same type.
 */
bool CPPFunctionType::
is_equal(const CPPDeclaration *other) const {
  const CPPFunctionType *ot = ((CPPDeclaration *)other)->as_function_type();
  assert(ot != nullptr);

  if (_return_type != ot->_return_type) {
    return false;
  }
  if (_flags != ot->_flags) {
    return false;
  }
  if (_parameters == ot->_parameters) {
    return true;
  }
  if (_parameters == nullptr || ot->_parameters == nullptr ||
      *_parameters != *ot->_parameters) {
    return false;
  }
  return true;
}


/**
 * Called by CPPDeclaration() to determine whether this type should be ordered
 * before another type of the same type, in an arbitrary but fixed ordering.
 */
bool CPPFunctionType::
is_less(const CPPDeclaration *other) const {
  const CPPFunctionType *ot = ((CPPDeclaration *)other)->as_function_type();
  assert(ot != nullptr);

  if (_return_type != ot->_return_type) {
    return _return_type < ot->_return_type;
  }
  if (_flags != ot->_flags) {
    return _flags < ot->_flags;
  }
  if (_parameters == ot->_parameters) {
    return 0;
  }
  if (_parameters == nullptr || ot->_parameters == nullptr) {
    return _parameters < ot->_parameters;
  }
  return *_parameters < *ot->_parameters;
}
