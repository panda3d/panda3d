// Filename: cppInstance.cxx
// Created by:  drose (19Oct99)
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


#include "cppInstance.h"
#include "cppInstanceIdentifier.h"
#include "cppIdentifier.h"
#include "cppTemplateScope.h"
#include "cppFunctionType.h"
#include "cppSimpleType.h"
#include "cppExpression.h"
#include "cppPreprocessor.h"
#include "cppParameterList.h"
#include "cppReferenceType.h"
#include "cppConstType.h"
#include "indent.h"

#include <algorithm>

////////////////////////////////////////////////////////////////////
//     Function: CPPInstance::Constructor
//       Access: Public
//  Description:
////////////////////////////////////////////////////////////////////
CPPInstance::
CPPInstance(CPPType *type, const string &name, int storage_class) :
  CPPDeclaration(CPPFile()),
  _type(type),
  _ident(new CPPIdentifier(name)),
  _storage_class(storage_class)
{
  _initializer = NULL;
}

////////////////////////////////////////////////////////////////////
//     Function: CPPInstance::Constructor
//       Access: Public
//  Description:
////////////////////////////////////////////////////////////////////
CPPInstance::
CPPInstance(CPPType *type, CPPIdentifier *ident, int storage_class) :
  CPPDeclaration(CPPFile()),
  _type(type),
  _ident(ident),
  _storage_class(storage_class)
{
  _initializer = NULL;
}

////////////////////////////////////////////////////////////////////
//     Function: CPPInstance::Constructor
//       Access: Public
//  Description: Constructs a new CPPInstance object that defines a
//               variable of the indicated type according to the type
//               and the InstanceIdentifier.  The InstanceIdentifier
//               pointer is deallocated.
////////////////////////////////////////////////////////////////////
CPPInstance::
CPPInstance(CPPType *type, CPPInstanceIdentifier *ii, int storage_class,
            const CPPFile &file) :
  CPPDeclaration(file)
{
  _type = ii->unroll_type(type);
  _ident = ii->_ident;
  ii->_ident = NULL;
  _storage_class = storage_class;
  _initializer = NULL;

  CPPParameterList *params = ii->get_initializer();
  if (params != (CPPParameterList *)NULL) {
    // In this case, the instance has a parameter-list initializer, e.g.:
    //
    //   int foo(0);
    //
    // We really should save this initializer in the instance object.
    // But we don't for now, since no one really cares about
    // initializers anyway.
  }

  delete ii;
}

////////////////////////////////////////////////////////////////////
//     Function: CPPInstance::Copy Constructor
//       Access: Public
//  Description:
////////////////////////////////////////////////////////////////////
CPPInstance::
CPPInstance(const CPPInstance &copy) :
  CPPDeclaration(copy),
  _type(copy._type),
  _ident(copy._ident),
  _initializer(copy._initializer),
  _storage_class(copy._storage_class)
{
  assert(_type != NULL);
}

////////////////////////////////////////////////////////////////////
//     Function: CPPInstance::Destructor
//       Access: Public
//  Description:
////////////////////////////////////////////////////////////////////
CPPInstance::
~CPPInstance() {
  // Can't delete the identifier.  Don't try.
}


////////////////////////////////////////////////////////////////////
//     Function: CPPInstance::make_typecast_function
//       Access: Public, Static
//  Description: Constructs and returns a new CPPInstance object that
//               corresponds to a function prototype declaration for a
//               typecast method, whose return type is implicit in the
//               identifier type.
////////////////////////////////////////////////////////////////////
CPPInstance *CPPInstance::
make_typecast_function(CPPInstance *inst, CPPIdentifier *ident,
                       CPPParameterList *parameters, int function_flags) {
  CPPType *type = CPPType::new_type(inst->_type);
  delete inst;

  function_flags |= (int)CPPFunctionType::F_operator_typecast;

  CPPType *ft =
    CPPType::new_type(new CPPFunctionType(type, parameters, function_flags));

  return new CPPInstance(ft, ident);
}

////////////////////////////////////////////////////////////////////
//     Function: CPPInstance::Equivalence Operator
//       Access: Public
//  Description:
////////////////////////////////////////////////////////////////////
bool CPPInstance::
operator == (const CPPInstance &other) const {
  if (_type != other._type) {
    return false;
  }
  if (_storage_class != other._storage_class) {
    return false;
  }

  // We *do* care about the identifier.  We need to differentiate
  // types of function variables, among possibly other things, based
  // on the identifier.
  if ((_ident == NULL && other._ident != NULL) ||
      (_ident != NULL && other._ident == NULL) ||
      (_ident != NULL && other._ident != NULL && *_ident != *other._ident))
  {
    return false;
  }

  // We similarly care about the initializer.
  if ((_initializer == NULL && other._initializer != NULL) ||
      (_initializer != NULL && other._initializer == NULL) ||
      (_initializer != NULL && other._initializer != NULL &&
       *_initializer != *other._initializer))
  {
    return false;
  }

  return true;
}

////////////////////////////////////////////////////////////////////
//     Function: CPPInstance::Nonequivalence Operator
//       Access: Public
//  Description:
////////////////////////////////////////////////////////////////////
bool CPPInstance::
operator != (const CPPInstance &other) const {
  return !operator == (other);
}

////////////////////////////////////////////////////////////////////
//     Function: CPPInstance::Ordering Operator
//       Access: Public
//  Description:
////////////////////////////////////////////////////////////////////
bool CPPInstance::
operator < (const CPPInstance &other) const {
  if (_type != other._type) {
    return _type < other._type;
  }
  if (_storage_class != other._storage_class) {
    return _storage_class < other._storage_class;
  }

  // We *do* care about the identifier.  We need to differentiate
  // types of function variables, among possibly other things, based
  // on the identifier.
  if ((_ident == NULL && other._ident != NULL) ||
      (_ident != NULL && other._ident == NULL) ||
      (_ident != NULL && other._ident != NULL && *_ident != *other._ident))
  {
    if (_ident == NULL || other._ident == NULL) {
      return _ident < other._ident;
    }
    return *_ident < *other._ident;
  }

  // We similarly care about the initializer.
  if ((_initializer == NULL && other._initializer != NULL) ||
      (_initializer != NULL && other._initializer == NULL) ||
      (_initializer != NULL && other._initializer != NULL &&
       *_initializer != *other._initializer))
  {
    if (_initializer == NULL || other._initializer == NULL) {
      return _initializer < other._initializer;
    }
    return *_initializer < *other._initializer;
  }

  return false;
}

////////////////////////////////////////////////////////////////////
//     Function: CPPInstance::set_initializer
//       Access: Public
//  Description: Sets the value of the expression that is used to
//               initialize the variable, or the default value for a
//               parameter.  If a non-null expression is set on a
//               function declaration, it implies that the function is
//               pure virtual.
////////////////////////////////////////////////////////////////////
void CPPInstance::
set_initializer(CPPExpression *initializer) {
  if (_type->as_function_type() != (CPPFunctionType *)NULL) {
    // This is a function declaration.
    if (initializer == (CPPExpression *)NULL) {
      _storage_class &= ~SC_pure_virtual;
    } else {
      _storage_class |= SC_pure_virtual;
    }
    _initializer = (CPPExpression *)NULL;
  } else {
    _initializer = initializer;
  }
}

////////////////////////////////////////////////////////////////////
//     Function: CPPInstance::is_scoped
//       Access: Public
//  Description:
////////////////////////////////////////////////////////////////////
bool CPPInstance::
is_scoped() const {
  if (_ident == NULL) {
    return false;
  } else {
    return _ident->is_scoped();
  }
}

////////////////////////////////////////////////////////////////////
//     Function: CPPInstance::get_scope
//       Access: Public
//  Description:
////////////////////////////////////////////////////////////////////
CPPScope *CPPInstance::
get_scope(CPPScope *current_scope, CPPScope *global_scope,
          CPPPreprocessor *error_sink) const {
  if (_ident == NULL) {
    return current_scope;
  } else {
    return _ident->get_scope(current_scope, global_scope, error_sink);
  }
}

////////////////////////////////////////////////////////////////////
//     Function: CPPInstance::get_simple_name
//       Access: Public
//  Description:
////////////////////////////////////////////////////////////////////
string CPPInstance::
get_simple_name() const {
  if (_ident == NULL) {
    return "";
  } else {
    return _ident->get_simple_name();
  }
}

////////////////////////////////////////////////////////////////////
//     Function: CPPInstance::get_local_name
//       Access: Public
//  Description:
////////////////////////////////////////////////////////////////////
string CPPInstance::
get_local_name(CPPScope *scope) const {
  if (_ident == NULL) {
    return "";
  } else {
    return _ident->get_local_name(scope);
  }
}

////////////////////////////////////////////////////////////////////
//     Function: CPPInstance::get_fully_scoped_name
//       Access: Public
//  Description:
////////////////////////////////////////////////////////////////////
string CPPInstance::
get_fully_scoped_name() const {
  if (_ident == NULL) {
    return "";
  } else {
    return _ident->get_fully_scoped_name();
  }
}

////////////////////////////////////////////////////////////////////
//     Function: CPPInstance::check_for_constructor
//       Access: Public
//  Description: If this is a function type instance, checks whether
//               the function name matches the class name (or ~name),
//               and if so, flags it as a constructor (or destructor).
////////////////////////////////////////////////////////////////////
void CPPInstance::
check_for_constructor(CPPScope *current_scope, CPPScope *global_scope) {
  CPPScope *scope = get_scope(current_scope, global_scope);
  if (scope == NULL) {
    scope = current_scope;
  }

  CPPFunctionType *func = _type->as_function_type();
  if (func != NULL) {
    string method_name = get_local_name(scope);
    string class_name = scope->get_local_name();

    if (!method_name.empty() && !class_name.empty()) {
      if (method_name == class_name) {
        CPPType *void_type = CPPType::new_type
          (new CPPSimpleType(CPPSimpleType::T_void));

        int flags = func->_flags | CPPFunctionType::F_constructor;

        // Check if it might be a copy or move constructor.
        CPPParameterList *params = func->_parameters;
        if (params->_parameters.size() == 1 && !params->_includes_ellipsis) {
          CPPType *param_type = params->_parameters[0]->_type;
          CPPReferenceType *ref_type = param_type->as_reference_type();

          if (ref_type != NULL) {
            param_type = ref_type->_pointing_at;

            if (param_type->get_subtype() == CPPDeclaration::ST_const) {
              param_type = param_type->as_const_type()->_wrapped_around;
            }

            if (class_name == param_type->get_simple_name()) {
              if (ref_type->_value_category == CPPReferenceType::VC_rvalue) {
                flags |= CPPFunctionType::F_move_constructor;
              } else {
                flags |= CPPFunctionType::F_copy_constructor;
              }
            }
          }
        }

        _type = CPPType::new_type
          (new CPPFunctionType(void_type, func->_parameters, flags));

      } else if (method_name == "~" + class_name) {
        CPPType *void_type = CPPType::new_type
          (new CPPSimpleType(CPPSimpleType::T_void));

        _type = CPPType::new_type
          (new CPPFunctionType(void_type, func->_parameters,
                               func->_flags | CPPFunctionType::F_destructor));
      }
    }
  }
}

////////////////////////////////////////////////////////////////////
//     Function: CPPInstance::instantiate
//       Access: Public
//  Description:
////////////////////////////////////////////////////////////////////
CPPDeclaration *CPPInstance::
instantiate(const CPPTemplateParameterList *actual_params,
            CPPScope *current_scope, CPPScope *global_scope,
            CPPPreprocessor *error_sink) const {

  if (!is_template()) {
    if (error_sink != NULL) {
      error_sink->warning("Ignoring template parameters for instance " +
                          _ident->get_local_name());
    }
    return (CPPInstance *)this;
  }

  Instantiations::const_iterator ii;
  ii = _instantiations.find(actual_params);
  if (ii != _instantiations.end()) {
    // We've already instantiated this instance with these parameters.
    // Return that.
    return (*ii).second;
  }


  CPPTemplateScope *tscope = get_template_scope();

  CPPDeclaration::SubstDecl subst;
  actual_params->build_subst_decl(tscope->_parameters, subst,
                                  current_scope, global_scope);

  CPPInstance *inst =
    ((CPPInstance *)this)->substitute_decl(subst, current_scope, global_scope)
    ->as_instance();
  if (inst == this) {
    // Hmm, nothing to substitute.  Make a new instance anyway, so we
    // can change the name.
    inst = new CPPInstance(*this);
  }
  assert(inst != NULL);
  inst->_ident = inst->_ident->substitute_decl(subst, current_scope, global_scope);
  if (inst->_ident == _ident) {
    inst->_ident = new CPPIdentifier(*inst->_ident);
  }
  inst->_ident->_names.back().set_templ
    (new CPPTemplateParameterList(*actual_params));

  inst->_template_scope = NULL;

  ((CPPInstance *)this)->_instantiations.insert(Instantiations::value_type(actual_params, inst));
  return inst;
}

////////////////////////////////////////////////////////////////////
//     Function: CPPInstance::is_fully_specified
//       Access: Public, Virtual
//  Description: Returns true if this declaration is an actual,
//               factual declaration, or false if some part of the
//               declaration depends on a template parameter which has
//               not yet been instantiated.
////////////////////////////////////////////////////////////////////
bool CPPInstance::
is_fully_specified() const {
  if (_ident != NULL && !_ident->is_fully_specified()) {
    return false;
  }
  if (_initializer != NULL && !_initializer->is_fully_specified()) {
    return false;
  }
  return CPPDeclaration::is_fully_specified() &&
    _type->is_fully_specified();
}

////////////////////////////////////////////////////////////////////
//     Function: CPPInstance::substitute_decl
//       Access: Public, Virtual
//  Description:
////////////////////////////////////////////////////////////////////
CPPDeclaration *CPPInstance::
substitute_decl(CPPDeclaration::SubstDecl &subst,
                CPPScope *current_scope, CPPScope *global_scope) {
  CPPDeclaration *top =
    CPPDeclaration::substitute_decl(subst, current_scope, global_scope);
  if (top != this) {
    return top;
  }

  CPPInstance *rep = new CPPInstance(*this);
  CPPDeclaration *new_type =
    _type->substitute_decl(subst, current_scope, global_scope);
  rep->_type = new_type->as_type();

  if (rep->_type == NULL) {
    rep->_type = _type;
  }

  if (_initializer != NULL) {
    rep->_initializer =
      _initializer->substitute_decl(subst, current_scope, global_scope)
      ->as_expression();
  }

  if (rep->_type == _type &&
      rep->_initializer == _initializer) {
    delete rep;
    rep = this;
  }

  subst.insert(SubstDecl::value_type(this, rep));
  return rep;
}

////////////////////////////////////////////////////////////////////
//     Function: CPPInstance::output
//       Access: Public, Virtual
//  Description:
////////////////////////////////////////////////////////////////////
void CPPInstance::
output(ostream &out, int indent_level, CPPScope *scope, bool complete) const {
  output(out, indent_level, scope, complete, -1);
}

////////////////////////////////////////////////////////////////////
//     Function: CPPInstance::output
//       Access: Public
//  Description: The extra parameter comes into play only when we
//               happen to be outputting a function prototype.  See
//               CPPFunctionType::output().
////////////////////////////////////////////////////////////////////
void CPPInstance::
output(ostream &out, int indent_level, CPPScope *scope, bool complete,
       int num_default_parameters) const {
  assert(_type != NULL);

  if (_type->is_parameter_expr()) {
    // In this case, the whole thing is really an expression, and not
    // an instance at all.  This can only happen if we parsed an
    // instance declaration while we thought we were parsing a
    // function prototype.
    out << *_initializer;
    return;
  }

  if (is_template()) {
    get_template_scope()->_parameters.write_formal(out, scope);
    indent(out, indent_level);
  }
  if (_storage_class & SC_static) {
    out << "static ";
  }
  if (_storage_class & SC_extern) {
    out << "extern ";
  }
  if (_storage_class & SC_c_binding) {
    out << "\"C\" ";
  }
  if (_storage_class & SC_virtual) {
    out << "virtual ";
  }
  if (_storage_class & SC_inline) {
    out << "inline ";
  }
  if (_storage_class & SC_explicit) {
    out << "explicit ";
  }
  if (_storage_class & SC_register) {
    out << "register ";
  }
  if (_storage_class & SC_volatile) {
    out << "volatile ";
  }
  if (_storage_class & SC_mutable) {
    out << "mutable ";
  }

  string name;
  if (_ident != NULL) {
    name = _ident->get_local_name(scope);
  }

  if (_type->as_function_type()) {
    _type->as_function_type()->
      output_instance(out, indent_level, scope, complete, "", name,
                      num_default_parameters);

  } else {
    _type->output_instance(out, indent_level, scope, complete, "", name);
  }

  if (_storage_class & SC_pure_virtual) {
    out << " = 0";
  }
  if (_initializer != NULL) {
    out << " = " << *_initializer;
  }
}


////////////////////////////////////////////////////////////////////
//     Function: CPPInstance::get_subtype
//       Access: Public, Virtual
//  Description:
////////////////////////////////////////////////////////////////////
CPPDeclaration::SubType CPPInstance::
get_subtype() const {
  return ST_instance;
}

////////////////////////////////////////////////////////////////////
//     Function: CPPInstance::as_instance
//       Access: Public, Virtual
//  Description:
////////////////////////////////////////////////////////////////////
CPPInstance *CPPInstance::
as_instance() {
  return this;
}
