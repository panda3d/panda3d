/**
 * PANDA 3D SOFTWARE
 * Copyright (c) Carnegie Mellon University.  All rights reserved.
 *
 * All use of this software is subject to the terms of the revised BSD
 * license.  You should have received a copy of this license along
 * with this source code in a file named "LICENSE."
 *
 * @file cppIdentifier.cxx
 * @author drose
 * @date 1999-10-26
 */

#include "cppIdentifier.h"
#include "cppScope.h"
#include "cppTemplateScope.h"
#include "cppPreprocessor.h"
#include "cppTemplateParameterList.h"
#include "cppTBDType.h"
#include "cppStructType.h"

using std::string;


/**
 *
 */
CPPIdentifier::
CPPIdentifier(const string &name, const CPPFile &file) {
  _names.push_back(CPPNameComponent(name));
  _native_scope = nullptr;
  _loc.first_line = 0;
  _loc.first_column = 0;
  _loc.last_line = 0;
  _loc.last_column = 0;
  _loc.file = file;
}

/**
 *
 */
CPPIdentifier::
CPPIdentifier(const CPPNameComponent &name, const CPPFile &file) {
  _names.push_back(name);
  _native_scope = nullptr;
  _loc.first_line = 0;
  _loc.first_column = 0;
  _loc.last_line = 0;
  _loc.last_column = 0;
  _loc.file = file;
}

/**
 *
 */
CPPIdentifier::
CPPIdentifier(const string &name, const cppyyltype &loc) : _loc(loc) {
  _names.push_back(CPPNameComponent(name));
  _native_scope = nullptr;
}

/**
 *
 */
CPPIdentifier::
CPPIdentifier(const CPPNameComponent &name, const cppyyltype &loc) : _loc(loc) {
  _names.push_back(name);
  _native_scope = nullptr;
}

/**
 *
 */
void CPPIdentifier::
add_name(const string &name) {
  _names.push_back(CPPNameComponent(name));
}

/**
 *
 */
void CPPIdentifier::
add_name(const CPPNameComponent &name) {
  _names.push_back(name);
}

/**
 *
 */
void CPPIdentifier::
prepend(CPPIdentifier *ident) {
  _names.insert(_names.begin(), ident->_names.begin(), ident->_names.end());
}

/**
 *
 */
bool CPPIdentifier::
operator == (const CPPIdentifier &other) const {
  if (_names.size() != other._names.size()) {
    return false;
  }
  for (int i = 0; i < (int)_names.size(); ++i) {
    if (_names[i] != other._names[i]) {
      return false;
    }
  }

  return true;
}

/**
 *
 */
bool CPPIdentifier::
operator != (const CPPIdentifier &other) const {
  return !(*this == other);
}

/**
 *
 */
bool CPPIdentifier::
operator < (const CPPIdentifier &other) const {
  if (_names.size() != other._names.size()) {
    return _names.size() < other._names.size();
  }
  for (int i = 0; i < (int)_names.size(); ++i) {
    if (_names[i] != other._names[i]) {
      return _names[i] < other._names[i];
    }
  }

  return false;
}

/**
 *
 */
bool CPPIdentifier::
is_scoped() const {
  return _names.size() > 1;
}


/**
 *
 */
string CPPIdentifier::
get_simple_name() const {
  return _names.back().get_name();
}

/**
 *
 */
string CPPIdentifier::
get_local_name(CPPScope *scope) const {
  assert(!_names.empty());

  string result;

  if (scope == nullptr || (_native_scope == nullptr && _names.size() == 1)) {
    result = _names.back().get_name_with_templ(scope);

  } else if (_names.front().empty()) {
    result = get_fully_scoped_name();

  } else {
    // Determine the scope of everything up until but not including the last
    // name.
    CPPScope *my_scope = get_scope(scope, nullptr);

    // Strip off template scopes, since they don't add anything particularly
    // meaningful to the local name.
    while (my_scope != nullptr && my_scope->as_template_scope() != nullptr) {
      my_scope = my_scope->get_parent_scope();
    }

    if (my_scope == nullptr) {
      result = get_fully_scoped_name();
    } else if (my_scope == scope) {
      return _names.back().get_name_with_templ(scope);
    } else {
      result = my_scope->get_local_name(scope);
      if (!result.empty()) {
        result += "::";
      }
      result += _names.back().get_name_with_templ(scope);
    }
  }
  return result;
}

/**
 *
 */
string CPPIdentifier::
get_fully_scoped_name() const {
  assert(!_names.empty());
  Names::const_iterator ni = _names.begin();
  string name = (*ni).get_name_with_templ();
  ++ni;
  while (ni != _names.end()) {
    name += "::" + (*ni).get_name_with_templ();
    ++ni;
  }
  return name;
}

/**
 * Returns true if this declaration is an actual, factual declaration, or
 * false if some part of the declaration depends on a template parameter which
 * has not yet been instantiated.
 */
bool CPPIdentifier::
is_fully_specified() const {
  Names::const_iterator ni;
  for (ni = _names.begin(); ni != _names.end(); ++ni) {
    if ((*ni).has_templ() && !(*ni).get_templ()->is_fully_specified()) {
      return false;
    }
  }

  return true;
}

/**
 * Returns true if the identifier includes a template parameter list that
 * includes some not-yet-defined type.
 */
bool CPPIdentifier::
is_tbd() const {
  Names::const_iterator ni;
  for (ni = _names.begin(); ni != _names.end(); ++ni) {
    if ((*ni).is_tbd()) {
      return true;
    }
  }

  return false;
}

/**
 *
 */
CPPScope *CPPIdentifier::
get_scope(CPPScope *current_scope, CPPScope *global_scope,
          CPPPreprocessor *error_sink) const {
  assert(!_names.empty());

  CPPScope *scope = _native_scope;
  if (scope == nullptr) {
    scope = current_scope;
  }
  int i = 0;

  if (_names[i].empty()) {
    // This identifier starts with a ::, thus it begins at the global scope.
    scope = global_scope;
    i++;
  }

  while (i + 1 < (int)_names.size() && scope != nullptr) {
    // Check for an explicitly specialized scope first.
    CPPScope *next_scope = scope->find_scope(_names[i].get_name_with_templ(), global_scope);
    if (next_scope == nullptr) {
      next_scope = scope->find_scope(_names[i].get_name(), global_scope);
      if (next_scope == nullptr) {
        if (error_sink != nullptr) {
          error_sink->error("Symbol " + _names[i].get_name() +
                            " is not a known scope in " +
                            scope->get_fully_scoped_name(),
                            _loc);
        }
        return nullptr;
      }
      if (_names[i].has_templ()) {
        next_scope = next_scope->instantiate(_names[i].get_templ(),
                                             current_scope, global_scope);
      }
    }
    scope = next_scope;
    i++;
  }

  return scope;
}

/**
 *
 */
CPPScope *CPPIdentifier::
get_scope(CPPScope *current_scope, CPPScope *global_scope,
          CPPDeclaration::SubstDecl &subst,
          CPPPreprocessor *error_sink) const {
  assert(!_names.empty());

  CPPScope *scope = _native_scope;
  if (scope == nullptr) {
    scope = current_scope;
  }
  int i = 0;

  if (_names[i].empty()) {
    // This identifier starts with a ::, thus it begins at the global scope.
    scope = global_scope;
    i++;
  }

  while (i + 1 < (int)_names.size() && scope != nullptr) {
    CPPScope *next_scope = scope->find_scope(_names[i].get_name(), subst,
                                             global_scope);
    if (next_scope == nullptr) {
      if (error_sink != nullptr) {
        error_sink->error("Symbol " + _names[i].get_name() +
                          " is not a known scope in " +
                          scope->get_fully_scoped_name(),
                          _loc);
      }
      return nullptr;
    }
    if (_names[i].has_templ()) {
      next_scope = next_scope->instantiate(_names[i].get_templ(),
                                           current_scope, global_scope);
    }
    scope = next_scope;
    i++;
  }

  return scope;
}

/**
 * Looks up the identifier in the current and/or global scopes, and returns a
 * CPPType pointer if it seems to refer to a type, or NULL if it does not.  If
 * force_instantiate is true, the type will be instantiated as fully as
 * possible right now, even if it means instantiating it into an identical
 * template type.  Otherwise, the instantiation may be delayed for
 * optimization reasons, and a CPPTBDType placeholder may be returned instead.
 */
CPPType *CPPIdentifier::
find_type(CPPScope *current_scope, CPPScope *global_scope,
          bool force_instantiate,
          CPPPreprocessor *error_sink) const {
  CPPScope *scope = get_scope(current_scope, global_scope, error_sink);
  if (scope == nullptr) {
    return nullptr;
  }

  CPPType *type = nullptr;
  if (!_names.back().has_templ()) {
    type = scope->find_type(get_simple_name());

  } else {
    CPPDeclaration *decl = find_symbol(current_scope, global_scope, error_sink);
    type = decl->as_type();
    /*
    if (type != NULL) {
      if (!type->is_incomplete() || force_instantiate) {
        type = type->instantiate(_names.back().get_templ(),
                                 current_scope, global_scope,
                                 error_sink)->as_type();

        // If we ended up with another template, instantiate later.
        if (type->is_template() && !force_instantiate) {
          type = CPPType::new_type(new CPPTBDType((CPPIdentifier *)this));
        }

      } else {
        // Otherwise, we'll have to instantiate the type later.
        type = CPPType::new_type(new CPPTBDType((CPPIdentifier *)this));
      }
      // type->_file.replace_nearer(_file);
    }
    */
  }
  return type;
}

/**
 * This flavor of find_type() will instantiate any scope names in the
 * identifier.  It's useful for fully defining a type while instantiating a
 * class.
 */
CPPType *CPPIdentifier::
find_type(CPPScope *current_scope, CPPScope *global_scope,
          CPPDeclaration::SubstDecl &subst,
          CPPPreprocessor *error_sink) const {
  CPPScope *scope = get_scope(current_scope, global_scope, subst, error_sink);
  if (scope == nullptr) {
    return nullptr;
  }

  CPPType *type = scope->find_type(get_simple_name(), subst, global_scope);
  if (type != nullptr && _names.back().has_templ()) {
    // This is a template type.

    if (is_fully_specified()) {
      // If our identifier fully specifies the instantiation, then apply it.
      CPPDeclaration *decl =
        type->instantiate(_names.back().get_templ(),
                          current_scope, global_scope,
                          error_sink);
      assert(decl != nullptr);
      CPPType *new_type = decl->as_type();
      assert(new_type != nullptr);
      if (new_type == type) {
        type = CPPType::new_type(new CPPTBDType((CPPIdentifier *)this));
      } else {
        type = new_type;
      }
    } else {
      // Otherwise, we'll have to instantiate the type later.
      type = CPPType::new_type(new CPPTBDType((CPPIdentifier *)this));
    }
    // type->_file.replace_nearer(_file);
  }
  return type;
}


/**
 *
 */
CPPDeclaration *CPPIdentifier::
find_symbol(CPPScope *current_scope, CPPScope *global_scope,
            CPPPreprocessor *error_sink) const {
  CPPScope *scope = get_scope(current_scope, global_scope, error_sink);
  if (scope == nullptr) {
    return nullptr;
  }

  CPPDeclaration *sym;
  if (!_names.back().has_templ()) {
    if (_names.size() > 1 && scope->get_simple_name() == get_simple_name()) {
/**

 */
      sym = scope->get_struct_type()->get_constructor();
    } else {
      sym = scope->find_symbol(get_simple_name());
    }

  } else {
    sym = scope->find_template(get_simple_name());
    if (sym != nullptr) {
      CPPType *type = sym->as_type();
      if (type != nullptr && type->is_incomplete()) {
        // We can't instantiate an incomplete type.
        sym = CPPType::new_type(new CPPTBDType((CPPIdentifier *)this));
      } else {
        // Instantiate the symbol.
        sym = sym->instantiate(_names.back().get_templ(), current_scope,
                               global_scope, error_sink);
      }
    }
  }

  return sym;
}

/**
 *
 */
CPPDeclaration *CPPIdentifier::
find_symbol(CPPScope *current_scope, CPPScope *global_scope,
            CPPDeclaration::SubstDecl &subst,
            CPPPreprocessor *error_sink) const {
  CPPScope *scope = get_scope(current_scope, global_scope, subst, error_sink);
  if (scope == nullptr) {
    return nullptr;
  }

  CPPDeclaration *sym;
  if (!_names.back().has_templ()) {
    if (_names.size() > 1 && scope->get_simple_name() == get_simple_name()) {
/**

 */
      sym = scope->get_struct_type()->get_constructor();
    } else {
      sym = scope->find_symbol(get_simple_name());
    }

  } else {
    sym = scope->find_template(get_simple_name());

    if (sym != nullptr) {
      CPPType *type = sym->as_type();
      if (type != nullptr && type->is_incomplete()) {
        // We can't instantiate an incomplete type.
        sym = CPPType::new_type(new CPPTBDType((CPPIdentifier *)this));
      } else {
        // Instantiate the symbol.
        sym = sym->instantiate(_names.back().get_templ(), current_scope,
                               global_scope, error_sink);
      }
    }
  }

  return sym;
}

/**
 *
 */
CPPDeclaration *CPPIdentifier::
find_template(CPPScope *current_scope, CPPScope *global_scope,
              CPPPreprocessor *error_sink) const {
  CPPScope *scope = get_scope(current_scope, global_scope, error_sink);
  if (scope == nullptr) {
    return nullptr;
  }
  return scope->find_template(get_simple_name());
}

/**
 *
 */
CPPScope *CPPIdentifier::
find_scope(CPPScope *current_scope, CPPScope *global_scope,
           CPPPreprocessor *error_sink) const {
  CPPScope *scope = get_scope(current_scope, global_scope, error_sink);
  if (scope == nullptr) {
    return nullptr;
  }
  return scope->find_scope(get_simple_name(), global_scope);
}


/**
 *
 */
CPPIdentifier *CPPIdentifier::
substitute_decl(CPPDeclaration::SubstDecl &subst,
                CPPScope *current_scope, CPPScope *global_scope) {
  CPPIdentifier *rep = new CPPIdentifier(*this);

  bool anything_changed = false;
  for (int i = 0; i < (int)rep->_names.size(); ++i) {
    if (_names[i].has_templ()) {
      rep->_names[i].set_templ
        (_names[i].get_templ()->substitute_decl(subst, current_scope, global_scope));
      if (rep->_names[i].get_templ() != _names[i].get_templ()) {
        anything_changed = true;
      }
    }
  }

  if (!anything_changed) {
    delete rep;
    rep = this;
  }

  return rep;
}

/**
 *
 */
void CPPIdentifier::
output(std::ostream &out, CPPScope *scope) const {
  if (scope == nullptr) {
    output_fully_scoped_name(out);
  } else {
    output_local_name(out, scope);
  }
}


/**
 *
 */
void CPPIdentifier::
output_local_name(std::ostream &out, CPPScope *scope) const {
  assert(!_names.empty());

  if (scope == nullptr || (_native_scope == nullptr && _names.size() == 1)) {
    out << _names.back();
  } else if (_names.front().empty()) {
    output_fully_scoped_name(out);
  } else {
    // Determine the scope of everything up until but not including the last
    // name.
    CPPScope *my_scope = get_scope(scope, nullptr);

    if (my_scope == nullptr) {
      output_fully_scoped_name(out);
    } else {
      out << my_scope->get_local_name(scope) << "::" << _names.back();
    }
  }
}

/**
 *
 */
void CPPIdentifier::
output_fully_scoped_name(std::ostream &out) const {
  if (_native_scope != nullptr) {
    _native_scope->output(out, nullptr);
    out << "::";
  }
  Names::const_iterator ni = _names.begin();
  out << (*ni);
  ++ni;
  while (ni != _names.end()) {
    out << "::" << (*ni);
    ++ni;
  }
}
