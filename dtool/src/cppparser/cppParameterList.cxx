/**
 * PANDA 3D SOFTWARE
 * Copyright (c) Carnegie Mellon University.  All rights reserved.
 *
 * All use of this software is subject to the terms of the revised BSD
 * license.  You should have received a copy of this license along
 * with this source code in a file named "LICENSE."
 *
 * @file cppParameterList.cxx
 * @author drose
 * @date 1999-10-21
 */

#include "cppParameterList.h"
#include "cppInstance.h"

/**
 *
 */
CPPParameterList::
CPPParameterList() {
  _includes_ellipsis = false;
}

/**
 * This is similar to operator == except it is more forgiving: it is true if
 * only the length and order of types is the same, never minding the instance
 * names or initial values.
 */
bool CPPParameterList::
is_equivalent(const CPPParameterList &other) const {
  if (_includes_ellipsis != other._includes_ellipsis) {
    return false;
  }
  if (_parameters.size() != other._parameters.size()) {
    return false;
  }
  for (int i = 0; i < (int)_parameters.size(); ++i) {
    if (!_parameters[i]->_type->is_equivalent(*other._parameters[i]->_type)) {
      return false;
    }
  }
  return true;
}

/**
 *
 */
bool CPPParameterList::
operator == (const CPPParameterList &other) const {
  if (_includes_ellipsis != other._includes_ellipsis) {
    return false;
  }
  if (_parameters.size() != other._parameters.size()) {
    return false;
  }
  for (int i = 0; i < (int)_parameters.size(); ++i) {
    if (*_parameters[i] != *other._parameters[i]) {
      return false;
    }
  }
  return true;
}

/**
 *
 */
bool CPPParameterList::
operator != (const CPPParameterList &other) const {
  return !(*this == other);
}

/**
 *
 */
bool CPPParameterList::
operator < (const CPPParameterList &other) const {
  if (_includes_ellipsis != other._includes_ellipsis) {
    return _includes_ellipsis < other._includes_ellipsis;
  }
  if (_parameters.size() != other._parameters.size()) {
    return _parameters.size() < other._parameters.size();
  }
  for (int i = 0; i < (int)_parameters.size(); ++i) {
    if (*_parameters[i] != *other._parameters[i]) {
      return *_parameters[i] < *other._parameters[i];
    }
  }
  return false;
}

/**
 * Returns true if any of the types in the parameter list are base on
 * CPPTBDType.
 */
bool CPPParameterList::
is_tbd() const {
  for (int i = 0; i < (int)_parameters.size(); ++i) {
    if (_parameters[i]->_type->is_tbd()) {
      return true;
    }
  }
  return false;
}

/**
 * Returns true if any of the types in the parameter list turns out to be a
 * constant expression, which is a clue that this parameter list is actually
 * intended to be an instance declaration.
 */
bool CPPParameterList::
is_parameter_expr() const {
  for (int i = 0; i < (int)_parameters.size(); ++i) {
    if (_parameters[i]->_type->is_parameter_expr()) {
      return true;
    }
  }
  return false;
}

/**
 * Returns true if this declaration is an actual, factual declaration, or
 * false if some part of the declaration depends on a template parameter which
 * has not yet been instantiated.
 */
bool CPPParameterList::
is_fully_specified() const {
  for (int i = 0; i < (int)_parameters.size(); ++i) {
    if (!_parameters[i]->is_fully_specified()) {
      return false;
    }
  }

  return true;
}

/**
 *
 */
CPPParameterList *CPPParameterList::
substitute_decl(CPPDeclaration::SubstDecl &subst,
                CPPScope *current_scope, CPPScope *global_scope) {
  CPPParameterList *rep = new CPPParameterList;
  bool any_changed = false;
  for (int i = 0; i < (int)_parameters.size(); ++i) {
    CPPInstance *inst =
      _parameters[i]->substitute_decl(subst, current_scope, global_scope)
      ->as_instance();
    if (inst != _parameters[i]) {
      any_changed = true;
    }
    rep->_parameters.push_back(inst);
  }

  if (!any_changed) {
    delete rep;
    rep = this;
  }
  return rep;
}


/**
 * Returns an equivalent CPPParameterList, in which all of the individual
 * types have been resolved.
 */
CPPParameterList *CPPParameterList::
resolve_type(CPPScope *current_scope, CPPScope *global_scope) {
  CPPParameterList *rep = new CPPParameterList;
  bool any_changed = false;
  for (int i = 0; i < (int)_parameters.size(); ++i) {
    CPPInstance *inst = _parameters[i];
    CPPType *new_type = inst->_type;
    if (new_type->is_tbd()) {
      new_type = new_type->resolve_type(current_scope, global_scope);
    }

    if (new_type != inst->_type) {
      any_changed = true;
      CPPInstance *new_inst = new CPPInstance(*inst);
      new_inst->_type = new_type;
      rep->_parameters.push_back(new_inst);
    } else {
      rep->_parameters.push_back(inst);
    }
  }

  if (!any_changed) {
    delete rep;
    rep = this;
  }
  return rep;
}

/**
 * If num_default_parameters is >= 0, it indicates the number of default
 * parameter values to show on output.  Otherwise, all parameter values are
 * shown.
 */
void CPPParameterList::
output(std::ostream &out, CPPScope *scope, bool parameter_names,
       int num_default_parameters) const {
  if (!_parameters.empty()) {
    for (int i = 0; i < (int)_parameters.size(); ++i) {
      if (i != 0) {
        out << ", ";
      }

      // Save the default value expression; we might be about to temporarily
      // clear it.
      CPPExpression *expr = _parameters[i]->_initializer;

      if (num_default_parameters >= 0 &&
          i < (int)_parameters.size() - num_default_parameters) {
        // Don't show the default value for this parameter.
        _parameters[i]->_initializer = nullptr;
      }

      if (parameter_names) {
        _parameters[i]->output(out, 0, scope, false);
      } else {
        _parameters[i]->_type->output(out, 0, scope, false);
      }

      // Restore the default value expression.
      _parameters[i]->_initializer = expr;
    }
    if (_includes_ellipsis) {
      out << ", ...";
    }

  } else if (_includes_ellipsis) {
    out << "...";

  } else {
    // No parameters.
    out << "void";
  }
}
