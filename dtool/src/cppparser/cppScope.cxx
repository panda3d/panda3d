/**
 * PANDA 3D SOFTWARE
 * Copyright (c) Carnegie Mellon University.  All rights reserved.
 *
 * All use of this software is subject to the terms of the revised BSD
 * license.  You should have received a copy of this license along
 * with this source code in a file named "LICENSE."
 *
 * @file cppScope.cxx
 * @author drose
 * @date 1999-10-21
 */

#include "cppScope.h"
#include "cppParser.h"
#include "cppDeclaration.h"
#include "cppNamespace.h"
#include "cppTypedefType.h"
#include "cppTypeDeclaration.h"
#include "cppExtensionType.h"
#include "cppEnumType.h"
#include "cppInstance.h"
#include "cppInstanceIdentifier.h"
#include "cppIdentifier.h"
#include "cppStructType.h"
#include "cppFunctionGroup.h"
#include "cppPreprocessor.h"
#include "cppTemplateScope.h"
#include "cppClassTemplateParameter.h"
#include "cppFunctionType.h"
#include "cppConstType.h"
#include "cppUsing.h"
#include "cppBisonDefs.h"
#include "indent.h"

using std::ostream;
using std::ostringstream;
using std::pair;
using std::string;

/**
 *
 */
CPPScope::
CPPScope(CPPScope *parent_scope,
         const CPPNameComponent &name, CPPVisibility starting_vis) :
  _name(name),
  _parent_scope(parent_scope),
  _current_vis(starting_vis)
{
  _struct_type = nullptr;
  _is_fully_specified = false;
  _fully_specified_known = false;
  _is_fully_specified_recursive_protect = false;
  _subst_decl_recursive_protect = false;
}

/**
 *
 */
CPPScope::
~CPPScope() {
}

/**
 * Sets the struct or class that owns this scope.  This should only be done
 * once, when the scope and its associated struct are created.  It's provided
 * so the scope can check the struct's ancestry for inherited symbols.
 */
void CPPScope::
set_struct_type(CPPStructType *struct_type) {
  _struct_type = struct_type;
}

/**
 * Returns the class or struct that defines this scope, if any.
 */
CPPStructType *CPPScope::
get_struct_type() const {
  return _struct_type;
}

/**
 * Returns the parent scope of this scope, if any.
 */
CPPScope *CPPScope::
get_parent_scope() const {
  return _parent_scope;
}

/**
 *
 */
void CPPScope::
set_current_vis(CPPVisibility current_vis) {
  _current_vis = current_vis;
}

/**
 *
 */
CPPVisibility CPPScope::
get_current_vis() const {
  return _current_vis;
}

/**
 *
 */
void CPPScope::
add_declaration(CPPDeclaration *decl, CPPScope *global_scope,
                CPPPreprocessor *preprocessor, const cppyyltype &pos) {
  decl->_vis = _current_vis;

  // Get the recent comments from the preprocessor.  These are the comments
  // that appeared preceding this particular declaration; they might be
  // relevant to the declaration.

  if (decl->_leading_comment == nullptr) {
    decl->_leading_comment =
      preprocessor->get_comment_before(pos.first_line, pos.file);
  }

  _declarations.push_back(decl);

  handle_declaration(decl, global_scope, preprocessor);
}

/**
 *
 */
void CPPScope::
add_enum_value(CPPInstance *inst) {
  inst->_vis = _current_vis;

  string name = inst->get_simple_name();
  if (!name.empty()) {
    _enum_values[name] = inst;
  }
}

/**
 *
 */
void CPPScope::
define_typedef_type(CPPTypedefType *type, CPPPreprocessor *error_sink) {
  string name = type->get_simple_name();

  pair<Types::iterator, bool> result =
    _types.insert(Types::value_type(name, type));

  if (!result.second) {
    CPPType *other_type = result.first->second;
    CPPTypedefType *other_td = other_type->as_typedef_type();

    // We don't do redefinitions of typedefs.  But we don't complain as long
    // as this is actually a typedef to the previous definition.
    if (other_type != type->_type &&
        (other_td == nullptr || !other_td->_type->is_equivalent(*type->_type))) {

      if (error_sink != nullptr) {
        ostringstream errstr;
        type->output(errstr, 0, nullptr, false);
        errstr << " has conflicting declaration as ";
        other_type->output(errstr, 0, nullptr, true);
        error_sink->error(errstr.str(), type->_ident->_loc);
        if (other_td != nullptr && other_td->_ident != nullptr) {
          error_sink->error("previous definition is here",
                            other_td->_ident->_loc);
        }
      }
    }
  } else {
    _types[name] = type;
  }

  // This might be a templated "using" definition.
  if (type->is_template()) {
    CPPTemplateScope *scope = type->get_template_scope();
    if (scope->_parameters._parameters.size() == 0) {
      return;
    }

    string simple_name = type->get_simple_name();

    pair<Templates::iterator, bool> result =
      _templates.insert(Templates::value_type(simple_name, type));

    if (!result.second) {
      // The template was not inserted because we already had a template
      // definition with the given name.  If the previous definition was
      // incomplete, replace it.
      CPPDeclaration *old_templ = (*result.first).second;
      CPPType *old_templ_type = old_templ->as_type();
      if (old_templ_type == nullptr || old_templ_type->is_incomplete()) {
        // The previous template definition was incomplete, maybe a forward
        // reference; replace it with the good one.
        (*result.first).second = type;
      }
    }
  }
}

/**
 *
 */
void CPPScope::
define_extension_type(CPPExtensionType *type, CPPPreprocessor *error_sink) {
  assert(type != nullptr);
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
  case CPPExtensionType::T_enum_struct:
  case CPPExtensionType::T_enum_class:
    _enums[name] = type;
    break;
  }

  // Create an implicit typedef for the extension.  CPPTypedefType *td = new
  // CPPTypedefType(type, name);
  pair<Types::iterator, bool> result =
    _types.insert(Types::value_type(name, type));

  if (!result.second) {
    // There's already a typedef for this extension.  This one overrides it
    // only if the other is a forward declaration.
    CPPType *other_type = (*result.first).second;

    if (other_type->get_subtype() == CPPDeclaration::ST_extension) {
      CPPExtensionType *other_ext = other_type->as_extension_type();

      if (other_ext->_type != type->_type) {
        if (error_sink != nullptr) {
          ostringstream errstr;
          errstr << type->_type << " " << type->get_fully_scoped_name()
                 << " was previously declared as " << other_ext->_type;
          error_sink->error(errstr.str(), type->_ident->_loc);

          if (other_ext->_ident != nullptr) {
            error_sink->error("previous declaration is here",
                              other_ext->_ident->_loc);
          }
        }
      }
      (*result.first).second = type;

    } else {
      CPPTypedefType *other_td = other_type->as_typedef_type();

      // Error out if the declaration is different than the previous one.
      if (other_type != type &&
          (other_td == nullptr || other_td->_type != type)) {

        if (error_sink != nullptr) {
          ostringstream errstr;
          if (!cppparser_output_class_keyword) {
            errstr << type->_type << " ";
          }
          type->output(errstr, 0, nullptr, false);
          errstr << " has conflicting definition as ";
          other_type->output(errstr, 0, nullptr, true);
          error_sink->error(errstr.str(), type->_ident->_loc);

          CPPExtensionType *other_ext = other_type->as_extension_type();
          if (other_ext != nullptr && other_ext->_ident != nullptr) {
            error_sink->error("previous definition is here",
                              other_ext->_ident->_loc);
          }
        }
      }
    }
  }

  if (type->is_template()) {
    CPPTemplateScope *scope = type->get_template_scope();
    if (scope->_parameters._parameters.size() == 0) {
      return;
    }

    string simple_name = type->get_simple_name();

    pair<Templates::iterator, bool> result =
      _templates.insert(Templates::value_type(simple_name, type));

    if (!result.second) {
      // The template was not inserted because we already had a template
      // definition with the given name.  If the previous definition was
      // incomplete, replace it.
      CPPDeclaration *old_templ = (*result.first).second;
      CPPType *old_templ_type = old_templ->as_type();
      if (old_templ_type == nullptr || old_templ_type->is_incomplete()) {
        // The previous template definition was incomplete, maybe a forward
        // reference; replace it with the good one.
        (*result.first).second = type;
      }
    }
  }
}

/**
 *
 */
void CPPScope::
define_namespace(CPPNamespace *ns) {
  string name = ns->get_simple_name();

  _namespaces[name] = ns;

  if (ns->_is_inline) {
    // Add an implicit using declaration for an inline namespace.
    _using.insert(ns->get_scope());
  }
}

/**
 *
 */
void CPPScope::
add_using(CPPUsing *using_decl, CPPScope *global_scope,
          CPPPreprocessor *error_sink) {
  if (using_decl->_full_namespace) {
    CPPScope *scope =
      using_decl->_ident->find_scope(this, global_scope);
    if (scope != nullptr) {
      _using.insert(scope);
    } else {
      if (error_sink != nullptr) {
        error_sink->warning("Attempt to use undefined namespace: " + using_decl->_ident->get_fully_scoped_name(), using_decl->_ident->_loc);
      }
    }
  } else {
    CPPDeclaration *decl = using_decl->_ident->find_symbol(this, global_scope);
    if (decl != nullptr) {
      handle_declaration(decl, global_scope, error_sink);
    } else {
      if (error_sink != nullptr) {
        error_sink->warning("Attempt to use unknown symbol: " + using_decl->_ident->get_fully_scoped_name(), using_decl->_ident->_loc);
      }
    }
  }
}

/**
 * Returns true if this declaration is an actual, factual declaration, or
 * false if some part of the declaration depends on a template parameter which
 * has not yet been instantiated.
 */
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

  if (_parent_scope != nullptr && !_parent_scope->is_fully_specified()) {
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

/**
 *
 */
CPPScope *CPPScope::
instantiate(const CPPTemplateParameterList *actual_params,
            CPPScope *current_scope, CPPScope *global_scope,
            CPPPreprocessor *error_sink) const {
  CPPScope *this_scope = (CPPScope *)this;

  if (_parent_scope == nullptr ||
      _parent_scope->as_template_scope() == nullptr) {
    if (error_sink != nullptr) {
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
    // We've already instantiated this scope with these parameters.  Return
    // that.
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
    // scope = new CPPScope(current_scope, name, V_public);
    scope = new CPPScope(_parent_scope, name, V_public);
    copy_substitute_decl(scope, subst, global_scope);

    // Also define any new template parameter types, in case we "instantiated"
    // this scope with another template parameter.
    CPPTemplateParameterList::Parameters::const_iterator pi;
    for (pi = actual_params->_parameters.begin();
         pi != actual_params->_parameters.end();
         ++pi) {
      CPPDeclaration *decl = (*pi);
      CPPClassTemplateParameter *ctp = decl->as_class_template_parameter();
      if (ctp != nullptr) {
        // CPPTypedefType *td = new CPPTypedefType(ctp, ctp->_ident);
        // scope->_typedefs.insert(Typedefs::value_type
        // (ctp->_ident->get_local_name(), td));
        scope->_types.insert(Types::value_type
                             (ctp->_ident->get_local_name(),
                              ctp));
      }
    }
  }

  // Finally, record this particular instantiation for future reference, so we
  // don't have to do this again.
  ((CPPScope *)this)->_instantiations.insert(Instantiations::value_type(actual_params, scope));

  return scope;
}

/**
 *
 */
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

  if (_parent_scope != nullptr &&
      _parent_scope->as_template_scope() != nullptr) {
    // If the parent of this scope is a template scope--e.g.  this scope has
    // template parameters--then we must first remove any of the template
    // parameters from the subst list.  These will later get substituted
    // properly during instantiation.
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

/**
 *
 */
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
    if (type != nullptr) {
      return type;
    }
  }

  if (_struct_type != nullptr) {
    CPPStructType::Derivation::const_iterator di;
    for (di = _struct_type->_derivation.begin();
         di != _struct_type->_derivation.end();
         ++di) {
      CPPStructType *st = (*di)._base->as_struct_type();
      if (st != nullptr) {
        CPPType *type = st->_scope->find_type(name, false);
        if (type != nullptr) {
          return type;
        }
      }
    }
  }

  if (recurse && _parent_scope != nullptr) {
    return _parent_scope->find_type(name);
  }

  return nullptr;
}

/**
 *
 */
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
    if (type != nullptr) {
      return type;
    }
  }

  if (_struct_type != nullptr) {
    CPPStructType::Derivation::const_iterator di;
    for (di = _struct_type->_derivation.begin();
         di != _struct_type->_derivation.end();
         ++di) {
      CPPStructType *st = (*di)._base->as_struct_type();
      if (st != nullptr) {
        CPPType *type = st->_scope->find_type(name, subst, global_scope,
                                              false);
        if (type != nullptr) {
          return type;
        }
      }
    }
  }

  if (recurse && _parent_scope != nullptr) {
    return _parent_scope->find_type(name, subst, global_scope);
  }

  return nullptr;
}

/**
 *
 */
CPPScope *CPPScope::
find_scope(const string &name, CPPScope *global_scope, bool recurse) const {
  Namespaces::const_iterator ni = _namespaces.find(name);
  if (ni != _namespaces.end()) {
    return (*ni).second->get_scope();
  }

  CPPType *type = nullptr;

  Types::const_iterator ti;
  ti = _types.find(name);
  if (ti != _types.end()) {
    type = (*ti).second;
    // Resolve if this is a typedef or const, or a TBD type.
    while (type->get_subtype() == CPPDeclaration::ST_const ||
           type->get_subtype() == CPPDeclaration::ST_typedef ||
           type->get_subtype() == CPPDeclaration::ST_tbd) {
      if (type->as_typedef_type() != nullptr) {
        type = type->as_typedef_type()->_type;
      } else if (type->as_const_type() != nullptr) {
        type = type->as_const_type()->_wrapped_around;
      } else {
        CPPType *new_type = type->resolve_type((CPPScope *)this, global_scope);
        if (new_type != type) {
          type = new_type;
        } else {
          break;
        }
      }
    }

  } else if (_struct_type != nullptr) {
    CPPStructType::Derivation::const_iterator di;
    for (di = _struct_type->_derivation.begin();
         di != _struct_type->_derivation.end();
         ++di) {
      CPPStructType *st = (*di)._base->as_struct_type();
      if (st != nullptr) {
        type = st->_scope->find_type(name, false);
      }
    }
  }

  if (type != nullptr) {
    CPPStructType *st = type->as_struct_type();
    if (st != nullptr) {
      return st->_scope;
    }

    CPPEnumType *et = type->as_enum_type();
    if (et != nullptr) {
      return et->_scope;
    }
  }

  Using::const_iterator ui;
  for (ui = _using.begin(); ui != _using.end(); ++ui) {
    CPPScope *scope = (*ui)->find_scope(name, global_scope, false);
    if (scope != nullptr) {
      return scope;
    }
  }

  if (recurse && _parent_scope != nullptr) {
    return _parent_scope->find_scope(name, global_scope);
  }

  return nullptr;
}

/**
 *
 */
CPPScope *CPPScope::
find_scope(const string &name, CPPDeclaration::SubstDecl &subst,
           CPPScope *global_scope, bool recurse) const {
  CPPType *type = find_type(name, subst, global_scope, recurse);
  if (type == nullptr) {
    return nullptr;
  }

  // Resolve if this is a typedef or const.
  while (type->get_subtype() == CPPDeclaration::ST_const ||
         type->get_subtype() == CPPDeclaration::ST_typedef) {
    if (type->as_typedef_type() != nullptr) {
      type = type->as_typedef_type()->_type;
    } else {
      type = type->as_const_type()->_wrapped_around;
    }
  }

  CPPStructType *st = type->as_struct_type();
  if (st != nullptr) {
    return st->_scope;
  }

  CPPEnumType *et = type->as_enum_type();
  if (et != nullptr) {
    return et->_scope;
  }

  return nullptr;
}

/**
 *
 */
CPPDeclaration *CPPScope::
find_symbol(const string &name, bool recurse) const {
  if (_struct_type != nullptr && name == get_simple_name()) {
    return _struct_type;
  }

  Functions::const_iterator fi;
  fi = _functions.find(name);
  if (fi != _functions.end()) {
    return (*fi).second;
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

  Using::const_iterator ui;
  for (ui = _using.begin(); ui != _using.end(); ++ui) {
    CPPDeclaration *decl = (*ui)->find_symbol(name, false);
    if (decl != nullptr) {
      return decl;
    }
  }

  if (_struct_type != nullptr) {
    CPPStructType::Derivation::const_iterator di;
    for (di = _struct_type->_derivation.begin();
         di != _struct_type->_derivation.end();
         ++di) {
      CPPStructType *st = (*di)._base->as_struct_type();
      if (st != nullptr) {
        CPPDeclaration *decl = st->_scope->find_symbol(name, false);
        if (decl != nullptr) {
          return decl;
        }
      }
    }
  }

  if (recurse && _parent_scope != nullptr) {
    return _parent_scope->find_symbol(name);
  }

  return nullptr;
}

/**
 *
 */
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
    if (decl != nullptr) {
      return decl;
    }
  }

  if (_struct_type != nullptr) {
    CPPStructType::Derivation::const_iterator di;
    for (di = _struct_type->_derivation.begin();
         di != _struct_type->_derivation.end();
         ++di) {
      CPPStructType *st = (*di)._base->as_struct_type();
      if (st != nullptr) {
        CPPDeclaration *decl = st->_scope->find_template(name, false);
        if (decl != nullptr) {
          return decl;
        }
      }
    }
  }

  if (recurse && _parent_scope != nullptr) {
    return _parent_scope->find_template(name);
  }

  return nullptr;
}

/**
 *
 */
string CPPScope::
get_simple_name() const {
  /*
  if (_struct_type != (CPPStructType *)NULL) {
    return _struct_type->get_simple_name();
  }
  */
  return _name.get_name();
}

/**
 *
 */
string CPPScope::
get_local_name(CPPScope *scope) const {
  /*
  if (_struct_type != (CPPStructType *)NULL) {
    return _struct_type->get_local_name(scope);
  }
  */

  if (scope != nullptr && _parent_scope != nullptr/* && _parent_scope != scope*/) {
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

/**
 *
 */
string CPPScope::
get_fully_scoped_name() const {
  /*
  if (_struct_type != (CPPStructType *)NULL) {
    return _struct_type->get_fully_scoped_name();
  }
  */

  if (_parent_scope != nullptr) {
    return _parent_scope->get_fully_scoped_name() + "::" +
      _name.get_name_with_templ();
  } else {
    return _name.get_name_with_templ();
  }
}

/**
 *
 */
void CPPScope::
output(ostream &out, CPPScope *scope) const {
  // out << get_local_name(scope);
  if (_parent_scope != nullptr && _parent_scope != scope) {
    _parent_scope->output(out, scope);
    out << "::";
  }
  out << _name;
}

/**
 *
 */
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

    if (cd->as_type() != nullptr || cd->as_namespace() != nullptr) {
      complete = true;
    }

    indent(out, indent_level);
    cd->output(out, indent_level, scope, complete);
    out << ";\n";
  }
}

/**
 * Returns the nearest ancestor of this scope that is a template scope, or
 * NULL if the scope is fully specified.
 */
CPPTemplateScope *CPPScope::
get_template_scope() {
  if (as_template_scope()) {
    return as_template_scope();
  }
  if (_parent_scope != nullptr) {
    return _parent_scope->get_template_scope();
  }
  return nullptr;
}

/**
 *
 */
CPPTemplateScope *CPPScope::
as_template_scope() {
  return nullptr;
}

/**
 * This is in support of both substitute_decl() and instantiate().  It's
 * similar in purpose to substitute_decl(), but this function assumes the
 * caller has already created a new, empty scope.  All of the declarations in
 * this scope are copied to the new scope, filtering through the subst decl.
 *
 * The return value is true if the scope is changed, false if it is not.
 */
bool CPPScope::
copy_substitute_decl(CPPScope *to_scope, CPPDeclaration::SubstDecl &subst,
                     CPPScope *global_scope) const {
  bool anything_changed = false;

  if (_struct_type != nullptr) {
    CPPScope *native_scope = nullptr;
    if (_struct_type->_ident != nullptr) {
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
    assert(decl != nullptr);
    CPPType *new_type = decl->as_type();
    assert(new_type != nullptr);
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
    assert(decl != nullptr);
    CPPType *new_type = decl->as_type();
    assert(new_type != nullptr);
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
    assert(decl != nullptr);
    CPPType *new_type = decl->as_type();
    assert(new_type != nullptr);
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
    assert(decl != nullptr);
    CPPType *new_type = decl->as_type();
    assert(new_type != nullptr);
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
    if (to_fgroup == nullptr) {
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


/**
 * Does the right thing with a newly given declaration: adds it to the typedef
 * list, or variables or functions, or whatever.
 */
void CPPScope::
handle_declaration(CPPDeclaration *decl, CPPScope *global_scope,
                   CPPPreprocessor *error_sink) {
  CPPTypedefType *def = decl->as_typedef_type();
  if (def != nullptr) {
    define_typedef_type(def, error_sink);

    CPPExtensionType *et = def->_type->as_extension_type();
    if (et != nullptr) {
      define_extension_type(et, error_sink);
    }
    return;
  }

  CPPTypeDeclaration *typedecl = decl->as_type_declaration();
  if (typedecl != nullptr) {
    CPPExtensionType *et = typedecl->_type->as_extension_type();
    if (et != nullptr) {
      define_extension_type(et, error_sink);
    }
    return;
  }

  CPPInstance *inst = decl->as_instance();
  if (inst != nullptr) {
    inst->check_for_constructor(this, global_scope);

    if (inst->_ident != nullptr) {
      // Not sure if this is the best place to assign this.  However, this
      // fixes a bug with variables in expressions not having the proper
      // scoping prefix.  ~rdb
      inst->_ident->_native_scope = this;
    }

    string name = inst->get_simple_name();
    if (!name.empty() && inst->get_scope(this, global_scope) == this) {
      if (inst->_type->as_function_type()) {
        // This is a function declaration; hence it gets added to the
        // _functions member.  But we must be careful to share common-named
        // functions.

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
        // This is not a function declaration; hence it gets added to the
        // _variables member.
        _variables[name] = inst;
      }

      if (inst->is_template()) {
        // Don't add a new template definition if we already had one by the
        // same name in another scope.

        if (find_template(name) == nullptr) {
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
  if (et != nullptr) {
    define_extension_type(et, error_sink);
  }
}
