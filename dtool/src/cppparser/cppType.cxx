// Filename: cppType.cxx
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


#include "cppType.h"
#include "cppTypedefType.h"
#include <algorithm>

CPPType::Types CPPType::_types;
CPPType::PreferredNames CPPType::_preferred_names;
CPPType::AltNames CPPType::_alt_names;

bool CPPTypeCompare::
operator () (CPPType *a, CPPType *b) const {
  return (*a) < (*b);
}

////////////////////////////////////////////////////////////////////
//     Function: CPPType::Constructor
//       Access: Public
//  Description:
////////////////////////////////////////////////////////////////////
CPPType::
CPPType(const CPPFile &file) :
  CPPDeclaration(file)
{
  _declaration = (CPPTypeDeclaration *)NULL;

  // This is set true by interrogate when the "forcetype" keyword is
  // used.
  _forcetype = false;
}

////////////////////////////////////////////////////////////////////
//     Function: CPPType::resolve_type
//       Access: Public, Virtual
//  Description: If this CPPType object is a forward reference or
//               other nonspecified reference to a type that might now
//               be known a real type, returns the real type.
//               Otherwise returns the type itself.
////////////////////////////////////////////////////////////////////
CPPType *CPPType::
resolve_type(CPPScope *, CPPScope *) {
  return this;
}

////////////////////////////////////////////////////////////////////
//     Function: CPPType::is_tbd
//       Access: Public, Virtual
//  Description: Returns true if the type, or any nested type within
//               the type, is a CPPTBDType and thus isn't fully
//               determined right now.  In this case, calling
//               resolve_type() may or may not resolve the type.
////////////////////////////////////////////////////////////////////
bool CPPType::
is_tbd() const {
  return false;
}

////////////////////////////////////////////////////////////////////
//     Function: CPPType::is_trivial
//       Access: Public, Virtual
//  Description: Returns true if the type is considered a Plain Old
//               Data (POD) type.
////////////////////////////////////////////////////////////////////
bool CPPType::
is_trivial() const {
  return false;
}

////////////////////////////////////////////////////////////////////
//     Function: CPPType::is_parameter_expr
//       Access: Public, Virtual
//  Description: Returns true if the type is a special parameter
//               expression type.
//
//               This sort of type is created to handle instance
//               declarations that initially look like function
//               prototypes.
////////////////////////////////////////////////////////////////////
bool CPPType::
is_parameter_expr() const {
  return false;
}

////////////////////////////////////////////////////////////////////
//     Function: CPPType::has_typedef_name
//       Access: Public
//  Description: Returns true if the type has even been typedef'ed and
//               therefore has a simple name available to stand for
//               it.  Extension types are all implicitly typedef'ed on
//               declaration.
////////////////////////////////////////////////////////////////////
bool CPPType::
has_typedef_name() const {
  return !_typedefs.empty();
}

////////////////////////////////////////////////////////////////////
//     Function: CPPType::get_typedef_name
//       Access: Public
//  Description: Returns a string that can be used to name the type,
//               if has_typedef_name() returned true.  This will be
//               the first typedef name applied to the type.
////////////////////////////////////////////////////////////////////
string CPPType::
get_typedef_name(CPPScope *scope) const {
  if (_typedefs.empty()) {
    return string();
  } else {
    return _typedefs.front()->get_local_name(scope);
  }
}

////////////////////////////////////////////////////////////////////
//     Function: CPPType::get_simple_name
//       Access: Public, Virtual
//  Description: Returns a fundametal one-word name for the type.
//               This name will not include any scoping operators or
//               template parameters, so it may not be a compilable
//               reference to the type.
////////////////////////////////////////////////////////////////////
string CPPType::
get_simple_name() const {
  return get_local_name();
}

////////////////////////////////////////////////////////////////////
//     Function: CPPType::get_local_name
//       Access: Public, Virtual
//  Description: Returns the compilable, correct name for this type
//               within the indicated scope.  If the scope is NULL,
//               within the scope the type is declared in.
////////////////////////////////////////////////////////////////////
string CPPType::
get_local_name(CPPScope *scope) const {
  ostringstream ostrm;
  output(ostrm, 0, scope, false);
  return ostrm.str();
}

////////////////////////////////////////////////////////////////////
//     Function: CPPType::get_fully_scoped_name
//       Access: Public, Virtual
//  Description: Returns the compilable, correct name for the type,
//               with completely explicit scoping.
////////////////////////////////////////////////////////////////////
string CPPType::
get_fully_scoped_name() const {
  return get_local_name();
}

////////////////////////////////////////////////////////////////////
//     Function: CPPType::get_preferred_name
//       Access: Public, Virtual
//  Description: Returns the best name to use for the type from a
//               programmer's point of view.  This will typically be a
//               typedef name if one is available, or the full C++
//               name if it is not.  The typedef may or may not be
//               visible within the current scope, so this type name
//               may not be compilable.
////////////////////////////////////////////////////////////////////
string CPPType::
get_preferred_name() const {
  string preferred_name = get_preferred_name_for(this);
  if (!preferred_name.empty()) {
    return preferred_name;
  }
  return get_local_name();
}

////////////////////////////////////////////////////////////////////
//     Function: CPPType::get_num_alt_names
//       Access: Public
//  Description: Returns the number of "alternate" names for this
//               type.  The alternate names are alternate typedef
//               names.  This list might be empty, or it might be
//               long.  One of these names may or may not be the same
//               as the "preferred" name.
////////////////////////////////////////////////////////////////////
int CPPType::
get_num_alt_names() const {
  // We do a lookup based on the type's name, instead of its pointer,
  // so we can resolve different expansions of the same type.
  string tname = this->get_fully_scoped_name();

  if (!tname.empty()) {
    AltNames::const_iterator ai;
    ai = _alt_names.find(tname);
    if (ai != _alt_names.end()) {
      const Names &names = (*ai).second;
      return names.size();
    }
  }

  return 0;
}

////////////////////////////////////////////////////////////////////
//     Function: CPPType::get_alt_name
//       Access: Public
//  Description: Returns the nth "alternate" name for this
//               type.  See get_num_alt_names().
////////////////////////////////////////////////////////////////////
string CPPType::
get_alt_name(int n) const {
  // We do a lookup based on the type's name, instead of its pointer,
  // so we can resolve different expansions of the same type.
  string tname = this->get_fully_scoped_name();

  if (!tname.empty()) {
    AltNames::const_iterator ai;
    ai = _alt_names.find(tname);
    if (ai != _alt_names.end()) {
      const Names &names = (*ai).second;
      if (n >= 0 && n < (int)names.size()) {
        return names[n];
      }
    }
  }

  return string();
}

////////////////////////////////////////////////////////////////////
//     Function: CPPType::is_incomplete
//       Access: Public, Virtual
//  Description: Returns true if the type has not yet been fully
//               specified, false if it has.
////////////////////////////////////////////////////////////////////
bool CPPType::
is_incomplete() const {
  return false;
}

////////////////////////////////////////////////////////////////////
//     Function: CPPType::is_equivalent
//       Access: Public, Virtual
//  Description: This is a little more forgiving than is_equal(): it
//               returns true if the types appear to be referring to
//               the same thing, even if they may have different
//               pointers or somewhat different definitions.  It's
//               useful for parameter matching, etc.
////////////////////////////////////////////////////////////////////
bool CPPType::
is_equivalent(const CPPType &other) const {
  if (get_subtype() != other.get_subtype()) {
    return false;
  }
  return is_equal(&other);
}


////////////////////////////////////////////////////////////////////
//     Function: CPPType::output_instance
//       Access: Public, Virtual
//  Description: Formats a C++-looking line that defines an instance
//               of the given type, with the indicated name.  In most
//               cases this will be "type name", but some types have
//               special exceptions.
////////////////////////////////////////////////////////////////////
void CPPType::
output_instance(ostream &out, const string &name, CPPScope *scope) const {
  output_instance(out, 0, scope, false, "", name);
}

////////////////////////////////////////////////////////////////////
//     Function: CPPType::output_instance
//       Access: Public, Virtual
//  Description: Formats a C++-looking line that defines an instance
//               of the given type, with the indicated name.  In most
//               cases this will be "type name", but some types have
//               special exceptions.
////////////////////////////////////////////////////////////////////
void CPPType::
output_instance(ostream &out, int indent_level, CPPScope *scope,
                bool complete, const string &prename,
                const string &name) const {
  output(out, indent_level, scope, complete);
  out << " " << prename << name;
}


////////////////////////////////////////////////////////////////////
//     Function: CPPType::as_type
//       Access: Public, Virtual
//  Description:
////////////////////////////////////////////////////////////////////
CPPType *CPPType::
as_type() {
  return this;
}


////////////////////////////////////////////////////////////////////
//     Function: CPPType::new_type
//       Access: Public, Static
//  Description: This should be called whenever a new CPPType object
//               is created.  It will uniquify the type pointers by
//               checking to see if some equivalent CPPType object has
//               previously been created; if it has, it returns the
//               old object and deletes the new one.  Otherwise, it
//               stores the new one and returns it.
////////////////////////////////////////////////////////////////////
CPPType *CPPType::
new_type(CPPType *type) {
  pair<Types::iterator, bool> result = _types.insert(type);
  if (result.second) {
    // The insertion has taken place; thus, this is the first time
    // this type has been declared.
    assert(*result.first == type);
    return type;
  }

  // If this triggers, we probably messed up by defining is_less()
  // incorrectly; they provide a relative ordering even though they
  // are equal to each other.  Or, we provided an is_equal() that
  // gives false negatives.
  assert(**result.first == *type);

  // The insertion has not taken place; thus, there was previously
  // another equivalent type declared.
  if (*result.first != type) {
    // *** Something wrong here.  Deleting this should always be safe;
    // however, it's not.  Thus, someone failed to call new_type() on
    // a type pointer before saving it somewhere.  Fix me soon.  ****

    delete type;
  }
  return *result.first;
}

////////////////////////////////////////////////////////////////////
//     Function: CPPType::record_alt_name_for
//       Access: Public, Static
//  Description: Records a global typedef name associated with the
//               indicated Type.  This will be an "alt" name, and it
//               may also become the "preferred" name.
////////////////////////////////////////////////////////////////////
void CPPType::
record_alt_name_for(const CPPType *type, const string &name) {
  if (!name.empty()) {
    string tname = type->get_fully_scoped_name();
    if (!tname.empty()) {
      if (tname.find('<') != string::npos) {
        // If the name contains a funny character like a template
        // name, then we implicitly take the first typedef as the
        // preferred name.
        _preferred_names.insert(PreferredNames::value_type(tname, name));
      }

      Names &names = _alt_names[tname];
      if (find(names.begin(), names.end(), name) == names.end()) {
        // It's not already recorded as an alt name, so record it now.
        names.push_back(name);
      }
    }
  }
}

////////////////////////////////////////////////////////////////////
//     Function: CPPType::get_preferred_name_for
//       Access: Public, Static
//  Description: Returns the previously-stored "preferred" name
//               associated with the type, if any, or empty string if
//               no name is associated.
////////////////////////////////////////////////////////////////////
string CPPType::
get_preferred_name_for(const CPPType *type) {
  // We do a lookup based on the type's name, instead of its pointer,
  // so we can resolve different expansions of the same type.
  string tname = type->get_fully_scoped_name();

  if (!tname.empty()) {
    PreferredNames::const_iterator pi;
    pi = _preferred_names.find(tname);
    if (pi != _preferred_names.end()) {
      return (*pi).second;
    }
  }

  return string();
}
