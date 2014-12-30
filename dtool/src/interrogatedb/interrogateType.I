// Filename: interrogateType.I
// Created by:  drose (31Jul00)
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


////////////////////////////////////////////////////////////////////
//     Function: InterrogateType::is_global
//       Access: Public
//  Description: Returns true if the type is marked as 'global'.  This
//               means only that it should appear in the global type
//               list.
////////////////////////////////////////////////////////////////////
INLINE bool InterrogateType::
is_global() const {
  return (_flags & F_global) != 0;
}

////////////////////////////////////////////////////////////////////
//     Function: InterrogateType::has_scoped_name
//       Access: Public
//  Description:
////////////////////////////////////////////////////////////////////
INLINE bool InterrogateType::
has_scoped_name() const {
  return !_scoped_name.empty();
}

////////////////////////////////////////////////////////////////////
//     Function: InterrogateType::get_scoped_name
//       Access: Public
//  Description:
////////////////////////////////////////////////////////////////////
INLINE const string &InterrogateType::
get_scoped_name() const {
  return _scoped_name;
}

////////////////////////////////////////////////////////////////////
//     Function: InterrogateType::has_true_name
//       Access: Public
//  Description:
////////////////////////////////////////////////////////////////////
INLINE bool InterrogateType::
has_true_name() const {
  return !_true_name.empty();
}

////////////////////////////////////////////////////////////////////
//     Function: InterrogateType::get_true_name
//       Access: Public
//  Description:
////////////////////////////////////////////////////////////////////
INLINE const string &InterrogateType::
get_true_name() const {
  return _true_name;
}

////////////////////////////////////////////////////////////////////
//     Function: InterrogateType::has_comment
//       Access: Public
//  Description:
////////////////////////////////////////////////////////////////////
INLINE bool InterrogateType::
has_comment() const {
  return !_comment.empty();
}

////////////////////////////////////////////////////////////////////
//     Function: InterrogateType::get_comment
//       Access: Public
//  Description:
////////////////////////////////////////////////////////////////////
INLINE const string &InterrogateType::
get_comment() const {
  return _comment;
}

////////////////////////////////////////////////////////////////////
//     Function: InterrogateType::is_nested
//       Access: Public
//  Description: Returns true if this type is nested within some class
//               definition.
////////////////////////////////////////////////////////////////////
INLINE bool InterrogateType::
is_nested() const {
  return (_flags & F_nested) != 0;
}

////////////////////////////////////////////////////////////////////
//     Function: InterrogateType::get_outer_class
//       Access: Public
//  Description: If is_nested() returns true, this is the class within
//               which this type is defined.
////////////////////////////////////////////////////////////////////
INLINE TypeIndex InterrogateType::
get_outer_class() const {
  return _outer_class;
}

////////////////////////////////////////////////////////////////////
//     Function: InterrogateType::is_atomic
//       Access: Public
//  Description:
////////////////////////////////////////////////////////////////////
INLINE bool InterrogateType::
is_atomic() const {
  return (_flags & F_atomic) != 0;
}

////////////////////////////////////////////////////////////////////
//     Function: InterrogateType::get_atomic_token
//       Access: Public
//  Description:
////////////////////////////////////////////////////////////////////
INLINE AtomicToken InterrogateType::
get_atomic_token() const {
  return _atomic_token;
}

////////////////////////////////////////////////////////////////////
//     Function: InterrogateType::is_unsigned
//       Access: Public
//  Description:
////////////////////////////////////////////////////////////////////
INLINE bool InterrogateType::
is_unsigned() const {
  return (_flags & F_unsigned) != 0;
}

////////////////////////////////////////////////////////////////////
//     Function: InterrogateType::is_signed
//       Access: Public
//  Description:
////////////////////////////////////////////////////////////////////
INLINE bool InterrogateType::
is_signed() const {
  return (_flags & F_signed) != 0;
}

////////////////////////////////////////////////////////////////////
//     Function: InterrogateType::is_long
//       Access: Public
//  Description:
////////////////////////////////////////////////////////////////////
INLINE bool InterrogateType::
is_long() const {
  return (_flags & F_long) != 0;
}

////////////////////////////////////////////////////////////////////
//     Function: InterrogateType::is_longlong
//       Access: Public
//  Description:
////////////////////////////////////////////////////////////////////
INLINE bool InterrogateType::
is_longlong() const {
  return (_flags & F_longlong) != 0;
}

////////////////////////////////////////////////////////////////////
//     Function: InterrogateType::is_short
//       Access: Public
//  Description:
////////////////////////////////////////////////////////////////////
INLINE bool InterrogateType::
is_short() const {
  return (_flags & F_short) != 0;
}

////////////////////////////////////////////////////////////////////
//     Function: InterrogateType::is_wrapped
//       Access: Public
//  Description:
////////////////////////////////////////////////////////////////////
INLINE bool InterrogateType::
is_wrapped() const {
  return (_flags & F_wrapped) != 0;
}

////////////////////////////////////////////////////////////////////
//     Function: InterrogateType::is_pointer
//       Access: Public
//  Description:
////////////////////////////////////////////////////////////////////
INLINE bool InterrogateType::
is_pointer() const {
  return (_flags & F_pointer) != 0;
}

////////////////////////////////////////////////////////////////////
//     Function: InterrogateType::is_const
//       Access: Public
//  Description:
////////////////////////////////////////////////////////////////////
INLINE bool InterrogateType::
is_const() const {
  return (_flags & F_const) != 0;
}

////////////////////////////////////////////////////////////////////
//     Function: InterrogateType::is_typedef
//       Access: Public
//  Description:
////////////////////////////////////////////////////////////////////
INLINE bool InterrogateType::
is_typedef() const {
  return (_flags & F_typedef) != 0;
}

////////////////////////////////////////////////////////////////////
//     Function: InterrogateType::get_wrapped_type
//       Access: Public
//  Description:
////////////////////////////////////////////////////////////////////
INLINE TypeIndex InterrogateType::
get_wrapped_type() const {
  return _wrapped_type;
}

////////////////////////////////////////////////////////////////////
//     Function: InterrogateType::is_array
//       Access: Public
//  Description:
////////////////////////////////////////////////////////////////////
INLINE bool InterrogateType::
is_array() const {
  return (_flags & F_array) != 0;
}

////////////////////////////////////////////////////////////////////
//     Function: InterrogateType::get_array_size
//       Access: Public
//  Description:
////////////////////////////////////////////////////////////////////
INLINE int InterrogateType::
get_array_size() const {
  return _array_size;
}

////////////////////////////////////////////////////////////////////
//     Function: InterrogateType::is_enum
//       Access: Public
//  Description:
////////////////////////////////////////////////////////////////////
INLINE bool InterrogateType::
is_enum() const {
  return (_flags & F_enum) != 0;
}

////////////////////////////////////////////////////////////////////
//     Function: InterrogateType::number_of_enum_values
//       Access: Public
//  Description:
////////////////////////////////////////////////////////////////////
INLINE int InterrogateType::
number_of_enum_values() const {
  return _enum_values.size();
}

////////////////////////////////////////////////////////////////////
//     Function: InterrogateType::get_enum_value_name
//       Access: Public
//  Description:
////////////////////////////////////////////////////////////////////
INLINE const string &InterrogateType::
get_enum_value_name(int n) const {
  if (n >= 0 && n < (int)_enum_values.size()) {
    return _enum_values[n]._name;
  }
  return _empty_string;
}

////////////////////////////////////////////////////////////////////
//     Function: InterrogateType::get_enum_value_scoped_name
//       Access: Public
//  Description:
////////////////////////////////////////////////////////////////////
INLINE const string &InterrogateType::
get_enum_value_scoped_name(int n) const {
  if (n >= 0 && n < (int)_enum_values.size()) {
    return _enum_values[n]._scoped_name;
  }
  return _empty_string;
}

////////////////////////////////////////////////////////////////////
//     Function: InterrogateType::get_enum_value_comment
//       Access: Public
//  Description:
////////////////////////////////////////////////////////////////////
INLINE const string &InterrogateType::
get_enum_value_comment(int n) const {
  if (n >= 0 && n < (int)_enum_values.size()) {
    return _enum_values[n]._comment;
  }
  return _empty_string;
}

////////////////////////////////////////////////////////////////////
//     Function: InterrogateType::get_enum_value
//       Access: Public
//  Description:
////////////////////////////////////////////////////////////////////
INLINE int InterrogateType::
get_enum_value(int n) const {
  if (n >= 0 && n < (int)_enum_values.size()) {
    return _enum_values[n]._value;
  }
  return 0;
}

////////////////////////////////////////////////////////////////////
//     Function: InterrogateType::is_struct
//       Access: Public
//  Description:
////////////////////////////////////////////////////////////////////
INLINE bool InterrogateType::
is_struct() const {
  return (_flags & F_struct) != 0;
}

////////////////////////////////////////////////////////////////////
//     Function: InterrogateType::is_class
//       Access: Public
//  Description:
////////////////////////////////////////////////////////////////////
INLINE bool InterrogateType::
is_class() const {
  return (_flags & F_class) != 0;
}

////////////////////////////////////////////////////////////////////
//     Function: InterrogateType::is_union
//       Access: Public
//  Description:
////////////////////////////////////////////////////////////////////
INLINE bool InterrogateType::
is_union() const {
  return (_flags & F_union) != 0;
}

////////////////////////////////////////////////////////////////////
//     Function: InterrogateType::is_fully_defined
//       Access: Public
//  Description:
////////////////////////////////////////////////////////////////////
INLINE bool InterrogateType::
is_fully_defined() const {
  return (_flags & F_fully_defined) != 0;
}

////////////////////////////////////////////////////////////////////
//     Function: InterrogateType::is_unpublished
//       Access: Public
//  Description: Returns true if the type is an unpublished type.
//               This either means the type is a nested type, and it
//               is protected or private within its scope, or that its
//               definition is simply not marked as 'published'.
////////////////////////////////////////////////////////////////////
INLINE bool InterrogateType::
is_unpublished() const {
  return (_flags & F_unpublished) != 0;
}

////////////////////////////////////////////////////////////////////
//     Function: InterrogateType::number_of_constructors
//       Access: Public
//  Description:
////////////////////////////////////////////////////////////////////
INLINE int InterrogateType::
number_of_constructors() const {
  return _constructors.size();
}

////////////////////////////////////////////////////////////////////
//     Function: InterrogateType::get_constructor
//       Access: Public
//  Description:
////////////////////////////////////////////////////////////////////
INLINE FunctionIndex InterrogateType::
get_constructor(int n) const {
  if (n >= 0 && n < (int)_constructors.size()) {
    return _constructors[n];
  } else {
    return 0;
  }
}

////////////////////////////////////////////////////////////////////
//     Function: InterrogateType::has_destructor
//       Access: Public
//  Description:
////////////////////////////////////////////////////////////////////
INLINE bool InterrogateType::
has_destructor() const {
  return (_destructor != 0);
}

////////////////////////////////////////////////////////////////////
//     Function: InterrogateType::destructor_is_inherited
//       Access: Public
//  Description:
////////////////////////////////////////////////////////////////////
INLINE bool InterrogateType::
destructor_is_inherited() const {
  return (_flags & F_inherited_destructor) != 0;
}

////////////////////////////////////////////////////////////////////
//     Function: InterrogateType::get_destructor
//       Access: Public
//  Description:
////////////////////////////////////////////////////////////////////
INLINE FunctionIndex InterrogateType::
get_destructor() const {
  return _destructor;
}

////////////////////////////////////////////////////////////////////
//     Function: InterrogateType::number_of_elements
//       Access: Public
//  Description:
////////////////////////////////////////////////////////////////////
INLINE int InterrogateType::
number_of_elements() const {
  return _elements.size();
}

////////////////////////////////////////////////////////////////////
//     Function: InterrogateType::get_element
//       Access: Public
//  Description:
////////////////////////////////////////////////////////////////////
INLINE ElementIndex InterrogateType::
get_element(int n) const {
  if (n >= 0 && n < (int)_elements.size()) {
    return _elements[n];
  } else {
    return 0;
  }
}

////////////////////////////////////////////////////////////////////
//     Function: InterrogateType::number_of_methods
//       Access: Public
//  Description:
////////////////////////////////////////////////////////////////////
INLINE int InterrogateType::
number_of_methods() const {
  return _methods.size();
}

////////////////////////////////////////////////////////////////////
//     Function: InterrogateType::get_method
//       Access: Public
//  Description:
////////////////////////////////////////////////////////////////////
INLINE FunctionIndex InterrogateType::
get_method(int n) const {
  if (n >= 0 && n < (int)_methods.size()) {
    return _methods[n];
  } else {
    return 0;
  }
}

////////////////////////////////////////////////////////////////////
//     Function: InterrogateType::number_of_make_seqs
//       Access: Public
//  Description:
////////////////////////////////////////////////////////////////////
INLINE int InterrogateType::
number_of_make_seqs() const {
  return _make_seqs.size();
}

////////////////////////////////////////////////////////////////////
//     Function: InterrogateType::get_make_seq
//       Access: Public
//  Description:
////////////////////////////////////////////////////////////////////
INLINE MakeSeqIndex InterrogateType::
get_make_seq(int n) const {
  if (n >= 0 && n < (int)_make_seqs.size()) {
    return _make_seqs[n];
  } else {
    return 0;
  }
}

////////////////////////////////////////////////////////////////////
//     Function: InterrogateType::number_of_casts
//       Access: Public
//  Description:
////////////////////////////////////////////////////////////////////
INLINE int InterrogateType::
number_of_casts() const {
  return _casts.size();
}

////////////////////////////////////////////////////////////////////
//     Function: InterrogateType::get_cast
//       Access: Public
//  Description:
////////////////////////////////////////////////////////////////////
INLINE FunctionIndex InterrogateType::
get_cast(int n) const {
  if (n >= 0 && n < (int)_casts.size()) {
    return _casts[n];
  } else {
    return 0;
  }
}

////////////////////////////////////////////////////////////////////
//     Function: InterrogateType::number_of_derivations
//       Access: Public
//  Description:
////////////////////////////////////////////////////////////////////
INLINE int InterrogateType::
number_of_derivations() const {
  return _derivations.size();
}

////////////////////////////////////////////////////////////////////
//     Function: InterrogateType::get_derivation
//       Access: Public
//  Description:
////////////////////////////////////////////////////////////////////
INLINE TypeIndex InterrogateType::
get_derivation(int n) const {
  if (n >= 0 && n < (int)_derivations.size()) {
    return _derivations[n]._base;
  } else {
    return 0;
  }
}

////////////////////////////////////////////////////////////////////
//     Function: InterrogateType::derivation_has_upcast
//       Access: Public
//  Description:
////////////////////////////////////////////////////////////////////
INLINE bool InterrogateType::
derivation_has_upcast(int n) const {
  if (n >= 0 && n < (int)_derivations.size()) {
    return (_derivations[n]._flags & DF_upcast) != 0;
  } else {
    return false;
  }
}

////////////////////////////////////////////////////////////////////
//     Function: InterrogateType::derivation_get_upcast
//       Access: Public
//  Description:
////////////////////////////////////////////////////////////////////
INLINE TypeIndex InterrogateType::
derivation_get_upcast(int n) const {
  if (n >= 0 && n < (int)_derivations.size()) {
    return _derivations[n]._upcast;
  } else {
    return 0;
  }
}

////////////////////////////////////////////////////////////////////
//     Function: InterrogateType::derivation_downcast_is_impossible
//       Access: Public
//  Description:
////////////////////////////////////////////////////////////////////
INLINE bool InterrogateType::
derivation_downcast_is_impossible(int n) const {
  if (n >= 0 && n < (int)_derivations.size()) {
    return (_derivations[n]._flags & DF_downcast_impossible) != 0;
  } else {
    return false;
  }
}

////////////////////////////////////////////////////////////////////
//     Function: InterrogateType::derivation_has_downcast
//       Access: Public
//  Description:
////////////////////////////////////////////////////////////////////
INLINE bool InterrogateType::
derivation_has_downcast(int n) const {
  if (n >= 0 && n < (int)_derivations.size()) {
    return (_derivations[n]._flags & DF_downcast) != 0;
  } else {
    return false;
  }
}

////////////////////////////////////////////////////////////////////
//     Function: InterrogateType::derivation_get_downcast
//       Access: Public
//  Description:
////////////////////////////////////////////////////////////////////
INLINE TypeIndex InterrogateType::
derivation_get_downcast(int n) const {
  if (n >= 0 && n < (int)_derivations.size()) {
    return _derivations[n]._downcast;
  } else {
    return 0;
  }
}

////////////////////////////////////////////////////////////////////
//     Function: InterrogateType::number_of_nested_types
//       Access: Public
//  Description:
////////////////////////////////////////////////////////////////////
INLINE int InterrogateType::
number_of_nested_types() const {
  return _nested_types.size();
}

////////////////////////////////////////////////////////////////////
//     Function: InterrogateType::get_nested_type
//       Access: Public
//  Description:
////////////////////////////////////////////////////////////////////
INLINE TypeIndex InterrogateType::
get_nested_type(int n) const {
  if (n >= 0 && n < (int)_nested_types.size()) {
    return _nested_types[n];
  } else {
    return 0;
  }
}

INLINE ostream &
operator << (ostream &out, const InterrogateType &type) {
  type.output(out);
  return out;
}

INLINE istream &
operator >> (istream &in, InterrogateType &type) {
  type.input(in);
  return in;
}

INLINE ostream &
operator << (ostream &out, const InterrogateType::Derivation &d) {
  d.output(out);
  return out;
}

INLINE istream &
operator >> (istream &in, InterrogateType::Derivation &d) {
  d.input(in);
  return in;
}

INLINE ostream &
operator << (ostream &out, const InterrogateType::EnumValue &ev) {
  ev.output(out);
  return out;
}

INLINE istream &
operator >> (istream &in, InterrogateType::EnumValue &ev) {
  ev.input(in);
  return in;
}

