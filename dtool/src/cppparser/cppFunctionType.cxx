// Filename: cppFunctionType.cxx
// Created by:  drose (21Oct99)
//
////////////////////////////////////////////////////////////////////
//
// PANDA 3D SOFTWARE
// Copyright (c) Carnegie Mellon University.  All rights reserved.
//
// All use of this software is subject to the terms of the revised BSD
// license.  You should have received a copy of this license along
// with this source code in a file named "LICENSE."
//
////////////////////////////////////////////////////////////////////


#include "cppFunctionType.h"
#include "cppParameterList.h"
#include "cppSimpleType.h"
#include "cppInstance.h"

////////////////////////////////////////////////////////////////////
//     Function: CPPFunctionType::Constructor
//       Access: Public
//  Description:
////////////////////////////////////////////////////////////////////
CPPFunctionType::
CPPFunctionType(CPPType *return_type, CPPParameterList *parameters,
                int flags) :
  CPPType(CPPFile()),
  _return_type(return_type),
  _parameters(parameters),
  _flags(flags)
{
  _class_owner = NULL;

  // If the parameter list contains just the token "void", it means no
  // parameters.
  if (_parameters->_parameters.size() == 1 &&
      _parameters->_parameters.front()->_type->as_simple_type() != NULL &&
      _parameters->_parameters.front()->_type->as_simple_type()->_type ==
      CPPSimpleType::T_void &&
      _parameters->_parameters.front()->_ident == NULL) {
    _parameters->_parameters.clear();
  }
}

////////////////////////////////////////////////////////////////////
//     Function: CPPFunctionType::Copy Constructor
//       Access: Public
//  Description:
////////////////////////////////////////////////////////////////////
CPPFunctionType::
CPPFunctionType(const CPPFunctionType &copy) :
  CPPType(copy),
  _return_type(copy._return_type),
  _parameters(copy._parameters),
  _flags(copy._flags),
  _class_owner(copy._class_owner)
{
}

////////////////////////////////////////////////////////////////////
//     Function: CPPFunctionType::Copy Assignment Operator
//       Access: Public
//  Description:
////////////////////////////////////////////////////////////////////
void CPPFunctionType::
operator = (const CPPFunctionType &copy) {
  CPPType::operator = (copy);
  _return_type = copy._return_type;
  _parameters = copy._parameters;
  _flags = copy._flags;
  _class_owner = copy._class_owner;
}

////////////////////////////////////////////////////////////////////
//     Function: CPPFunctionType::is_fully_specified
//       Access: Public, Virtual
//  Description: Returns true if this declaration is an actual,
//               factual declaration, or false if some part of the
//               declaration depends on a template parameter which has
//               not yet been instantiated.
////////////////////////////////////////////////////////////////////
bool CPPFunctionType::
is_fully_specified() const {
  return CPPType::is_fully_specified() &&
    _return_type->is_fully_specified() &&
    _parameters->is_fully_specified();
}

////////////////////////////////////////////////////////////////////
//     Function: CPPFunctionType::substitute_decl
//       Access: Public, Virtual
//  Description:
////////////////////////////////////////////////////////////////////
CPPDeclaration *CPPFunctionType::
substitute_decl(CPPDeclaration::SubstDecl &subst,
                CPPScope *current_scope, CPPScope *global_scope) {
  SubstDecl::const_iterator si = subst.find(this);
  if (si != subst.end()) {
    return (*si).second;
  }

  CPPFunctionType *rep = new CPPFunctionType(*this);
  rep->_return_type =
    _return_type->substitute_decl(subst, current_scope, global_scope)
    ->as_type();

  rep->_parameters =
    _parameters->substitute_decl(subst, current_scope, global_scope);

  if (rep->_return_type == _return_type &&
      rep->_parameters == _parameters) {
    delete rep;
    rep = this;
  }
  rep = CPPType::new_type(rep)->as_function_type();

  subst.insert(SubstDecl::value_type(this, rep));
  return rep;
}

////////////////////////////////////////////////////////////////////
//     Function: CPPFunctionType::resolve_type
//       Access: Public, Virtual
//  Description: If this CPPType object is a forward reference or
//               other nonspecified reference to a type that might now
//               be known a real type, returns the real type.
//               Otherwise returns the type itself.
////////////////////////////////////////////////////////////////////
CPPType *CPPFunctionType::
resolve_type(CPPScope *current_scope, CPPScope *global_scope) {
  CPPType *rtype = _return_type->resolve_type(current_scope, global_scope);
  CPPParameterList *params =
    _parameters->resolve_type(current_scope, global_scope);

  if (rtype != _return_type || params != _parameters) {
    CPPFunctionType *rep = new CPPFunctionType(*this);
    rep->_return_type = rtype;
    rep->_parameters = params;
    return CPPType::new_type(rep);
  }
  return this;
}

////////////////////////////////////////////////////////////////////
//     Function: CPPFunctionType::is_tbd
//       Access: Public, Virtual
//  Description: Returns true if the type, or any nested type within
//               the type, is a CPPTBDType and thus isn't fully
//               determined right now.  In this case, calling
//               resolve_type() may or may not resolve the type.
////////////////////////////////////////////////////////////////////
bool CPPFunctionType::
is_tbd() const {
  if (_return_type->is_tbd()) {
    return true;
  }
  return _parameters->is_tbd();
}

////////////////////////////////////////////////////////////////////
//     Function: CPPFunctionType::is_trivial
//       Access: Public, Virtual
//  Description: Returns true if the type is considered a Plain Old
//               Data (POD) type.
////////////////////////////////////////////////////////////////////
bool CPPFunctionType::
is_trivial() const {
  return false;
}

////////////////////////////////////////////////////////////////////
//     Function: CPPFunctionType::output
//       Access: Public, Virtual
//  Description:
////////////////////////////////////////////////////////////////////
void CPPFunctionType::
output(ostream &out, int indent_level, CPPScope *scope, bool complete) const {
  output(out, indent_level, scope, complete, -1);
}

////////////////////////////////////////////////////////////////////
//     Function: CPPFunctionType::output
//       Access: Public
//  Description: The additional parameter allows us to specify the
//               number of parameters we wish to show the default
//               values for.  If num_default_parameters is >= 0, it
//               indicates the number of default parameter values to
//               show on output.  Otherwise, all parameter values are
//               shown.
////////////////////////////////////////////////////////////////////
void CPPFunctionType::
output(ostream &out, int indent_level, CPPScope *scope, bool complete,
       int num_default_parameters) const {
  _return_type->output(out, indent_level, scope, complete);
  out << "(";
  _parameters->output(out, scope, true, num_default_parameters);
  out << ")";
  if (_flags & F_const_method) {
    out << " const";
  }
}

////////////////////////////////////////////////////////////////////
//     Function: CPPFunctionType::output_instance
//       Access: Public, Virtual
//  Description: Formats a C++-looking line that defines an instance
//               of the given type, with the indicated name.  In most
//               cases this will be "type name", but some types have
//               special exceptions.
////////////////////////////////////////////////////////////////////
void CPPFunctionType::
output_instance(ostream &out, int indent_level, CPPScope *scope,
                bool complete, const string &prename,
                const string &name) const {
  output_instance(out, indent_level, scope, complete, prename, name, -1);
}

////////////////////////////////////////////////////////////////////
//     Function: CPPFunctionType::output_instance
//       Access: Public
//  Description: The additional parameter allows us to specify the
//               number of parameters we wish to show the default
//               values for.  If num_default_parameters is >= 0, it
//               indicates the number of default parameter values to
//               show on output.  Otherwise, all parameter values are
//               shown.
////////////////////////////////////////////////////////////////////
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
  if (_flags & F_noexcept) {
    out << " noexcept";
  }
}

////////////////////////////////////////////////////////////////////
//     Function: CPPFunctionType::get_num_default_parameters
//       Access: Public
//  Description: Returns the number of parameters in the list that may
//               take default values.
////////////////////////////////////////////////////////////////////
int CPPFunctionType::
get_num_default_parameters() const {
  // The trick is just to count, beginning from the end and working
  // towards the front, the number of parameters that have some
  // initializer.

  const CPPParameterList::Parameters &params = _parameters->_parameters;
  CPPParameterList::Parameters::const_reverse_iterator pi;
  int count = 0;
  for (pi = params.rbegin();
       pi != params.rend() && (*pi)->_initializer != (CPPExpression *)NULL;
       ++pi) {
    count++;
  }

  return count;
}

////////////////////////////////////////////////////////////////////
//     Function: CPPFunctionType::get_subtype
//       Access: Public, Virtual
//  Description:
////////////////////////////////////////////////////////////////////
CPPDeclaration::SubType CPPFunctionType::
get_subtype() const {
  return ST_function;
}

////////////////////////////////////////////////////////////////////
//     Function: CPPFunctionType::as_function_type
//       Access: Public, Virtual
//  Description:
////////////////////////////////////////////////////////////////////
CPPFunctionType *CPPFunctionType::
as_function_type() {
  return this;
}

////////////////////////////////////////////////////////////////////
//     Function: CPPFunctionType::is_equivalent_function
//       Access: Public
//  Description: This is similar to is_equal(), except it is more
//               forgiving: it considers the functions to be
//               equivalent only if the return type and the types of
//               all parameters match.
////////////////////////////////////////////////////////////////////
bool CPPFunctionType::
is_equivalent_function(const CPPFunctionType &other) const {
  if (!_return_type->is_equivalent(*other._return_type)) {
    return false;
  }

  if (_flags != other._flags) {
    return false;
  }

  if (!_parameters->is_equivalent(*other._parameters)) {
    return false;
  }

  return true;
}

////////////////////////////////////////////////////////////////////
//     Function: CPPFunctionType::is_equal
//       Access: Protected, Virtual
//  Description: Called by CPPDeclaration() to determine whether this type is
//               equivalent to another type of the same type.
////////////////////////////////////////////////////////////////////
bool CPPFunctionType::
is_equal(const CPPDeclaration *other) const {
  const CPPFunctionType *ot = ((CPPDeclaration *)other)->as_function_type();
  assert(ot != NULL);

  if (_return_type != ot->_return_type) {
    return false;
  }
  if (_flags != ot->_flags) {
    return false;
  }
  if (*_parameters != *ot->_parameters) {
    return false;
  }
  return true;
}


////////////////////////////////////////////////////////////////////
//     Function: CPPFunctionType::is_less
//       Access: Protected, Virtual
//  Description: Called by CPPDeclaration() to determine whether this type
//               should be ordered before another type of the same
//               type, in an arbitrary but fixed ordering.
////////////////////////////////////////////////////////////////////
bool CPPFunctionType::
is_less(const CPPDeclaration *other) const {
  const CPPFunctionType *ot = ((CPPDeclaration *)other)->as_function_type();
  assert(ot != NULL);

  if (_return_type != ot->_return_type) {
    return _return_type < ot->_return_type;
  }
  if (_flags != ot->_flags) {
    return _flags < ot->_flags;
  }

  return *_parameters < *ot->_parameters;
}
