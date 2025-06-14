/**
 * PANDA 3D SOFTWARE
 * Copyright (c) Carnegie Mellon University.  All rights reserved.
 *
 * All use of this software is subject to the terms of the revised BSD
 * license.  You should have received a copy of this license along
 * with this source code in a file named "LICENSE."
 *
 * @file cppStructType.cxx
 * @author drose
 * @date 1999-10-19
 */

#include "cppStructType.h"
#include "cppTypedefType.h"
#include "cppReferenceType.h"
#include "cppScope.h"
#include "cppTypeProxy.h"
#include "cppTemplateScope.h"
#include "cppFunctionGroup.h"
#include "cppFunctionType.h"
#include "cppParameterList.h"
#include "cppTBDType.h"
#include "indent.h"
#include "cppParser.h"

/**
 *
 */
void CPPStructType::Base::
output(std::ostream &out) const {
  if (_is_virtual) {
    out << "virtual ";
  }
  out << _vis << " " << *_base;
}

/**
 *
 */
CPPStructType::
CPPStructType(CPPStructType::Type type, CPPIdentifier *ident,
              CPPScope *current_scope, CPPScope *scope,
              const CPPFile &file, CPPAttributeList attr) :
  CPPExtensionType(type, ident, current_scope, file, std::move(attr)),
  _scope(scope),
  _final(false)
{
  _subst_decl_recursive_protect = false;
  _incomplete = true;
}

/**
 *
 */
CPPStructType::
CPPStructType(const CPPStructType &copy) :
  CPPExtensionType(copy),
  _scope(copy._scope),
  _incomplete(copy._incomplete),
  _final(copy._final),
  _derivation(copy._derivation)
{
  _subst_decl_recursive_protect = false;
}

/**
 *
 */
void CPPStructType::
operator = (const CPPStructType &copy) {
  CPPExtensionType::operator = (copy);
  _scope = copy._scope;
  _incomplete = copy._incomplete;
  _derivation = copy._derivation;
  _final = copy._final;
}

/**
 * A handy function used while parsing to add a new base class to the list of
 * classes (or structs) this class derives from.
 */
void CPPStructType::
append_derivation(CPPType *base, CPPVisibility vis, bool is_virtual) {
  if (base != nullptr) {
    // Unwrap any typedefs, since we can't inherit from a typedef.
    CPPTypedefType *def = base->as_typedef_type();
    while (def != nullptr) {
      base = def->_type;
      def = base->as_typedef_type();
    }

    if (vis == V_unknown && base->as_extension_type() != nullptr) {
      // Default visibility.
      if (base->as_extension_type()->_type == T_class) {
        vis = V_private;
      } else {
        vis = V_public;
      }
    }

    Base b;
    b._base = base;
    b._vis = vis;
    b._is_virtual = is_virtual;

    _derivation.push_back(b);
  }
}

/**
 *
 */
CPPScope *CPPStructType::
get_scope() const {
  return _scope;
}

/**
 * Returns true if this struct declaration is abstract, e.g.  it contains or
 * inherits at least one method that is pure virtual.
 */
bool CPPStructType::
is_abstract() const {
  VFunctions funcs;
  get_pure_virtual_funcs(funcs);
  return !funcs.empty();
}

/**
 * Returns true if this struct declaration is a base class of the other given
 * class, or the same class.
 */
bool CPPStructType::
is_base_of(const CPPStructType *other) const {
  if (this == other) {
    return true;
  }
  Derivation::const_iterator di;
  for (di = other->_derivation.begin(); di != other->_derivation.end(); ++di) {
    const CPPStructType *base = (*di)._base->as_struct_type();
    if (base != nullptr && is_base_of(base)) {
      return true;
    }
  }
  return false;
}

/**
 * Returns true if this struct declaration defines no non-static data members
 * other than bit-fields of size 0, no virtual functions, no virtual base
 * classes, and no non-empty base classes, and is not a union.
 */
bool CPPStructType::
is_empty() const {
  if (_type == T_union) {
    return false;
  }

  if (check_virtual()) {
    return false;
  }

  // Make sure all base classes are empty and non-virtual.
  Derivation::const_iterator di;
  for (di = _derivation.begin(); di != _derivation.end(); ++di) {
    CPPStructType *base = (*di)._base->as_struct_type();
    if ((*di)._is_virtual || (base != nullptr && !base->is_empty())) {
      return false;
    }
  }

  // Make sure there are no non-static data members.
  CPPScope::Variables::const_iterator vi;
  for (vi = _scope->_variables.begin(); vi != _scope->_variables.end(); ++vi) {
    CPPInstance *instance = (*vi).second;
    assert(instance != nullptr);

    if (instance->_storage_class & CPPInstance::SC_static) {
      // Static members don't count.
      continue;
    }

    // Only members with a bit width of 0 are okay.
    if (instance->_bit_width != 0) {
      return false;
    }
  }

  return true;
}

/**
 * Returns true if this class or any of its base classes have virtual methods.
 */
bool CPPStructType::
is_polymorphic() const {
  if (_type == T_union) {
    return false;
  }
  return check_virtual();
}

/**
 * Returns true if the type is considered a standard layout type.
 */
bool CPPStructType::
is_standard_layout() const {
  assert(_scope != nullptr);

  CPPVisibility member_vis = V_unknown;

  // Make sure all data members have the same vis and are standard layout.
  CPPScope::Variables::const_iterator vi;
  for (vi = _scope->_variables.begin(); vi != _scope->_variables.end(); ++vi) {
    CPPInstance *instance = (*vi).second;
    assert(instance != nullptr);

    if (instance->_storage_class & CPPInstance::SC_static) {
      // Static members don't count.
      continue;
    }

    // Finally, check if the data member itself is standard layout.
    assert(instance->_type != nullptr);
    if (!instance->_type->is_standard_layout()) {
      return false;
    }

    if (member_vis == V_unknown) {
      // The first non-static data member may not be a base class.
      CPPStructType *struct_type = instance->_type->remove_cv()->as_struct_type();
      if (struct_type != nullptr && struct_type->is_base_of(this)) {
        return false;
      }
      member_vis = instance->_vis;

    } else if (member_vis != instance->_vis) {
      // All members need to have the same access control.
      return false;
    }
  }

  // Make sure all base classes are standard-layout and non-virtual.
  Derivation::const_iterator di;
  for (di = _derivation.begin(); di != _derivation.end(); ++di) {
    CPPStructType *base = (*di)._base->as_struct_type();
    if ((*di)._is_virtual) {
      return false;
    }

    // If this class had instance members, all base classes need to be empty.
    if (member_vis != V_unknown) {
      if (!base->is_empty()) {
        return false;
      }
    } else {
      if (!base->is_standard_layout()) {
        return false;
      }
    }
  }

  // Make sure we have no virtual functions.
  return !check_virtual();
}

/**
 * Returns true if the type is considered a Plain Old Data (POD) type.
 */
bool CPPStructType::
is_trivial() const {
  // Make sure all base classes are trivial and non-virtual.
  Derivation::const_iterator di;
  for (di = _derivation.begin(); di != _derivation.end(); ++di) {
    CPPStructType *base = (*di)._base->as_struct_type();
    if ((*di)._is_virtual || (base != nullptr && !base->is_trivial())) {
      return false;
    }
  }

  assert(_scope != nullptr);

  // Make sure all members are trivial.
  CPPScope::Variables::const_iterator vi;
  for (vi = _scope->_variables.begin(); vi != _scope->_variables.end(); ++vi) {
    CPPInstance *instance = (*vi).second;
    assert(instance != nullptr);

    if (instance->_storage_class & CPPInstance::SC_static) {
      // Static members don't count.
      continue;
    }

    if (instance->_initializer != nullptr) {
      // A member with an initializer means the default constructor would
      // assign a value.  This means the type can't be trivial.
      return false;
    }

    // Finally, check if the data member itself is non-trivial.
    assert(instance->_type != nullptr);
    if (!instance->_type->is_trivial()) {
      return false;
    }
  }

  // Now look for functions that are virtual or con/destructors.
  CPPScope::Functions::const_iterator fi;
  for (fi = _scope->_functions.begin(); fi != _scope->_functions.end(); ++fi) {
    CPPFunctionGroup *fgroup = (*fi).second;

    CPPFunctionGroup::Instances::const_iterator ii;
    for (ii = fgroup->_instances.begin(); ii != fgroup->_instances.end(); ++ii) {
      CPPInstance *inst = (*ii);

      if (inst->_storage_class & CPPInstance::SC_virtual) {
        // Virtual functions are banned right off the bat.
        return false;
      }

      // The following checks don't apply for defaulted functions.
      if (inst->_storage_class & CPPInstance::SC_defaulted) {
        continue;
      }

      assert(inst->_type != nullptr);
      CPPFunctionType *ftype = inst->_type->as_function_type();
      assert(ftype != nullptr);

      if (ftype->_flags & (CPPFunctionType::F_destructor |
                           CPPFunctionType::F_move_constructor |
                           CPPFunctionType::F_copy_constructor)) {
        // User-provided destructors and copy/move constructors are not
        // trivial unless they are defaulted (and not virtual).
        return false;
      }

      if ((ftype->_flags & CPPFunctionType::F_constructor) != 0) {
        if (ftype->_parameters->_parameters.size() == 0 &&
            !ftype->_parameters->_includes_ellipsis) {
          // Same for the default constructor.
          return false;
        }
      }

      if (fgroup->_name == "operator =") {
        // Or assignment operators.
        return false;
      }
    }
  }

  // Finally, the class must be default-constructible.
  return is_default_constructible(V_public);
}

/**
 * Returns true if the type can be safely copied by memcpy or memmove.
 */
bool CPPStructType::
is_trivially_copyable() const {
  // Make sure all base classes are trivially copyable and non-virtual.
  Derivation::const_iterator di;
  for (di = _derivation.begin(); di != _derivation.end(); ++di) {
    CPPStructType *base = (*di)._base->as_struct_type();
    if ((*di)._is_virtual || (base != nullptr && !base->is_trivially_copyable())) {
      return false;
    }
  }

  assert(_scope != nullptr);

  // Make sure all members are trivially copyable.
  CPPScope::Variables::const_iterator vi;
  for (vi = _scope->_variables.begin(); vi != _scope->_variables.end(); ++vi) {
    CPPInstance *instance = (*vi).second;
    assert(instance != nullptr);

    if (instance->_storage_class & CPPInstance::SC_static) {
      // Static members don't count.
      continue;
    }

    assert(instance->_type != nullptr);
    if (!instance->_type->is_trivially_copyable()) {
      return false;
    }
  }

  // Now look for functions that are virtual or con/destructors.
  CPPScope::Functions::const_iterator fi;
  for (fi = _scope->_functions.begin(); fi != _scope->_functions.end(); ++fi) {
    CPPFunctionGroup *fgroup = (*fi).second;

    CPPFunctionGroup::Instances::const_iterator ii;
    for (ii = fgroup->_instances.begin(); ii != fgroup->_instances.end(); ++ii) {
      CPPInstance *inst = (*ii);

      if (inst->_storage_class & CPPInstance::SC_virtual) {
        // Virtual functions are banned right off the bat.
        return false;
      }

      // The following checks don't apply for defaulted functions.
      if (inst->_storage_class & CPPInstance::SC_defaulted) {
        continue;
      }

      assert(inst->_type != nullptr);
      CPPFunctionType *ftype = inst->_type->as_function_type();
      assert(ftype != nullptr);

      if (ftype->_flags & (CPPFunctionType::F_destructor |
                           CPPFunctionType::F_move_constructor |
                           CPPFunctionType::F_copy_constructor)) {
        // User-provided destructors and copy/move constructors are not
        // trivial unless they are defaulted (and not virtual).
        return false;
      }

      if (fgroup->_name == "operator =") {
        // Or assignment operators.
        return false;
      }
    }
  }

  return true;
}

/**
 * Returns true if the type can be constructed using the given argument.
 * This implementation is rudimentary, as it does not attempt to follow all of
 * the implicit type conversion rules, but it is still useful.
 */
bool CPPStructType::
is_constructible(const CPPType *given_type) const {
  // Does the type match the copy constructor or move constructor?
  CPPType *base_type = ((CPPType *)given_type)->remove_reference();
  if (is_equivalent(*base_type->remove_cv())) {
    const CPPReferenceType *ref_type = given_type->as_reference_type();
    if (ref_type == nullptr ||
        ref_type->_value_category == CPPReferenceType::VC_rvalue) {
      return is_move_constructible(V_public);
    } else {
      return is_copy_constructible(V_public);
    }
  }

  if (is_abstract()) {
    return false;
  }

  // Check for a different constructor.
  CPPFunctionGroup *fgroup = get_constructor();
  if (fgroup != nullptr) {
    CPPFunctionGroup::Instances::const_iterator ii;
    for (ii = fgroup->_instances.begin();
        ii != fgroup->_instances.end();
        ++ii) {
      CPPInstance *inst = (*ii);
      assert(inst->_type != nullptr);

      CPPFunctionType *ftype = inst->_type->as_function_type();
      assert(ftype != nullptr);

      CPPParameterList *params = ftype->_parameters;
      if (params->_parameters.size() == 1 && !params->_includes_ellipsis) {
        CPPType *param_type = params->_parameters[0]->_type->remove_reference();

        if (!param_type->is_const() && base_type->is_const()) {
          // Can't pass a const object to a function taking a non-const.
          continue;
        }

        // It's deleted, anyhow.
        if ((inst->_storage_class & CPPInstance::SC_deleted) != 0) {
          continue;
        }

        if (param_type->is_equivalent(*base_type)) {
          return true;
        }
      }
    }
  }

  return false;
}

/**
 * Returns true if the type is default-constructible.
 */
bool CPPStructType::
is_default_constructible() const {
  return is_default_constructible(V_public);
}

/**
 * Returns true if the type is copy-constructible.
 */
bool CPPStructType::
is_copy_constructible() const {
  return is_copy_constructible(V_public);
}

/**
 * Returns true if the type is copy-assignable.
 */
bool CPPStructType::
is_copy_assignable() const {
  if (is_abstract()) {
    return false;
  }
  return is_copy_assignable(V_public);
}

/**
 * Returns true if the type is destructible.
 */
bool CPPStructType::
is_destructible() const {
  return is_destructible(V_public);
}

/**
 * Returns true if the type is default-constructible.
 */
bool CPPStructType::
is_default_constructible(CPPVisibility min_vis) const {
  if (is_abstract()) {
    return false;
  }

  CPPInstance *constructor = get_default_constructor();
  if (constructor != nullptr) {
    // It has a default constructor.
    if (constructor->_vis > min_vis) {
      // Inaccessible default constructor.
      return false;
    }

    if (constructor->_storage_class & CPPInstance::SC_deleted) {
      // Deleted default constructor.
      return false;
    }

    return true;
  }

  // Does it have constructors at all?  If so, no implicit one is generated.
  if (get_constructor() != nullptr) {
    return false;
  }

  // Implicit default constructor.  Check if the implicit default constructor
  // is deleted.
  Derivation::const_iterator di;
  for (di = _derivation.begin(); di != _derivation.end(); ++di) {
    CPPStructType *base = (*di)._base->as_struct_type();
    if (base != nullptr) {
      if (!base->is_default_constructible(V_protected)) {
        return false;
      }
    }
  }

  // Make sure all members are default-constructible or have default values.
  CPPScope::Variables::const_iterator vi;
  for (vi = _scope->_variables.begin(); vi != _scope->_variables.end(); ++vi) {
    CPPInstance *instance = (*vi).second;
    assert(instance != nullptr);

    if (instance->_storage_class & CPPInstance::SC_static) {
      // Static members don't count.
      continue;
    }

    if (instance->_initializer != nullptr) {
      // It has a default value.
      continue;
    }

    if (!instance->_type->is_default_constructible()) {
      return false;
    }
  }

  return true;
}

/**
 * Returns true if the type is copy-constructible.
 */
bool CPPStructType::
is_copy_constructible(CPPVisibility min_vis) const {
  if (is_abstract()) {
    return false;
  }

  CPPInstance *constructor = get_copy_constructor();
  if (constructor != nullptr) {
    // It has a copy constructor.
    if (constructor->_vis > min_vis) {
      // Inaccessible copy constructor.
      return false;
    }

    if (constructor->_storage_class & CPPInstance::SC_deleted) {
      // Deleted copy constructor.
      return false;
    }

    return true;
  }

  if (get_move_constructor() != nullptr ||
      get_move_assignment_operator() != nullptr) {
    // A user-declared move constructor or move assignment operator means that
    // the implicitly-declared copy constructor is deleted.
    return false;
  }

  CPPInstance *destructor = get_destructor();
  if (destructor != nullptr) {
    if (destructor->_vis > min_vis) {
      // Inaccessible destructor.
      return false;
    }

    if (destructor->_storage_class & CPPInstance::SC_deleted) {
      // Deleted destructor.
      return false;
    }
  }

  // Implicit copy constructor.  Check if the implicit copy constructor is
  // deleted.
  Derivation::const_iterator di;
  for (di = _derivation.begin(); di != _derivation.end(); ++di) {
    CPPStructType *base = (*di)._base->as_struct_type();
    if (base != nullptr) {
      if (!base->is_copy_constructible(V_protected)) {
        return false;
      }
    }
  }

  // Make sure all members are copy-constructible.
  CPPScope::Variables::const_iterator vi;
  for (vi = _scope->_variables.begin(); vi != _scope->_variables.end(); ++vi) {
    CPPInstance *instance = (*vi).second;
    assert(instance != nullptr);

    if (instance->_storage_class & CPPInstance::SC_static) {
      // Static members don't count.
      continue;
    }

    if (!instance->_type->is_copy_constructible()) {
      return false;
    }
  }

  return true;
}

/**
 * Returns true if the type is move-constructible.
 */
bool CPPStructType::
is_move_constructible(CPPVisibility min_vis) const {
  CPPInstance *constructor = get_move_constructor();
  if (constructor != nullptr) {
    // It has a user-declared move constructor.
    if (constructor->_vis > min_vis) {
      // Inaccessible move constructor.
      return false;
    }

    if (constructor->_storage_class & CPPInstance::SC_deleted) {
      // It is deleted.
      return false;
    }

    if (is_abstract()) {
      return false;
    }

    return true;
  }

  return is_copy_constructible(min_vis);
}

/**
 * Returns true if the type is copy-assignable, without checking whether the
 * class is abstract.
 */
bool CPPStructType::
is_copy_assignable(CPPVisibility min_vis) const {
  CPPInstance *assignment_operator = get_copy_assignment_operator();
  if (assignment_operator != nullptr) {
    // It has a copy assignment operator.
    if (assignment_operator->_vis > min_vis) {
      // Inaccessible copy assignment operator.
      return false;
    }

    if (assignment_operator->_storage_class & CPPInstance::SC_deleted) {
      // Deleted copy assignment operator.
      return false;
    }

    // NB: if it's defaulted, it may still be deleted.
    if ((assignment_operator->_storage_class & CPPInstance::SC_defaulted) == 0) {
      return true;
    }
  }

  // Implicit copy assignment operator.  Check if the implicit or defaulted
  // copy assignment operator is deleted.
  if (!assignment_operator && (get_move_constructor() || get_move_assignment_operator())) {
    // It's not explicitly defaulted, and there is a move constructor or move
    // assignment operator, so the implicitly-declared one is deleted.
    return false;
  }

  Derivation::const_iterator di;
  for (di = _derivation.begin(); di != _derivation.end(); ++di) {
    CPPStructType *base = (*di)._base->as_struct_type();
    if (base != nullptr) {
      if (!base->is_copy_assignable(V_protected)) {
        return false;
      }
    }
  }

  // Make sure all members are assignable.
  CPPScope::Variables::const_iterator vi;
  for (vi = _scope->_variables.begin(); vi != _scope->_variables.end(); ++vi) {
    CPPInstance *instance = (*vi).second;
    assert(instance != nullptr);

    if (instance->_storage_class & CPPInstance::SC_static) {
      // Static members don't count.
      continue;
    }

    if (!instance->_type->is_copy_assignable()) {
      // Const or reference member, can't do it.
      return false;
    }
  }

  return true;
}

/**
 * Returns true if the type is move-assignable.
 */
bool CPPStructType::
is_move_assignable(CPPVisibility min_vis) const {
  CPPInstance *assignment_operator = get_move_assignment_operator();
  if (assignment_operator != nullptr) {
    // It has a user-declared move assignment_operator.
    if (assignment_operator->_vis > min_vis) {
      // Inaccessible move assignment_operator.
      return false;
    }

    if (assignment_operator->_storage_class & CPPInstance::SC_deleted) {
      // It is deleted.
      return false;
    }

    if (is_abstract()) {
      return false;
    }

    return true;
  }

  return is_copy_assignable(min_vis);
}

/**
 * Returns true if the type is destructible.
 */
bool CPPStructType::
is_destructible(CPPVisibility min_vis) const {
  // Do we have an explicit destructor?
  CPPInstance *destructor = get_destructor();
  if (destructor != nullptr) {
    if (destructor->_vis > min_vis) {
      // Yes, but it's inaccessible.
      return false;
    }

    if (destructor->_storage_class & CPPInstance::SC_deleted) {
      // Yes, but it's explicitly been deleted.
      return false;
    }

    return true;
  }

  // Make sure all base classes are destructible.
  Derivation::const_iterator di;
  for (di = _derivation.begin(); di != _derivation.end(); ++di) {
    CPPStructType *base = (*di)._base->as_struct_type();
    if (base != nullptr && !base->is_destructible(V_protected)) {
      return false;
    }
  }

  assert(_scope != nullptr);

  // Make sure all members are destructible.
  CPPScope::Variables::const_iterator vi;
  for (vi = _scope->_variables.begin(); vi != _scope->_variables.end(); ++vi) {
    CPPInstance *instance = (*vi).second;
    assert(instance != nullptr);

    if (instance->_storage_class & CPPInstance::SC_static) {
      // Static members don't count.
      continue;
    }

    // If the data member is not destructible, no go.
    assert(instance->_type != nullptr);
    if (!instance->_type->is_destructible()) {
      return false;
    }
  }

  return true;
}

/**
 * Returns true if variables of this type may be implicitly converted to
 * the other type.
 */
bool CPPStructType::
is_convertible_to(const CPPType *other) const {
  if (CPPType::is_convertible_to(other)) {
    return true;
  }

  // Check all typecast operators to see whether we can cast to a type that is
  // convertible to the other type.
  CPPScope::Functions::const_iterator fi;
  for (fi = _scope->_functions.begin(); fi != _scope->_functions.end(); ++fi) {
    CPPFunctionGroup *fgroup = (*fi).second;

    CPPFunctionGroup::Instances::const_iterator ii;
    for (ii = fgroup->_instances.begin(); ii != fgroup->_instances.end(); ++ii) {
      CPPInstance *inst = (*ii);

      if (inst->_storage_class & (CPPInstance::SC_deleted | CPPInstance::SC_static | CPPInstance::SC_explicit)) {
        // Exclude static/deleted/explicit methods.
        continue;
      }

      // Also, the instance needs to be publicly visible.
      if (inst->_vis > V_public) {
        continue;
      }

      assert(inst->_type != nullptr);
      CPPFunctionType *ftype = inst->_type->as_function_type();
      assert(ftype != nullptr);

      if (ftype->_return_type != nullptr &&
          (ftype->_flags & CPPFunctionType::F_operator_typecast) != 0) {
        // Yes, this is a typecast operator.  Test using the return type.
        if (ftype->_return_type->is_convertible_to(other)) {
          return true;
        }
      }
    }
  }

  // Check whether any of the base classes are convertible.
  Derivation::const_iterator di;
  for (di = _derivation.begin(); di != _derivation.end(); ++di) {
    CPPStructType *base = (*di)._base->as_struct_type();
    if (base != nullptr && (*di)._vis <= V_public && !base->is_convertible_to(other)) {
      return true;
    }
  }

  return false;
}

/**
 * Ensures all functions are correctly marked with the "virtual" flag if they
 * are truly virtual by virtue of inheritance, rather than simply being
 * labeled virtual.
 *
 * This also sets the CPPInstance::SC_inherited_virtual flags on those virtual
 * methods that override a virtual method defined in a parent class (as
 * opposed to those that appear for this first time in this class).  It is
 * sometimes useful to know whether a given virtual method represents the
 * first time that particular method appears.
 *
 * The return value is true if this class defines or inherits any virtual
 * methods (and thus requires a virtual function pointer), or false otherwise.
 */
bool CPPStructType::
check_virtual() const {
  VFunctions funcs;
  get_virtual_funcs(funcs);
  return !funcs.empty();
}

/**
 * Returns true if this class, or any of its base classes, has a virtual
 * destructor.
 */
bool CPPStructType::
has_virtual_destructor() const {
  CPPInstance *destructor = get_destructor();
  if (destructor != nullptr) {
    if (destructor->_storage_class & CPPInstance::SC_virtual) {
      return true;
    }
  }

  Derivation::const_iterator di;
  for (di = _derivation.begin(); di != _derivation.end(); ++di) {
    CPPStructType *base = (*di)._base->as_struct_type();
    if (base != nullptr && base->has_virtual_destructor()) {
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
bool CPPStructType::
is_fully_specified() const {
  if (_scope != nullptr && !_scope->is_fully_specified()) {
    return false;
  }
  return CPPType::is_fully_specified();
}

/**
 * Returns true if the type has not yet been fully specified, false if it has.
 */
bool CPPStructType::
is_incomplete() const {
  return _incomplete;
}

/**
 * Returns the constructor defined for the struct type, if any, or NULL if no
 * constructor is found.
 */
CPPFunctionGroup *CPPStructType::
get_constructor() const {
  // Just look for the function with the same name as the class.
  CPPScope::Functions::const_iterator fi;
  fi = _scope->_functions.find(get_simple_name());
  if (fi != _scope->_functions.end()) {
    return fi->second;
  } else {
    return nullptr;
  }
}

/**
 * Returns the default constructor defined for the struct type, or NULL if
 * there is no user-declared constructor that takes 0 arguments.
 */
CPPInstance *CPPStructType::
get_default_constructor() const {
  CPPFunctionGroup *fgroup = get_constructor();
  if (fgroup == nullptr) {
    return nullptr;
  }

  CPPFunctionGroup::Instances::const_iterator ii;
  for (ii = fgroup->_instances.begin();
       ii != fgroup->_instances.end();
       ++ii) {
    CPPInstance *inst = (*ii);
    assert(inst->_type != nullptr);

    CPPFunctionType *ftype = inst->_type->as_function_type();
    assert(ftype != nullptr);

    if (ftype->_parameters->_parameters.size() == 0 ||
        ftype->_parameters->_parameters.front()->_initializer != nullptr) {
      // It takes 0 parameters (or all parameters have default values).
      return inst;
    }
  }

  return nullptr;
}

/**
 * Returns the copy constructor defined for the struct type, or NULL if no
 * user-declared copy constructor exists.
 */
CPPInstance *CPPStructType::
get_copy_constructor() const {
  CPPFunctionGroup *fgroup = get_constructor();
  if (fgroup == nullptr) {
    return nullptr;
  }

  CPPFunctionGroup::Instances::const_iterator ii;
  for (ii = fgroup->_instances.begin();
       ii != fgroup->_instances.end();
       ++ii) {
    CPPInstance *inst = (*ii);
    assert(inst->_type != nullptr);

    CPPFunctionType *ftype = inst->_type->as_function_type();
    assert(ftype != nullptr);

    if ((ftype->_flags & CPPFunctionType::F_copy_constructor) != 0) {
      return inst;
    }
  }

  return nullptr;
}

/**
 * Returns the move constructor defined for the struct type, or NULL if no
 * user-declared move constructor exists.
 */
CPPInstance *CPPStructType::
get_move_constructor() const {
  CPPFunctionGroup *fgroup = get_constructor();
  if (fgroup == nullptr) {
    return nullptr;
  }

  CPPFunctionGroup::Instances::const_iterator ii;
  for (ii = fgroup->_instances.begin();
       ii != fgroup->_instances.end();
       ++ii) {
    CPPInstance *inst = (*ii);
    assert(inst->_type != nullptr);

    CPPFunctionType *ftype = inst->_type->as_function_type();
    assert(ftype != nullptr);

    if ((ftype->_flags & CPPFunctionType::F_move_constructor) != 0) {
      return inst;
    }
  }

  return nullptr;
}

/**
 * Returns the assignment operator defined for the struct type, if any, or
 * NULL if no assignment operator is found.
 */
CPPFunctionGroup *CPPStructType::
get_assignment_operator() const {
  // Just look for the function with the name "operator ="
  CPPScope::Functions::const_iterator fi;
  fi = _scope->_functions.find("operator =");
  if (fi != _scope->_functions.end()) {
    return fi->second;
  } else {
    return nullptr;
  }
}

/**
 * Returns the copy assignment operator defined for the struct type, or NULL
 * if no user-declared copy assignment operator exists.
 */
CPPInstance *CPPStructType::
get_copy_assignment_operator() const {
  CPPFunctionGroup *fgroup = get_assignment_operator();
  if (fgroup == nullptr) {
    return nullptr;
  }

  CPPFunctionGroup::Instances::const_iterator ii;
  for (ii = fgroup->_instances.begin();
       ii != fgroup->_instances.end();
       ++ii) {
    CPPInstance *inst = (*ii);
    assert(inst->_type != nullptr);

    CPPFunctionType *ftype = inst->_type->as_function_type();
    assert(ftype != nullptr);

    if ((ftype->_flags & CPPFunctionType::F_copy_assignment_operator) != 0) {
      return inst;
    }
  }

  return nullptr;
}

/**
 * Returns the move assignment operator defined for the struct type, or NULL
 * if no user-declared move assignment operator exists.
 */
CPPInstance *CPPStructType::
get_move_assignment_operator() const {
  CPPFunctionGroup *fgroup = get_assignment_operator();
  if (fgroup == nullptr) {
    return nullptr;
  }

  CPPFunctionGroup::Instances::const_iterator ii;
  for (ii = fgroup->_instances.begin();
       ii != fgroup->_instances.end();
       ++ii) {
    CPPInstance *inst = (*ii);
    assert(inst->_type != nullptr);

    CPPFunctionType *ftype = inst->_type->as_function_type();
    assert(ftype != nullptr);

    if ((ftype->_flags & CPPFunctionType::F_move_assignment_operator) != 0) {
      return inst;
    }
  }

  return nullptr;
}

/**
 * Returns the destructor defined for the struct type, if any, or NULL if no
 * user-declared destructor is found.
 */
CPPInstance *CPPStructType::
get_destructor() const {
  // Iterate through all the functions that begin with '~' until we find one
  // that claims to be a destructor.  In theory, there should only be one such
  // function.
  CPPScope::Functions::const_iterator fi;
  fi = _scope->_functions.lower_bound("~");

  while (fi != _scope->_functions.end() &&
         (*fi).first[0] == '~') {
    CPPFunctionGroup *fgroup = (*fi).second;
    CPPFunctionGroup::Instances::const_iterator ii;
    for (ii = fgroup->_instances.begin();
         ii != fgroup->_instances.end();
         ++ii) {
      CPPInstance *inst = (*ii);
      assert(inst->_type != nullptr);

      CPPFunctionType *ftype = inst->_type->as_function_type();
      assert(ftype != nullptr);

      if ((ftype->_flags & CPPFunctionType::F_destructor) != 0) {
        return inst;
      }
    }
    ++fi;
  }

  return nullptr;
}

/**
 *
 */
CPPDeclaration *CPPStructType::
instantiate(const CPPTemplateParameterList *actual_params,
            CPPScope *current_scope, CPPScope *global_scope,
            CPPPreprocessor *error_sink) const {

  // I *think* this assertion is no longer valid.  Who knows.
  // assert(!_incomplete);

  if (_scope == nullptr) {
    if (error_sink != nullptr) {
      error_sink->warning("Ignoring template parameters for class " +
                          get_local_name());
    }
    return (CPPDeclaration *)this;
  }

  CPPScope *scope =
    _scope->instantiate(actual_params, current_scope, global_scope, error_sink);

  if (scope->get_struct_type()->get_scope() != scope) {
    // Hmm, this type seems to be not completely defined.  We must be in the
    // middle of recursively instantiating the scope.  Thus, we don't yet know
    // what its associated struct type will be.

    // Postpone the evaluation of this type.
    CPPIdentifier *ident = new CPPIdentifier(get_fully_scoped_name(), _file);

    return CPPType::new_type(new CPPTBDType(ident));
  }

  CPPType *result = scope->get_struct_type();
  result = CPPType::new_type(result);
  if (result != (CPPType *)this) {
    // This really means the method ought to be non-const.  But I'm too lazy
    // to propagate this change all the way back right now, so this hack is
    // here.
    ((CPPStructType *)this)->_instantiations.insert(result);
  }
  return result;
}

/**
 *
 */
CPPDeclaration *CPPStructType::
substitute_decl(CPPDeclaration::SubstDecl &subst,
                CPPScope *current_scope, CPPScope *global_scope) {
  SubstDecl::const_iterator si = subst.find(this);
  if (si != subst.end()) {
    assert((*si).second != nullptr);
    return (*si).second;
  }

  if (_incomplete) {
    // We haven't finished defining the class yet.
    return this;
  }

  if (_subst_decl_recursive_protect) {
    // We're already executing this block; we'll have to return a proxy to the
    // type which we'll define later.
    CPPTypeProxy *proxy = new CPPTypeProxy;
    _proxies.push_back(proxy);
    assert(proxy != nullptr);
    return proxy;
  }
  _subst_decl_recursive_protect = true;

  CPPStructType *rep = new CPPStructType(*this);

  if (_ident != nullptr) {
    rep->_ident =
      _ident->substitute_decl(subst, current_scope, global_scope);
  }

  if (_scope != nullptr) {
    rep->_scope =
      _scope->substitute_decl(subst, current_scope, global_scope);
    if (rep->_scope != _scope) {
      rep->_scope->set_struct_type(rep);

      // If we just instantiated a template scope, write the template
      // parameters into our identifier.
      CPPScope *pscope = rep->_scope->get_parent_scope();

      if (pscope != nullptr &&
          pscope->_name.has_templ()) {

        // If the struct name didn't have an explicit template reference
        // before, now it does.
        if (_ident != nullptr && !_ident->_names.empty() && !_ident->_names.back().has_templ()) {
          if (rep->is_template()) {
            rep->_template_scope = nullptr;
            CPPNameComponent nc(get_simple_name());
            nc.set_templ(pscope->_name.get_templ());
            rep->_ident = new CPPIdentifier(nc, _file);
          }
        }
      }
    }
  }

  bool unchanged =
    (rep->_ident == _ident && rep->_scope == _scope);

  for (int i = 0; i < (int)_derivation.size(); ++i) {
    rep->_derivation[i]._base =
      _derivation[i]._base->substitute_decl(subst, current_scope, global_scope)->as_type();
    if (rep->_derivation[i]._base != _derivation[i]._base) {
      unchanged = false;
    }
  }

  if (unchanged) {
    delete rep;
    rep = this;
  }

  subst.insert(SubstDecl::value_type(this, rep));

  _subst_decl_recursive_protect = false;
  // Now fill in all the proxies we created for our recursive references.
  Proxies::iterator pi;
  for (pi = _proxies.begin(); pi != _proxies.end(); ++pi) {
    (*pi)->_actual_type = rep;
  }

  assert(rep != nullptr);
  rep = CPPType::new_type(rep)->as_struct_type();
  assert(rep != nullptr);
  if (rep != this) {
    _instantiations.insert(rep);
  }
  return rep;
}

/**
 *
 */
void CPPStructType::
output(std::ostream &out, int indent_level, CPPScope *scope, bool complete) const {
  if (!complete && _ident != nullptr) {
    // If we have a name, use it.
    if (cppparser_output_class_keyword) {
      out << _type << " ";
    }
    out << _ident->get_local_name(scope);

    if (is_template()) {
      CPPTemplateScope *tscope = get_template_scope();
      tscope->_parameters.output(out, scope);
    }

  } else {
    if (is_template()) {
      get_template_scope()->_parameters.write_formal(out, scope);
      indent(out, indent_level);
    }
    out << _type;

    if (!_attributes.is_empty()) {
      out << " " << _attributes;
    }

    if (_ident != nullptr) {
      out << " " << _ident->get_local_name(scope);
    }

    if (_final) {
      out << " final";
    }

    // Show any derivation we may have
    if (!_derivation.empty()) {
      Derivation::const_iterator di = _derivation.begin();
      out << " : " << *di;
      ++di;
      while (di != _derivation.end()) {
        out << ", " << *di;
        ++di;
      }
    }

    out << " {\n";
    _scope->write(out, indent_level + 2, _scope);
    indent(out, indent_level) << "}";
  }
}

/**
 *
 */
CPPDeclaration::SubType CPPStructType::
get_subtype() const {
  return ST_struct;
}

/**
 *
 */
CPPStructType *CPPStructType::
as_struct_type() {
  return this;
}


/**
 * Fills funcs up with a list of all the virtual function declarations (pure-
 * virtual or otherwise) defined at or above this class.  This is used to
 * determine which functions in a given class are actually virtual, since a
 * function is virtual whose parent class holds a virtual function by the same
 * name, whether or not it is actually declared virtual in the derived class.
 */
void CPPStructType::
get_virtual_funcs(VFunctions &funcs) const {
  // First, get all the virtual funcs from our parents.
  Derivation::const_iterator di;
  for (di = _derivation.begin(); di != _derivation.end(); ++di) {
    VFunctions vf;
    CPPStructType *base = (*di)._base->as_struct_type();
    if (base != nullptr) {
      base->get_virtual_funcs(vf);
      funcs.splice(funcs.end(), vf);
    }
  }

  // Now look for matching functions in this class that we can now infer are
  // virtual.
  VFunctions::iterator vfi, vfnext;
  vfi = funcs.begin();
  while (vfi != funcs.end()) {
    vfnext = vfi;
    ++vfnext;

    CPPInstance *inst = (*vfi);
    assert(inst->_type != nullptr);
    CPPFunctionType *base_ftype = inst->_type->as_function_type();
    assert(base_ftype != nullptr);

    if (inst->_storage_class & CPPInstance::SC_deleted) {
      // Ignore deleted functions.

    } else if ((base_ftype->_flags & CPPFunctionType::F_destructor) != 0) {
      // Match destructor-for-destructor; don't try to match destructors up by
      // name.
      CPPInstance *destructor = get_destructor();
      if (destructor != nullptr) {
        // It's a match!  This destructor is virtual.
        funcs.erase(vfi);
        destructor->_storage_class |=
          (CPPInstance::SC_virtual | CPPInstance::SC_inherited_virtual);
      }

    } else {
      // Non-destructors we can try to match up by name.
      std::string fname = inst->get_local_name();
      CPPScope::Functions::const_iterator fi;
      fi = _scope->_functions.find(fname);

      if (fi != _scope->_functions.end()) {
        CPPFunctionGroup *fgroup = (*fi).second;

        // Look for a matching function amid this group.
        bool match_found = false;
        CPPFunctionGroup::Instances::const_iterator ii;
        for (ii = fgroup->_instances.begin();
             ii != fgroup->_instances.end() && !match_found;
             ++ii) {
          CPPInstance *new_inst = (*ii);
          assert(new_inst->_type != nullptr);

          CPPFunctionType *new_ftype = new_inst->_type->as_function_type();
          assert(new_ftype != nullptr);

          if (new_ftype->match_virtual_override(*base_ftype)) {
            // It's a match!  We now know it's virtual.  Erase this function
            // from the list, so we can add it back in below.
            funcs.erase(vfi);
            match_found = true;

            // In fact, it's not only definitely virtual, but it's *inherited*
            // virtual, which means only that the interface is defined in some
            // parent class.  Sometimes this is useful to know.
            new_inst->_storage_class |=
              (CPPInstance::SC_virtual | CPPInstance::SC_inherited_virtual);
          }
        }
      }
    }
    vfi = vfnext;
  }

  // Finally, look for more virtual function definitions.
  CPPScope::Functions::const_iterator fi;
  for (fi = _scope->_functions.begin();
       fi != _scope->_functions.end();
       ++fi) {
    CPPFunctionGroup *fgroup = (*fi).second;
    CPPFunctionGroup::Instances::const_iterator ii;
    for (ii = fgroup->_instances.begin();
         ii != fgroup->_instances.end();
         ++ii) {
      CPPInstance *inst = (*ii);
      if ((inst->_storage_class & CPPInstance::SC_virtual) != 0 &&
          (inst->_storage_class & CPPInstance::SC_deleted) == 0) {
        // Here's a virtual function.
        funcs.push_back(inst);
      }
    }
  }
}

/**
 * Fills funcs up with a list of all the pure virtual function declarations
 * defined at or above this class that have not been given definitions.
 */
void CPPStructType::
get_pure_virtual_funcs(VFunctions &funcs) const {
  // First, get all the virtual functions.
  VFunctions vfuncs;
  get_virtual_funcs(vfuncs);

  // Now traverse the list, getting out those functions that are pure virtual.
  VFunctions::iterator vfi;
  for (vfi = vfuncs.begin(); vfi != vfuncs.end(); ++vfi) {
    CPPInstance *inst = (*vfi);
    if ((inst->_storage_class & CPPInstance::SC_pure_virtual) != 0) {
      funcs.push_back(inst);
    }
  }
}

/**
 * Called by CPPDeclaration to determine whether this type is equivalent to
 * another type of the same type.
 */
bool CPPStructType::
is_equal(const CPPDeclaration *other) const {
  return CPPDeclaration::is_equal(other);
  /*
  const CPPStructType *ot = ((CPPDeclaration *)other)->as_struct_type();
  assert(ot != NULL);

  return this == ot ||
    (get_fully_scoped_name() == ot->get_fully_scoped_name());
  */
}

/**
 * Called by CPPDeclaration to determine whether this type should be ordered
 * before another type of the same type, in an arbitrary but fixed ordering.
 */
bool CPPStructType::
is_less(const CPPDeclaration *other) const {
  return CPPDeclaration::is_less(other);
  /*
  const CPPStructType *ot = ((CPPDeclaration *)other)->as_struct_type();
  assert(ot != NULL);

  if (this == ot) {
    return false;
  }

  return
    (get_fully_scoped_name() < ot->get_fully_scoped_name());
  */
}
