// Filename: cppScope.cxx
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


#include "cppScope.h"
#include "cppDeclaration.h"
#include "cppNamespace.h"
#include "cppTypedefType.h"
#include "cppTypeDeclaration.h"
#include "cppExtensionType.h"
#include "cppInstance.h"
#include "cppInstanceIdentifier.h"
#include "cppIdentifier.h"
#include "cppStructType.h"
#include "cppFunctionGroup.h"
#include "cppPreprocessor.h"
#include "cppTemplateScope.h"
#include "cppClassTemplateParameter.h"
#include "cppFunctionType.h"
#include "cppUsing.h"
#include "cppBisonDefs.h"
#include "indent.h"

////////////////////////////////////////////////////////////////////
//     Function: CPPScope::Constructor
//       Access: Public
//  Description:
////////////////////////////////////////////////////////////////////
CPPScope::
CPPScope(CPPScope *parent_scope,
         const CPPNameComponent &name, CPPVisibility starting_vis) :
  _name(name),
  _parent_scope(parent_scope),
  _current_vis(starting_vis)
{
  _struct_type = NULL;
  _is_fully_specified = false;
  _fully_specified_known = false;
  _is_fully_specified_recursive_protect = false;
  _subst_decl_recursive_protect = false;
}

////////////////////////////////////////////////////////////////////
//     Function: CPPScope::Destructor
//       Access: Public, Virtual
//  Description:
////////////////////////////////////////////////////////////////////
CPPScope::
~CPPScope() {
}

////////////////////////////////////////////////////////////////////
//     Function: CPPScope::set_struct_type
//       Access: Public
//  Description: Sets the struct or class that owns this scope.  This
//               should only be done once, when the scope and its
//               associated struct are created.  It's provided so the
//               scope can check the struct's ancestry for inherited
//               symbols.
////////////////////////////////////////////////////////////////////
void CPPScope::
set_struct_type(CPPStructType *struct_type) {
  _struct_type = struct_type;
}

////////////////////////////////////////////////////////////////////
//     Function: CPPScope::get_struct_type
//       Access: Public
//  Description: Returns the class or struct that defines this scope,
//               if any.
////////////////////////////////////////////////////////////////////
CPPStructType *CPPScope::
get_struct_type() const {
  return _struct_type;
}

////////////////////////////////////////////////////////////////////
//     Function: CPPScope::get_parent_scope
//       Access: Public
//  Description: Returns the parent scope of this scope, if any.
////////////////////////////////////////////////////////////////////
CPPScope *CPPScope::
get_parent_scope() const {
  return _parent_scope;
}

////////////////////////////////////////////////////////////////////
//     Function: CPPScope::set_current_vis
//       Access: Public
//  Description:
////////////////////////////////////////////////////////////////////
void CPPScope::
set_current_vis(CPPVisibility current_vis) {
  _current_vis = current_vis;
}

////////////////////////////////////////////////////////////////////
//     Function: CPPScope::get_current_vis
//       Access: Public
//  Description:
////////////////////////////////////////////////////////////////////
CPPVisibility CPPScope::
get_current_vis() const {
  return _current_vis;
}

////////////////////////////////////////////////////////////////////
//     Function: CPPScope::add_declaration
//       Access: Public, Virtual
//  Description:
////////////////////////////////////////////////////////////////////
void CPPScope::
add_declaration(CPPDeclaration *decl, CPPScope *global_scope,
                CPPPreprocessor *preprocessor, const cppyyltype &pos) {
  decl->_vis = _current_vis;

  // Get the recent comments from the preprocessor.  These are the
  // comments that appeared preceding this particular declaration;
  // they might be relevant to the declaration.

  if (decl->_leading_comment == (CPPCommentBlock *)NULL) {
    decl->_leading_comment =
      preprocessor->get_comment_before(pos.first_line, pos.file);
  }

  _declarations.push_back(decl);

  handle_declaration(decl, global_scope, preprocessor);
}

////////////////////////////////////////////////////////////////////
//     Function: CPPScope::add_enum_value
//       Access: Public, Virtual
//  Description:
////////////////////////////////////////////////////////////////////
void CPPScope::
add_enum_value(CPPInstance *inst, CPPPreprocessor *preprocessor,
               const cppyyltype &pos) {
  inst->_vis = _current_vis;

  if (inst->_leading_comment == (CPPCommentBlock *)NULL) {
    // Same-line comment?
    CPPCommentBlock *comment =
      preprocessor->get_comment_on(pos.first_line, pos.file);

    if (comment == (CPPCommentBlock *)NULL) {
      // Nope.  Check for a comment before this line.
      comment =
        preprocessor->get_comment_before(pos.first_line, pos.file);

      if (comment != NULL) {
        // This is a bit of a hack, but it prevents us from picking
        // up a same-line comment from the previous line.
        if (comment->_line_number != pos.first_line - 1 ||
            comment->_col_number <= pos.first_column) {

          inst->_leading_comment = comment;
        }
      }
    } else {
      inst->_leading_comment = comment;
    }
  }

  string name = inst->get_simple_name();
  if (!name.empty()) {
    _enum_values[name] = inst;
  }
}

////////////////////////////////////////////////////////////////////
//     Function: CPPScope::define_extension_type
//       Access: Public, Virtual
//  Description:
////////////////////////////////////////////////////////////////////
void CPPScope::
define_extension_type(CPPExtensionType *type, CPPPreprocessor *error_sink) {
  assert(type != NULL);
  string name = type->get_local_name(this);
  if (name.empty()) {
    return;
  }

  switch (type->_type) {
  case CPPExtensionType::T_class:
    _classes[name] = type;
    break;

  case CPPExtensionType::T_struct:
    _structs[name] = type;
    break;

  case CPPExtensionType::T_union:
    _unions[name] = type;
    break;

  case CPPExtensionType::T_enum:
    _enums[name] = type;
    break;
  }

  // Create an implicit typedef for the extension.
  //CPPTypedefType *td = new CPPTypedefType(type, name);
  pair<Types::iterator, bool> result =
    _types.insert(Types::value_type(name, type));

  if (!result.second) {
    // There's already a typedef for this extension.  This one
    // overrides it only if the other is a forward declaration.
    CPPType *other_type = (*result.first).second;

    if (other_type->get_subtype() == CPPDeclaration::ST_extension) {
      CPPExtensionType *other_ext = other_type->as_extension_type();

      if (other_ext->_type != type->_type) {
        if (error_sink != NULL) {
          ostringstream errstr;
          errstr << other_ext->_type << " " << type->get_fully_scoped_name()
                 << " was previously declared as " << other_ext->_type << "\n";
          error_sink->error(errstr.str());
        }
      }
      (*result.first).second = type;

    } else {
      CPPTypedefType *other_td = other_type->as_typedef_type();

      // Error out if the declaration is different than the previous one.
      if (other_type != type &&
          (other_td == NULL || other_td->_type != type)) {

        if (error_sink != NULL) {
          ostringstream errstr;
          type->output(errstr, 0, NULL, false);
          errstr << " has conflicting declaration as ";
          other_type->output(errstr, 0, NULL, true);
          error_sink->error(errstr.str());
        }
      }
    }
  }

  if (type->is_template()) {
    string simple_name = type->get_simple_name();

    pair<Templates::iterator, bool> result =
      _templates.insert(Templates::value_type(simple_name, type));

    if (!result.second) {
      // The template was not inserted because we already had a
      // template definition with the given name.  If the previous
      // definition was incomplete, replace it.
      CPPDeclaration *old_templ = (*result.first).second;
      CPPType *old_templ_type = old_templ->as_type();
      if (old_templ_type == NULL || old_templ_type->is_incomplete()) {
        // The previous template definition was incomplete, maybe a
        // forward reference; replace it with the good one.
        (*result.first).second = type;
      }
    }
  }
}

////////////////////////////////////////////////////////////////////
//     Function: CPPScope::define_namespace
//       Access: Public, Virtual
//  Description:
////////////////////////////////////////////////////////////////////
void CPPScope::
define_namespace(CPPNamespace *scope) {
  string name = scope->get_simple_name();

  _namespaces[name] = scope;
}

////////////////////////////////////////////////////////////////////
//     Function: CPPScope::add_using
//       Access: Public, Virtual
//  Description:
////////////////////////////////////////////////////////////////////
void CPPScope::
add_using(CPPUsing *using_decl, CPPScope *global_scope,
          CPPPreprocessor *error_sink) {
  if (using_decl->_full_namespace) {
    CPPScope *scope =
      using_decl->_ident->find_scope(this, global_scope);
    if (scope != NULL) {
      _using.insert(scope);
    } else {
      if (error_sink != NULL) {
        error_sink->warning("Attempt to use undefined namespace: " + using_decl->_ident->get_fully_scoped_name());
      }
    }
  } else {
    CPPDeclaration *decl = using_decl->_ident->find_symbol(this, global_scope);
    if (decl != NULL) {
      handle_declaration(decl, global_scope, error_sink);
    } else {
      if (error_sink != NULL) {
        error_sink->warning("Attempt to use unknown symbol: " + using_decl->_ident->get_fully_scoped_name());
      }
    }
  }
}

////////////////////////////////////////////////////////////////////
//     Function: CPPScope::is_fully_specified
//       Access: Public, Virtual
//  Description: Returns true if this declaration is an actual,
//               factual declaration, or false if some part of the
//               declaration depends on a template parameter which has
//               not yet been instantiated.
////////////////////////////////////////////////////////////////////
bool CPPScope::
is_fully_specified() const {
  if (_fully_specified_known) {
    return _is_fully_specified;
  }

  if (_is_fully_specified_recursive_protect) {
    // We're already executing this block.
    return true;
  }
  ((CPPScope *)this)->_is_fully_specified_recursive_protect = true;

  bool specified = true;

  if (_parent_scope != NULL && !_parent_scope->is_fully_specified()) {
    specified = false;
  }

  Declarations::const_iterator di;
  for (di = _declarations.begin();
       di != _declarations.end() && specified;
       ++di) {
    if (!(*di)->is_fully_specified()) {
      specified = false;
    }
  }

  ((CPPScope *)this)->_fully_specified_known = true;
  ((CPPScope *)this)->_is_fully_specified = specified;
  ((CPPScope *)this)->_is_fully_specified_recursive_protect = false;

  return specified;
}

////////////////////////////////////////////////////////////////////
//     Function: CPPScope::instantiate
//       Access: Public
//  Description:
////////////////////////////////////////////////////////////////////
CPPScope *CPPScope::
instantiate(const CPPTemplateParameterList *actual_params,
            CPPScope *current_scope, CPPScope *global_scope,
            CPPPreprocessor *error_sink) const {
  CPPScope *this_scope = (CPPScope *)this;

  if (_parent_scope == NULL ||
      _parent_scope->as_template_scope() == NULL) {
    if (error_sink != NULL) {
      error_sink->warning("Ignoring template parameters for scope " +
                          get_local_name());
    }
    return this_scope;
  }

  if (is_fully_specified()) {
    return this_scope;
  }

  Instantiations::const_iterator ii;
  ii = _instantiations.find(actual_params);
  if (ii != _instantiations.end()) {
    // We've already instantiated this scope with these parameters.
    // Return that.
    return (*ii).second;
  }

  /*
    cerr << "Instantiating " << get_simple_name()
         << "<" << *actual_params << ">\n";
  */

  // Build the mapping of formal parameters to actual parameters.
  CPPTemplateScope *tscope = _parent_scope->as_template_scope();
  CPPDeclaration::SubstDecl subst;
  actual_params->build_subst_decl(tscope->_parameters, subst,
                                  current_scope, global_scope);

  CPPScope *scope;
  if (subst.empty()) {
    scope = (CPPScope *)this;

  } else {
    CPPNameComponent name = _name;
    name.set_templ(new CPPTemplateParameterList(*actual_params));
    //    scope = new CPPScope(current_scope, name, V_public);
    scope = new CPPScope(_parent_scope, name, V_public);
    copy_substitute_decl(scope, subst, global_scope);

    // Also define any new template parameter types, in case we
    // "instantiated" this scope with another template parameter.
    CPPTemplateParameterList::Parameters::const_iterator pi;
    for (pi = actual_params->_parameters.begin();
         pi != actual_params->_parameters.end();
         ++pi) {
      CPPDeclaration *decl = (*pi);
      CPPClassTemplateParameter *ctp = decl->as_class_template_parameter();
      if (ctp != NULL) {
        //CPPTypedefType *td = new CPPTypedefType(ctp, ctp->_ident);
        //scope->_typedefs.insert(Typedefs::value_type
        //                        (ctp->_ident->get_local_name(),
        //                         td));
        scope->_types.insert(Types::value_type
                             (ctp->_ident->get_local_name(),
                              ctp));
      }
    }
  }

  // Finally, record this particular instantiation for future
  // reference, so we don't have to do this again.
  ((CPPScope *)this)->_instantiations.insert(Instantiations::value_type(actual_params, scope));

  return scope;
}

////////////////////////////////////////////////////////////////////
//     Function: CPPScope::substitute_decl
//       Access: Public, Virtual
//  Description:
////////////////////////////////////////////////////////////////////
CPPScope *CPPScope::
substitute_decl(CPPDeclaration::SubstDecl &subst,
                CPPScope *current_scope, CPPScope *global_scope) const {
  CPPScope *this_scope = (CPPScope *)this;

  if (is_fully_specified()) {
    return this_scope;
  }

  if (_subst_decl_recursive_protect) {
    // We're already executing this block.
    return this_scope;
  }
  ((CPPScope *)this)->_subst_decl_recursive_protect = true;

  CPPScope *rep = new CPPScope(current_scope, _name, V_public);
  bool anything_changed;

  if (_parent_scope != NULL &&
      _parent_scope->as_template_scope() != NULL) {
    // If the parent of this scope is a template scope--e.g. this
    // scope has template parameters--then we must first remove any of
    // the template parameters from the subst list.  These will later
    // get substituted properly during instantiation.
    const CPPTemplateParameterList &p =
      _parent_scope->as_template_scope()->_parameters;

    CPPDeclaration::SubstDecl new_subst = subst;
    CPPTemplateParameterList::Parameters::const_iterator pi;
    for (pi = p._parameters.begin(); pi != p._parameters.end(); ++pi) {
      new_subst.erase(*pi);
    }
    anything_changed = copy_substitute_decl(rep, new_subst, global_scope);
  } else {
    anything_changed = copy_substitute_decl(rep, subst, global_scope);
  }

  if (!anything_changed && rep->_parent_scope == _parent_scope) {
    delete rep;
    rep = (CPPScope *)this;
  }
  ((CPPScope *)this)->_subst_decl_recursive_protect = false;

  return rep;
}

////////////////////////////////////////////////////////////////////
//     Function: CPPScope::find_type
//       Access: Public
//  Description:
////////////////////////////////////////////////////////////////////
CPPType *CPPScope::
find_type(const string &name, bool recurse) const {
  Types::const_iterator ti;
  ti = _types.find(name);
  if (ti != _types.end()) {
    return ti->second;
  }

  Using::const_iterator ui;
  for (ui = _using.begin(); ui != _using.end(); ++ui) {
    CPPType *type = (*ui)->find_type(name, false);
    if (type != NULL) {
      return type;
    }
  }

  if (_struct_type != NULL) {
    CPPStructType::Derivation::const_iterator di;
    for (di = _struct_type->_derivation.begin();
         di != _struct_type->_derivation.end();
         ++di) {
      CPPStructType *st = (*di)._base->as_struct_type();
      if (st != NULL) {
        CPPType *type = st->_scope->find_type(name, false);
        if (type != NULL) {
          return type;
        }
      }
    }
  }

  if (recurse && _parent_scope != NULL) {
    return _parent_scope->find_type(name);
  }

  return NULL;
}

////////////////////////////////////////////////////////////////////
//     Function: CPPScope::find_type
//       Access: Public
//  Description:
////////////////////////////////////////////////////////////////////
CPPType *CPPScope::
find_type(const string &name, CPPDeclaration::SubstDecl &subst,
          CPPScope *global_scope, bool recurse) const {
  Types::const_iterator ti;
  ti = _types.find(name);
  if (ti != _types.end()) {
    CPPScope *current_scope = (CPPScope *)this;
    return (*ti).second->substitute_decl
      (subst, current_scope, global_scope)->as_type();
  }

  Using::const_iterator ui;
  for (ui = _using.begin(); ui != _using.end(); ++ui) {
    CPPType *type = (*ui)->find_type(name, subst, global_scope, false);
    if (type != NULL) {
      return type;
    }
  }

  if (_struct_type != NULL) {
    CPPStructType::Derivation::const_iterator di;
    for (di = _struct_type->_derivation.begin();
         di != _struct_type->_derivation.end();
         ++di) {
      CPPStructType *st = (*di)._base->as_struct_type();
      if (st != NULL) {
        CPPType *type = st->_scope->find_type(name, subst, global_scope,
                                              false);
        if (type != NULL) {
          return type;
        }
      }
    }
  }

  if (recurse && _parent_scope != NULL) {
    return _parent_scope->find_type(name, subst, global_scope);
  }

  return NULL;
}

////////////////////////////////////////////////////////////////////
//     Function: CPPScope::find_scope
//       Access: Public
//  Description:
////////////////////////////////////////////////////////////////////
CPPScope *CPPScope::
find_scope(const string &name, bool recurse) const {
  Namespaces::const_iterator ni = _namespaces.find(name);
  if (ni != _namespaces.end()) {
    return (*ni).second->get_scope();
  }

  CPPType *type = (CPPType *)NULL;

  Types::const_iterator ti;
  ti = _types.find(name);
  if (ti != _types.end()) {
    type = (*ti).second;
    // Resolve if this is a typedef.
    while (type->as_typedef_type() != (CPPTypedefType *)NULL) {
      type = type->as_typedef_type()->_type;
    }

  } else if (_struct_type != NULL) {
    CPPStructType::Derivation::const_iterator di;
    for (di = _struct_type->_derivation.begin();
         di != _struct_type->_derivation.end();
         ++di) {
      CPPStructType *st = (*di)._base->as_struct_type();
      if (st != NULL) {
        type = st->_scope->find_type(name, false);
      }
    }
  }

  if (type != NULL) {
    CPPStructType *st = type->as_struct_type();
    if (st != NULL) {
      return st->_scope;
    }
  }

  Using::const_iterator ui;
  for (ui = _using.begin(); ui != _using.end(); ++ui) {
    CPPScope *scope = (*ui)->find_scope(name, false);
    if (scope != NULL) {
      return scope;
    }
  }

  if (recurse && _parent_scope != NULL) {
    return _parent_scope->find_scope(name);
  }

  return (CPPScope *)NULL;
}

////////////////////////////////////////////////////////////////////
//     Function: CPPScope::find_scope
//       Access: Public
//  Description:
////////////////////////////////////////////////////////////////////
CPPScope *CPPScope::
find_scope(const string &name, CPPDeclaration::SubstDecl &subst,
           CPPScope *global_scope, bool recurse) const {
  CPPType *type = find_type(name, subst, global_scope, recurse);
  if (type == NULL) {
    return NULL;
  }

  // Resolve this if it is a typedef.
  while (type->get_subtype() == CPPDeclaration::ST_typedef) {
    type = type->as_typedef_type()->_type;
  }

  CPPStructType *st = type->as_struct_type();
  if (st == NULL) {
    return NULL;
  }

  return st->_scope;
}

////////////////////////////////////////////////////////////////////
//     Function: CPPScope::find_symbol
//       Access: Public
//  Description:
////////////////////////////////////////////////////////////////////
CPPDeclaration *CPPScope::
find_symbol(const string &name, bool recurse) const {
  if (_struct_type != NULL && name == get_simple_name()) {
    return _struct_type;
  }

  Types::const_iterator ti;
  ti = _types.find(name);
  if (ti != _types.end()) {
    return (*ti).second;
  }

  Variables::const_iterator vi;
  vi = _variables.find(name);
  if (vi != _variables.end()) {
    return (*vi).second;
  }

  vi = _enum_values.find(name);
  if (vi != _enum_values.end()) {
    return (*vi).second;
  }

  Functions::const_iterator fi;
  fi = _functions.find(name);
  if (fi != _functions.end()) {
    return (*fi).second;
  }

  Using::const_iterator ui;
  for (ui = _using.begin(); ui != _using.end(); ++ui) {
    CPPDeclaration *decl = (*ui)->find_symbol(name, false);
    if (decl != NULL) {
      return decl;
    }
  }

  if (_struct_type != NULL) {
    CPPStructType::Derivation::const_iterator di;
    for (di = _struct_type->_derivation.begin();
         di != _struct_type->_derivation.end();
         ++di) {
      CPPStructType *st = (*di)._base->as_struct_type();
      if (st != NULL) {
        CPPDeclaration *decl = st->_scope->find_symbol(name, false);
        if (decl != NULL) {
          return decl;
        }
      }
    }
  }

  if (recurse && _parent_scope != NULL) {
    return _parent_scope->find_symbol(name);
  }

  return NULL;
}

////////////////////////////////////////////////////////////////////
//     Function: CPPScope::find_template
//       Access: Public
//  Description:
////////////////////////////////////////////////////////////////////
CPPDeclaration *CPPScope::
find_template(const string &name, bool recurse) const {
  Templates::const_iterator ti;
  ti = _templates.find(name);
  if (ti != _templates.end()) {
    return (*ti).second;
  }

  Using::const_iterator ui;
  for (ui = _using.begin(); ui != _using.end(); ++ui) {
    CPPDeclaration *decl = (*ui)->find_template(name, false);
    if (decl != NULL) {
      return decl;
    }
  }

  if (_struct_type != NULL) {
    CPPStructType::Derivation::const_iterator di;
    for (di = _struct_type->_derivation.begin();
         di != _struct_type->_derivation.end();
         ++di) {
      CPPStructType *st = (*di)._base->as_struct_type();
      if (st != NULL) {
        CPPDeclaration *decl = st->_scope->find_template(name, false);
        if (decl != NULL) {
          return decl;
        }
      }
    }
  }

  if (recurse && _parent_scope != NULL) {
    return _parent_scope->find_template(name);
  }

  return NULL;
}

////////////////////////////////////////////////////////////////////
//     Function: CPPScope::get_simple_name
//       Access: Public, Virtual
//  Description:
////////////////////////////////////////////////////////////////////
string CPPScope::
get_simple_name() const {
  /*
  if (_struct_type != (CPPStructType *)NULL) {
    return _struct_type->get_simple_name();
  }
  */
  return _name.get_name();
}

////////////////////////////////////////////////////////////////////
//     Function: CPPScope::get_local_name
//       Access: Public, Virtual
//  Description:
////////////////////////////////////////////////////////////////////
string CPPScope::
get_local_name(CPPScope *scope) const {
  /*
  if (_struct_type != (CPPStructType *)NULL) {
    return _struct_type->get_local_name(scope);
  }
  */

  if (scope != NULL && _parent_scope != NULL/* && _parent_scope != scope*/) {
    string parent_scope_name = _parent_scope->get_local_name(scope);
    if (parent_scope_name.empty()) {
      return _name.get_name_with_templ();
    } else {
      return parent_scope_name + "::" +
        _name.get_name_with_templ();
    }
  } else {
    return _name.get_name_with_templ();
  }
}

////////////////////////////////////////////////////////////////////
//     Function: CPPScope::get_fully_scoped_name
//       Access: Public, Virtual
//  Description:
////////////////////////////////////////////////////////////////////
string CPPScope::
get_fully_scoped_name() const {
  /*
  if (_struct_type != (CPPStructType *)NULL) {
    return _struct_type->get_fully_scoped_name();
  }
  */

  if (_parent_scope != NULL) {
    return _parent_scope->get_fully_scoped_name() + "::" +
      _name.get_name_with_templ();
  } else {
    return _name.get_name_with_templ();
  }
}

////////////////////////////////////////////////////////////////////
//     Function: CPPScope::output
//       Access: Public, Virtual
//  Description:
////////////////////////////////////////////////////////////////////
void CPPScope::
output(ostream &out, CPPScope *scope) const {
  //  out << get_local_name(scope);
  if (_parent_scope != NULL && _parent_scope != scope) {
    _parent_scope->output(out, scope);
    out << "::";
  }
  out << _name;
}

////////////////////////////////////////////////////////////////////
//     Function: CPPScope::write
//       Access: Public
//  Description:
////////////////////////////////////////////////////////////////////
void CPPScope::
write(ostream &out, int indent_level, CPPScope *scope) const {
  CPPVisibility vis = V_unknown;
  Declarations::const_iterator di;
  for (di = _declarations.begin(); di != _declarations.end(); ++di) {
    CPPDeclaration *cd = (*di);
    if (cd->_vis != vis && indent_level > 0) {
      vis = cd->_vis;
      indent(out, indent_level - 2) << vis << ":\n";
    }
    bool complete = false;

    if (cd->as_type() != NULL || cd->as_namespace() != NULL) {
      complete = true;
    }

    indent(out, indent_level);
    cd->output(out, indent_level, scope, complete);
    out << ";\n";
  }
}

////////////////////////////////////////////////////////////////////
//     Function: CPPScope::get_template_scope
//       Access: Public
//  Description: Returns the nearest ancestor of this scope that is a
//               template scope, or NULL if the scope is fully
//               specified.
////////////////////////////////////////////////////////////////////
CPPTemplateScope *CPPScope::
get_template_scope() {
  if (as_template_scope()) {
    return as_template_scope();
  }
  if (_parent_scope != NULL) {
    return _parent_scope->get_template_scope();
  }
  return (CPPTemplateScope *)NULL;
}

////////////////////////////////////////////////////////////////////
//     Function: CPPScope::as_template_scope
//       Access: Public, Virtual
//  Description:
////////////////////////////////////////////////////////////////////
CPPTemplateScope *CPPScope::
as_template_scope() {
  return (CPPTemplateScope *)NULL;
}

////////////////////////////////////////////////////////////////////
//     Function: CPPScope::copy_substitute_decl
//       Access: Private
//  Description: This is in support of both substitute_decl() and
//               instantiate().  It's similar in purpose to
//               substitute_decl(), but this function assumes the
//               caller has already created a new, empty scope.  All
//               of the declarations in this scope are copied to the
//               new scope, filtering through the subst decl.
//
//               The return value is true if the scope is changed,
//               false if it is not.
////////////////////////////////////////////////////////////////////
bool CPPScope::
copy_substitute_decl(CPPScope *to_scope, CPPDeclaration::SubstDecl &subst,
                     CPPScope *global_scope) const {
  bool anything_changed = false;

  if (_struct_type != NULL) {
    CPPScope *native_scope = (CPPScope *)NULL;
    if (_struct_type->_ident != (CPPIdentifier *)NULL) {
      native_scope = _struct_type->_ident->_native_scope;
    }
    to_scope->_struct_type =
      new CPPStructType(_struct_type->_type,
                        new CPPIdentifier(to_scope->_name, _struct_type->_file),
                        native_scope, to_scope, _struct_type->_file);
    to_scope->_struct_type->_incomplete = false;

    // Copy the derivation to the new type.
    CPPStructType::Derivation::const_iterator di;
    for (di = _struct_type->_derivation.begin();
         di != _struct_type->_derivation.end();
         ++di) {
      CPPStructType::Base b = (*di);
      b._base =
        (*di)._base->substitute_decl(subst, to_scope, global_scope)->as_type();
      to_scope->_struct_type->_derivation.push_back(b);
      if (b._base != (*di)._base) {
        anything_changed = true;
      }
    }
  }

  Declarations::const_iterator di;
  for (di = _declarations.begin(); di != _declarations.end(); ++di) {
    CPPDeclaration *decl =
      (*di)->substitute_decl(subst, to_scope, global_scope);
    to_scope->_declarations.push_back(decl);
    if (decl != (*di)) {
      anything_changed = true;
    }
  }

  ExtensionTypes::const_iterator ei;
  for (ei = _structs.begin(); ei != _structs.end(); ++ei) {
    string name = (*ei).first;
    CPPType *source_type = (*ei).second;
    CPPDeclaration *decl =
      source_type->substitute_decl(subst, to_scope, global_scope);
    assert(decl != NULL);
    CPPType *new_type = decl->as_type();
    assert(new_type != NULL);
    to_scope->_structs.insert(ExtensionTypes::value_type(name, new_type));
    if (new_type != source_type) {
      anything_changed = true;
    }
  }
  for (ei = _classes.begin(); ei != _classes.end(); ++ei) {
    string name = (*ei).first;
    CPPType *source_type = (*ei).second;
    CPPDeclaration *decl =
      source_type->substitute_decl(subst, to_scope, global_scope);
    assert(decl != NULL);
    CPPType *new_type = decl->as_type();
    assert(new_type != NULL);
    to_scope->_classes.insert(ExtensionTypes::value_type(name, new_type));
    if (new_type != source_type) {
      anything_changed = true;
    }
  }
  for (ei = _unions.begin(); ei != _unions.end(); ++ei) {
    string name = (*ei).first;
    CPPType *source_type = (*ei).second;
    CPPDeclaration *decl =
      source_type->substitute_decl(subst, to_scope, global_scope);
    assert(decl != NULL);
    CPPType *new_type = decl->as_type();
    assert(new_type != NULL);
    to_scope->_unions.insert(ExtensionTypes::value_type(name, new_type));
    if (new_type != source_type) {
      anything_changed = true;
    }
  }
  for (ei = _enums.begin(); ei != _enums.end(); ++ei) {
    string name = (*ei).first;
    CPPType *source_type = (*ei).second;
    CPPDeclaration *decl =
      source_type->substitute_decl(subst, to_scope, global_scope);
    assert(decl != NULL);
    CPPType *new_type = decl->as_type();
    assert(new_type != NULL);
    to_scope->_enums.insert(ExtensionTypes::value_type(name, new_type));
    if (new_type != source_type) {
      anything_changed = true;
    }
  }
  Functions::const_iterator fi;
  for (fi = _functions.begin(); fi != _functions.end(); ++fi) {
    CPPFunctionGroup *fgroup = (*fi).second;
    string name = fgroup->_name;

    CPPFunctionGroup *&to_fgroup = to_scope->_functions[name];
    if (to_fgroup == (CPPFunctionGroup *)NULL) {
      to_fgroup = new CPPFunctionGroup(name);
    }

    CPPFunctionGroup::Instances::const_iterator ii;
    for (ii = fgroup->_instances.begin();
         ii != fgroup->_instances.end();
         ++ii) {
      CPPInstance *inst =
        (*ii)->substitute_decl(subst, to_scope, global_scope)->as_instance();
      to_fgroup->_instances.push_back(inst);
      if (inst != (*ii)) {
        anything_changed = true;
      }
    }
  }

  Types::const_iterator ti;
  for (ti = _types.begin(); ti != _types.end(); ++ti) {
    CPPType *td =
      (*ti).second->substitute_decl(subst, to_scope, global_scope)->as_type();
    to_scope->_types.insert(Types::value_type((*ti).first, td));
    if (td != (*ti).second) {
      anything_changed = true;
    }
  }
  Variables::const_iterator vi;
  for (vi = _variables.begin(); vi != _variables.end(); ++vi) {
    CPPInstance *inst =
      (*vi).second->substitute_decl(subst, to_scope, global_scope)->as_instance();
    to_scope->_variables.insert(Variables::value_type((*vi).first, inst));
    if (inst != (*vi).second) {
      // I don't know if this _native_scope assignment is right, but it
      // fixes some issues with variables in instantiated template scopes
      // being printed out with an uninstantiated template scope prefix. ~rdb
      inst->_ident->_native_scope = to_scope;
      anything_changed = true;
    }
  }

  for (vi = _enum_values.begin(); vi != _enum_values.end(); ++vi) {
    CPPInstance *inst =
      (*vi).second->substitute_decl(subst, to_scope, global_scope)->as_instance();
    to_scope->_enum_values.insert(Variables::value_type((*vi).first, inst));
    if (inst != (*vi).second) {
      anything_changed = true;
    }
  }

  Templates::const_iterator tmi;
  for (tmi = _templates.begin(); tmi != _templates.end(); ++tmi) {
    CPPDeclaration *decl =
      (*tmi).second->substitute_decl(subst, to_scope, global_scope);
    to_scope->_templates.insert(Templates::value_type((*tmi).first, decl));
    if (decl != (*tmi).second) {
      anything_changed = true;
    }
  }

  return anything_changed;
}


////////////////////////////////////////////////////////////////////
//     Function: CPPScope::handle_declaration
//       Access: Private
//  Description: Does the right thing with a newly given declaration:
//               adds it to the typedef list, or variables or
//               functions, or whatever.
////////////////////////////////////////////////////////////////////
void CPPScope::
handle_declaration(CPPDeclaration *decl, CPPScope *global_scope,
                   CPPPreprocessor *error_sink) {
  CPPTypedefType *def = decl->as_typedef_type();
  if (def != NULL) {
    string name = def->get_simple_name();

    pair<Types::iterator, bool> result =
      _types.insert(Types::value_type(name, def));

    if (!result.second) {
      CPPType *other_type = result.first->second;
      CPPTypedefType *other_td = other_type->as_typedef_type();

      // We don't do redefinitions of typedefs.  But we don't complain
      // as long as this is actually a typedef to the previous definition.
      if (other_type != def->_type &&
          (other_td == NULL || other_td->_type != def->_type)) {

        if (error_sink != NULL) {
          ostringstream errstr;
          def->output(errstr, 0, NULL, false);
          errstr << " has conflicting declaration as ";
          other_type->output(errstr, 0, NULL, true);
          error_sink->error(errstr.str());
        }
      }
    } else {
      _types[name] = def;
    }

    CPPExtensionType *et = def->_type->as_extension_type();
    if (et != NULL) {
      define_extension_type(et, error_sink);
    }

    return;
  }

  CPPTypeDeclaration *typedecl = decl->as_type_declaration();
  if (typedecl != (CPPTypeDeclaration *)NULL) {
    CPPExtensionType *et = typedecl->_type->as_extension_type();
    if (et != NULL) {
      define_extension_type(et, error_sink);
    }
    return;
  }

  CPPInstance *inst = decl->as_instance();
  if (inst != NULL) {
    inst->check_for_constructor(this, global_scope);

    if (inst->_ident != NULL) {
      // Not sure if this is the best place to assign this.  However,
      // this fixes a bug with variables in expressions not having
      // the proper scoping prefix. ~rdb
      inst->_ident->_native_scope = this;
    }

    string name = inst->get_simple_name();
    if (!name.empty() && inst->get_scope(this, global_scope) == this) {
      if (inst->_type->as_function_type()) {
        // This is a function declaration; hence it gets added to
        // the _functions member.  But we must be careful to share
        // common-named functions.

        CPPFunctionGroup *fgroup;
        Functions::const_iterator fi;
        fi = _functions.find(name);
        if (fi == _functions.end()) {
          fgroup = new CPPFunctionGroup(name);
          _functions.insert(Functions::value_type(name, fgroup));
        } else {
          fgroup = (*fi).second;
        }
        fgroup->_instances.push_back(inst);

      } else {
        // This is not a function declaration; hence it gets added
        // to the _variables member.
        _variables[name] = inst;
      }

      if (inst->is_template()) {
        // Don't add a new template definition if we already had one
        // by the same name in another scope.

        if (find_template(name) == NULL) {
          _templates.insert(Templates::value_type(name, inst));
        }

        /*
        if (inst->_type->as_function_type() == NULL ||
            (inst->_type->as_function_type()->_flags &
             CPPFunctionType::F_constructor) == 0) {
          _templates.insert(Templates::value_type(name, inst));
        }
        */
      }
    }
    return;
  }

  CPPExtensionType *et = decl->as_extension_type();
  if (et != NULL) {
    define_extension_type(et, error_sink);
  }
}
