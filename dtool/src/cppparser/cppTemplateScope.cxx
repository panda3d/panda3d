// Filename: cppTemplateScope.cxx
// Created by:  drose (28Oct99)
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


#include "cppTemplateScope.h"
#include "cppExtensionType.h"
#include "cppClassTemplateParameter.h"
#include "cppIdentifier.h"
#include "cppTypedefType.h"

////////////////////////////////////////////////////////////////////
//     Function: CPPTemplateScope::Constructor
//       Access: Public
//  Description:
////////////////////////////////////////////////////////////////////
CPPTemplateScope::
CPPTemplateScope(CPPScope *parent_scope) :
  CPPScope(parent_scope, CPPNameComponent("template"), V_public)
{
}


////////////////////////////////////////////////////////////////////
//     Function: CPPTemplateScope::add_declaration
//       Access: Public, Virtual
//  Description:
////////////////////////////////////////////////////////////////////
void CPPTemplateScope::
add_declaration(CPPDeclaration *decl, CPPScope *global_scope,
                CPPPreprocessor *preprocessor,
                const cppyyltype &pos) {
  decl->_template_scope = this;
  assert(_parent_scope != NULL);
  _parent_scope->add_declaration(decl, global_scope, preprocessor, pos);
}

////////////////////////////////////////////////////////////////////
//     Function: CPPTemplateScope::add_enum_value
//       Access: Public, Virtual
//  Description:
////////////////////////////////////////////////////////////////////
void CPPTemplateScope::
add_enum_value(CPPInstance *inst, CPPPreprocessor *preprocessor,
               const cppyyltype &pos) {
  inst->_template_scope = this;
  assert(_parent_scope != NULL);
  _parent_scope->add_enum_value(inst, preprocessor, pos);
}

////////////////////////////////////////////////////////////////////
//     Function: CPPTemplateScope::define_extension_type
//       Access: Public, Virtual
//  Description:
////////////////////////////////////////////////////////////////////
void CPPTemplateScope::
define_extension_type(CPPExtensionType *type, CPPPreprocessor *error_sink) {
  type->_template_scope = this;
  assert(_parent_scope != NULL);
  _parent_scope->define_extension_type(type, error_sink);
}

////////////////////////////////////////////////////////////////////
//     Function: CPPTemplateScope::define_namespace
//       Access: Public, Virtual
//  Description:
////////////////////////////////////////////////////////////////////
void CPPTemplateScope::
define_namespace(CPPNamespace *scope) {
  assert(_parent_scope != NULL);
  _parent_scope->define_namespace(scope);
}

////////////////////////////////////////////////////////////////////
//     Function: CPPTemplateScope::add_using
//       Access: Public, Virtual
//  Description:
////////////////////////////////////////////////////////////////////
void CPPTemplateScope::
add_using(CPPUsing *using_decl, CPPScope *global_scope,
          CPPPreprocessor *error_sink) {
  assert(_parent_scope != NULL);
  _parent_scope->add_using(using_decl, global_scope, error_sink);
}

////////////////////////////////////////////////////////////////////
//     Function: CPPTemplateScope::add_template_parameter
//       Access: Public
//  Description:
////////////////////////////////////////////////////////////////////
void CPPTemplateScope::
add_template_parameter(CPPDeclaration *param) {
  _parameters._parameters.push_back(param);
  CPPClassTemplateParameter *cl = param->as_class_template_parameter();
  if (cl != NULL) {
    // Create an implicit typedef for this class parameter.
    string name = cl->_ident->get_local_name();
    _types[name] = cl;
  }

  CPPInstance *inst = param->as_instance();
  if (inst != NULL) {
    // Register the variable for this value parameter.
    string name = inst->get_local_name();
    if (!name.empty()) {
      _variables[name] = inst;
    }
  }
}

////////////////////////////////////////////////////////////////////
//     Function: CPPTemplateScope::is_fully_specified
//       Access: Public, Virtual
//  Description: Returns true if this declaration is an actual,
//               factual declaration, or false if some part of the
//               declaration depends on a template parameter which has
//               not yet been instantiated.
////////////////////////////////////////////////////////////////////
bool CPPTemplateScope::
is_fully_specified() const {
  return false;
}

////////////////////////////////////////////////////////////////////
//     Function: CPPTemplateScope::get_simple_name
//       Access: Public, Virtual
//  Description:
////////////////////////////////////////////////////////////////////
string CPPTemplateScope::
get_simple_name() const {
  assert(_parent_scope != NULL);
  return _parent_scope->get_simple_name();
}

////////////////////////////////////////////////////////////////////
//     Function: CPPTemplateScope::get_local_name
//       Access: Public, Virtual
//  Description:
////////////////////////////////////////////////////////////////////
string CPPTemplateScope::
get_local_name(CPPScope *scope) const {
  assert(_parent_scope != NULL);
  return _parent_scope->get_local_name(scope);
}

////////////////////////////////////////////////////////////////////
//     Function: CPPTemplateScope::get_fully_scoped_name
//       Access: Public, Virtual
//  Description:
////////////////////////////////////////////////////////////////////
string CPPTemplateScope::
get_fully_scoped_name() const {
  assert(_parent_scope != NULL);
  return _parent_scope->get_fully_scoped_name();
}

////////////////////////////////////////////////////////////////////
//     Function: CPPTemplateScope::output
//       Access: Public, Virtual
//  Description:
////////////////////////////////////////////////////////////////////
void CPPTemplateScope::
output(ostream &out, CPPScope *scope) const {
  CPPScope::output(out, scope);
  out << "< ";
  _parameters.output(out, scope);
  out << " >";
}

////////////////////////////////////////////////////////////////////
//     Function: CPPTemplateScope::as_template_scope
//       Access: Public, Virtual
//  Description:
////////////////////////////////////////////////////////////////////
CPPTemplateScope *CPPTemplateScope::
as_template_scope() {
  return this;
}
