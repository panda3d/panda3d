// Filename: cppStructType.cxx
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


#include "cppStructType.h"
#include "cppTypedefType.h"
#include "cppScope.h"
#include "cppTypeProxy.h"
#include "cppTemplateScope.h"
#include "cppFunctionGroup.h"
#include "cppFunctionType.h"
#include "cppParameterList.h"
#include "cppTBDType.h"
#include "indent.h"
#include "cppParser.h"

////////////////////////////////////////////////////////////////////
//     Function: CPPStructType::Base::output
//       Access: Public
//  Description:
////////////////////////////////////////////////////////////////////
void CPPStructType::Base::
output(ostream &out) const {
  if (_is_virtual) {
    out << "virtual ";
  }
  out << _vis << " " << *_base;
}

////////////////////////////////////////////////////////////////////
//     Function: CPPStructType::Constructor
//       Access: Public
//  Description:
////////////////////////////////////////////////////////////////////
CPPStructType::
CPPStructType(CPPStructType::Type type, CPPIdentifier *ident,
              CPPScope *current_scope, CPPScope *scope,
              const CPPFile &file) :
  CPPExtensionType(type, ident, current_scope, file),
  _scope(scope)
{
  _subst_decl_recursive_protect = false;
  _incomplete = true;
}

////////////////////////////////////////////////////////////////////
//     Function: CPPStructType::Copy Constructor
//       Access: Public
//  Description:
////////////////////////////////////////////////////////////////////
CPPStructType::
CPPStructType(const CPPStructType &copy) :
  CPPExtensionType(copy),
  _scope(copy._scope),
  _incomplete(copy._incomplete),
  _derivation(copy._derivation)
{
  _subst_decl_recursive_protect = false;
}

////////////////////////////////////////////////////////////////////
//     Function: CPPStructType::Copy Assignment Operator
//       Access: Public
//  Description:
////////////////////////////////////////////////////////////////////
void CPPStructType::
operator = (const CPPStructType &copy) {
  CPPExtensionType::operator = (copy);
  _scope = copy._scope;
  _incomplete = copy._incomplete;
  _derivation = copy._derivation;
}

////////////////////////////////////////////////////////////////////
//     Function: CPPStructType::append_derivation
//       Access: Public
//  Description: A handy function used while parsing to add a new base
//               class to the list of classes (or structs) this class
//               derives from.
////////////////////////////////////////////////////////////////////
void CPPStructType::
append_derivation(CPPType *base, CPPVisibility vis, bool is_virtual) {
  if (base != NULL) {
    // Unwrap any typedefs, since we can't inherit from a typedef.
    CPPTypedefType *def = base->as_typedef_type();
    while (def != NULL) {
      base = def->_type;
      def = base->as_typedef_type();
    }

    Base b;
    b._base = base;
    b._vis = vis;
    b._is_virtual = is_virtual;

    _derivation.push_back(b);
  }
}

////////////////////////////////////////////////////////////////////
//     Function: CPPStructType::get_scope
//       Access: Public
//  Description:
////////////////////////////////////////////////////////////////////
CPPScope *CPPStructType::
get_scope() const {
  return _scope;
}

////////////////////////////////////////////////////////////////////
//     Function: CPPStructType::is_abstract
//       Access: Public
//  Description: Returns true if this struct declaration is abstract,
//               e.g. it contains or inherits at least one method that
//               is pure virtual.
////////////////////////////////////////////////////////////////////
bool CPPStructType::
is_abstract() const {
  VFunctions funcs;
  get_pure_virtual_funcs(funcs);
  return !funcs.empty();
}

////////////////////////////////////////////////////////////////////
//     Function: CPPStructType::is_trivial
//       Access: Public, Virtual
//  Description: Returns true if the type is considered a Plain Old
//               Data (POD) type.
////////////////////////////////////////////////////////////////////
bool CPPStructType::
is_trivial() const {
  // Make sure all base classes are trivial.
  Derivation::const_iterator di;
  for (di = _derivation.begin(); di != _derivation.end(); ++di) {
    CPPStructType *base = (*di)._base->as_struct_type();
    if (base != NULL && !base->is_trivial()) {
      return false;
    }
  }

  assert(_scope != NULL);

  // Make sure all members are trivial.
  CPPScope::Variables::const_iterator vi;
  for (vi = _scope->_variables.begin(); vi != _scope->_variables.end(); ++vi) {
    CPPInstance *instance = (*vi).second;
    assert(instance != NULL);

    if (instance->_storage_class & CPPInstance::SC_static) {
      // Static members don't count.
      continue;
    }

    if (instance->_initializer != NULL) {
      // A member with an initializer means the default constructor would
      // assign a value.  This means the type can't be trivial.
      return false;
    }

    // Finally, check if the data member itself is non-trivial.
    assert(instance->_type != NULL);
    if (!instance->_type->is_trivial()) {
      return false;
    }
  }

  // Now look for functions that are virtual or con/destructors.
  bool is_default_constructible = true;
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

      assert(inst->_type != (CPPType *)NULL);
      CPPFunctionType *ftype = inst->_type->as_function_type();
      assert(ftype != (CPPFunctionType *)NULL);

      if (ftype->_flags & (CPPFunctionType::F_destructor |
                           CPPFunctionType::F_move_constructor |
                           CPPFunctionType::F_copy_constructor)) {
        // User-provided destructors and copy/move constructors are not trivial.
        return false;
      }

      if ((ftype->_flags & CPPFunctionType::F_constructor) != 0) {
        if (ftype->_parameters->_parameters.size() == 0 &&
            !ftype->_parameters->_includes_ellipsis) {
          // Same for the default constructor.
          return false;
        }
        // The presence of a non-default constructor makes the class not
        // default-constructible.
        is_default_constructible = false;
      }

      if (fgroup->_name == "operator =") {
        // Or assignment operators.
        return false;
      }
    }
  }

  // Finally, the class must be default-constructible.
  return is_default_constructible;
}

////////////////////////////////////////////////////////////////////
//     Function: CPPStructType::check_virtual
//       Access: Public
//  Description: Ensures all functions are correctly marked with the
//               "virtual" flag if they are truly virtual by virtue of
//               inheritance, rather than simply being labeled
//               virtual.
//
//               This also sets the CPPInstance::SC_inherited_virtual
//               flags on those virtual methods that override a
//               virtual method defined in a parent class (as opposed
//               to those that appear for this first time in this
//               class).  It is sometimes useful to know whether a
//               given virtual method represents the first time that
//               particular method appears.
//
//               The return value is true if this class defines or
//               inherits any virtual methods (and thus requires a
//               virtual function pointer), or false otherwise.
////////////////////////////////////////////////////////////////////
bool CPPStructType::
check_virtual() {
  VFunctions funcs;
  get_virtual_funcs(funcs);
  return !funcs.empty();
}

////////////////////////////////////////////////////////////////////
//     Function: CPPStructType::is_fully_specified
//       Access: Public, Virtual
//  Description: Returns true if this declaration is an actual,
//               factual declaration, or false if some part of the
//               declaration depends on a template parameter which has
//               not yet been instantiated.
////////////////////////////////////////////////////////////////////
bool CPPStructType::
is_fully_specified() const {
  if (_scope != NULL && !_scope->is_fully_specified()) {
    return false;
  }
  return CPPType::is_fully_specified();
}

////////////////////////////////////////////////////////////////////
//     Function: CPPStructType::is_incomplete
//       Access: Public, Virtual
//  Description: Returns true if the type has not yet been fully
//               specified, false if it has.
////////////////////////////////////////////////////////////////////
bool CPPStructType::
is_incomplete() const {
  return _incomplete;
}

////////////////////////////////////////////////////////////////////
//     Function: CPPStructType::get_destructor
//       Access: Public
//  Description: Returns the destructor defined for the struct type,
//               if any, or NULL if no destructor is found.
////////////////////////////////////////////////////////////////////
CPPInstance *CPPStructType::
get_destructor() const {
  // Iterate through all the functions that begin with '~' until we
  // find one that claims to be a destructor.  In theory, there should
  // only be one such function.
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
      assert(inst->_type != (CPPType *)NULL);

      CPPFunctionType *ftype = inst->_type->as_function_type();
      assert(ftype != (CPPFunctionType *)NULL);

      if ((ftype->_flags & CPPFunctionType::F_destructor) != 0) {
        return inst;
      }
    }
    ++fi;
  }

  return (CPPInstance *)NULL;
}

////////////////////////////////////////////////////////////////////
//     Function: CPPStructType::instantiate
//       Access: Public, Virtual
//  Description:
////////////////////////////////////////////////////////////////////
CPPDeclaration *CPPStructType::
instantiate(const CPPTemplateParameterList *actual_params,
            CPPScope *current_scope, CPPScope *global_scope,
            CPPPreprocessor *error_sink) const {

  // I *think* this assertion is no longer valid.  Who knows.
  //  assert(!_incomplete);

  if (_scope == NULL) {
    if (error_sink != NULL) {
      error_sink->warning("Ignoring template parameters for class " +
                          get_local_name());
    }
    return (CPPDeclaration *)this;
  }

  CPPScope *scope =
    _scope->instantiate(actual_params, current_scope, global_scope, error_sink);

  if (scope->get_struct_type()->get_scope() != scope) {
    // Hmm, this type seems to be not completely defined.  We must be
    // in the middle of recursively instantiating the scope.  Thus, we
    // don't yet know what its associated struct type will be.

    // Postpone the evaluation of this type.
    CPPIdentifier *ident = new CPPIdentifier(get_fully_scoped_name(), _file);

    return CPPType::new_type(new CPPTBDType(ident));
  }

  CPPType *result = scope->get_struct_type();
  result = CPPType::new_type(result);
  if (result != (CPPType *)this) {
    // This really means the method ought to be non-const.  But I'm
    // too lazy to propagate this change all the way back right now,
    // so this hack is here.
    ((CPPStructType *)this)->_instantiations.insert(result);
  }
  return result;
}

////////////////////////////////////////////////////////////////////
//     Function: CPPStructType::substitute_decl
//       Access: Public, Virtual
//  Description:
////////////////////////////////////////////////////////////////////
CPPDeclaration *CPPStructType::
substitute_decl(CPPDeclaration::SubstDecl &subst,
                CPPScope *current_scope, CPPScope *global_scope) {
  SubstDecl::const_iterator si = subst.find(this);
  if (si != subst.end()) {
    assert((*si).second != NULL);
    return (*si).second;
  }

  if (_incomplete) {
    // We haven't finished defining the class yet.
    return this;
  }

  if (_subst_decl_recursive_protect) {
    // We're already executing this block; we'll have to return a
    // proxy to the type which we'll define later.
    CPPTypeProxy *proxy = new CPPTypeProxy;
    _proxies.push_back(proxy);
    assert(proxy != NULL);
    return proxy;
  }
  _subst_decl_recursive_protect = true;

  CPPStructType *rep = new CPPStructType(*this);

  if (_ident != NULL) {
    rep->_ident =
      _ident->substitute_decl(subst, current_scope, global_scope);
  }

  if (_scope != NULL) {
    rep->_scope =
      _scope->substitute_decl(subst, current_scope, global_scope);
    if (rep->_scope != _scope) {
      rep->_scope->set_struct_type(rep);

      // If we just instantiated a template scope, write the template
      // parameters into our identifier.
      CPPScope *pscope = rep->_scope->get_parent_scope();

      if (pscope != (CPPScope *)NULL &&
          pscope->_name.has_templ()) {

        // If the struct name didn't have an explicit template
        // reference before, now it does.
        if (!_ident->_names.empty() && !_ident->_names.back().has_templ()) {
          if (rep->is_template()) {
            rep->_template_scope = (CPPTemplateScope *)NULL;
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
  // Now fill in all the proxies we created for our recursive
  // references.
  Proxies::iterator pi;
  for (pi = _proxies.begin(); pi != _proxies.end(); ++pi) {
    (*pi)->_actual_type = rep;
  }

  assert(rep != NULL);
  rep = CPPType::new_type(rep)->as_struct_type();
  assert(rep != NULL);
  if (rep != this) {
    _instantiations.insert(rep);
  }
  return rep;
}

////////////////////////////////////////////////////////////////////
//     Function: CPPStructType::output
//       Access: Public, Virtual
//  Description:
////////////////////////////////////////////////////////////////////
void CPPStructType::
output(ostream &out, int indent_level, CPPScope *scope, bool complete) const {
  if (!complete && _ident != NULL) {
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
    if (_ident != NULL) {
      out << _type << " " << _ident->get_local_name(scope);
    } else {
      out << _type;
    }

    // Show any derivation we may have
    if (!_derivation.empty()) {
      Derivation::const_iterator di = _derivation.begin();
      out << ": " << *di;
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

////////////////////////////////////////////////////////////////////
//     Function: CPPStructType::get_subtype
//       Access: Public, Virtual
//  Description:
////////////////////////////////////////////////////////////////////
CPPDeclaration::SubType CPPStructType::
get_subtype() const {
  return ST_struct;
}

////////////////////////////////////////////////////////////////////
//     Function: CPPStructType::as_struct_type
//       Access: Public, Virtual
//  Description:
////////////////////////////////////////////////////////////////////
CPPStructType *CPPStructType::
as_struct_type() {
  return this;
}


////////////////////////////////////////////////////////////////////
//     Function: CPPStructType::get_virtual_funcs
//       Access: Public
//  Description: Fills funcs up with a list of all the virtual
//               function declarations (pure-virtual or otherwise)
//               defined at or above this class.  This is used to
//               determine which functions in a given class are
//               actually virtual, since a function is virtual whose
//               parent class holds a virtual function by the same
//               name, whether or not it is actually declared virtual
//               in the derived class.
////////////////////////////////////////////////////////////////////
void CPPStructType::
get_virtual_funcs(VFunctions &funcs) const {
  // First, get all the virtual funcs from our parents.
  Derivation::const_iterator di;
  for (di = _derivation.begin(); di != _derivation.end(); ++di) {
    VFunctions vf;
    CPPStructType *base = (*di)._base->as_struct_type();
    if (base != NULL) {
      base->get_virtual_funcs(vf);
      funcs.splice(funcs.end(), vf);
    }
  }

  // Now look for matching functions in this class that we can now
  // infer are virtual.
  VFunctions::iterator vfi, vfnext;
  vfi = funcs.begin();
  while (vfi != funcs.end()) {
    vfnext = vfi;
    ++vfnext;

    CPPInstance *inst = (*vfi);
    assert(inst->_type != (CPPType *)NULL);
    CPPFunctionType *base_ftype = inst->_type->as_function_type();
    assert(base_ftype != (CPPFunctionType *)NULL);

    if ((base_ftype->_flags & CPPFunctionType::F_destructor) != 0) {
      // Match destructor-for-destructor; don't try to match
      // destructors up by name.
      CPPInstance *destructor = get_destructor();
      if (destructor != (CPPInstance *)NULL) {
        // It's a match!  This destructor is virtual.
        funcs.erase(vfi);
        destructor->_storage_class |=
          (CPPInstance::SC_virtual | CPPInstance::SC_inherited_virtual);
      }

    } else {
      // Non-destructors we can try to match up by name.
      string fname = inst->get_local_name();
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
          assert(new_inst->_type != (CPPType *)NULL);

          CPPFunctionType *new_ftype = new_inst->_type->as_function_type();
          assert(new_ftype != (CPPFunctionType *)NULL);

          if (new_ftype->is_equivalent_function(*base_ftype)) {
            // It's a match!  We now know it's virtual.  Erase this
            // function from the list, so we can add it back in below.
            funcs.erase(vfi);
            match_found = true;

            // In fact, it's not only definitely virtual, but it's
            // *inherited* virtual, which means only that the
            // interface is defined in some parent class.  Sometimes
            // this is useful to know.
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
      if ((inst->_storage_class & CPPInstance::SC_virtual) != 0) {
        // Here's a virtual function.
        funcs.push_back(inst);
      }
    }
  }
}

////////////////////////////////////////////////////////////////////
//     Function: CPPStructType::count_pure_virtual_funcs
//       Access: Public
//  Description: Fills funcs up with a list of all the pure virtual
//               function declarations defined at or above this class
//               that have not been given definitions.
////////////////////////////////////////////////////////////////////
void CPPStructType::
get_pure_virtual_funcs(VFunctions &funcs) const {
  // First, get all the virtual functions.
  VFunctions vfuncs;
  get_virtual_funcs(vfuncs);

  // Now traverse the list, getting out those functions that are pure
  // virtual.
  VFunctions::iterator vfi;
  for (vfi = vfuncs.begin(); vfi != vfuncs.end(); ++vfi) {
    CPPInstance *inst = (*vfi);
    if ((inst->_storage_class & CPPInstance::SC_pure_virtual) != 0) {
      funcs.push_back(inst);
    }
  }
}

////////////////////////////////////////////////////////////////////
//     Function: CPPStructType::is_equal
//       Access: Protected, Virtual
//  Description: Called by CPPDeclaration to determine whether this
//               type is equivalent to another type of the same type.
////////////////////////////////////////////////////////////////////
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

////////////////////////////////////////////////////////////////////
//     Function: CPPStructType::is_less
//       Access: Protected, Virtual
//  Description: Called by CPPDeclaration to determine whether this
//               type should be ordered before another type of the
//               same type, in an arbitrary but fixed ordering.
////////////////////////////////////////////////////////////////////
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

