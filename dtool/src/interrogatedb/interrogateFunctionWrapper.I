// Filename: interrogateFunctionWrapper.I
// Created by:  drose (06Aug00)
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
//     Function: InterrogateFunctionWrapper::Constructor
//       Access: Public
//  Description:
////////////////////////////////////////////////////////////////////
INLINE InterrogateFunctionWrapper::
InterrogateFunctionWrapper(InterrogateModuleDef *def) :
  InterrogateComponent(def)
{
  _flags = 0;
  _function = 0;
  _return_type = 0;
  _return_value_destructor = 0;
}

////////////////////////////////////////////////////////////////////
//     Function: InterrogateFunctionWrapper::Copy Constructor
//       Access: Public
//  Description:
////////////////////////////////////////////////////////////////////
INLINE InterrogateFunctionWrapper::
InterrogateFunctionWrapper(const InterrogateFunctionWrapper &copy) {
  (*this) = copy;
}

////////////////////////////////////////////////////////////////////
//     Function: InterrogateFunctionWrapper::Copy Assignment Operator
//       Access: Public
//  Description:
////////////////////////////////////////////////////////////////////
INLINE void InterrogateFunctionWrapper::
operator = (const InterrogateFunctionWrapper &copy) {
  InterrogateComponent::operator = (copy);
  _flags = copy._flags;
  _function = copy._function;
  _return_type = copy._return_type;
  _return_value_destructor = copy._return_value_destructor;
  _unique_name = copy._unique_name;
  _comment = copy._comment;
  _parameters = copy._parameters;
}

////////////////////////////////////////////////////////////////////
//     Function: InterrogateFunctionWrapper::get_function
//       Access: Public
//  Description: Returns the FunctionIndex of the function that this
//               wrapper corresponds to.
////////////////////////////////////////////////////////////////////
INLINE FunctionIndex InterrogateFunctionWrapper::
get_function() const {
  return _function;
}

////////////////////////////////////////////////////////////////////
//     Function: InterrogateFunctionWrapper::is_callable_by_name
//       Access: Public
//  Description:
////////////////////////////////////////////////////////////////////
INLINE bool InterrogateFunctionWrapper::
is_callable_by_name() const {
  return (_flags & F_callable_by_name) != 0;
}

////////////////////////////////////////////////////////////////////
//     Function: InterrogateFunctionWrapper::has_return_value
//       Access: Public
//  Description:
////////////////////////////////////////////////////////////////////
INLINE bool InterrogateFunctionWrapper::
has_return_value() const {
  return (_flags & F_has_return) != 0;
}

////////////////////////////////////////////////////////////////////
//     Function: InterrogateFunctionWrapper::get_return_type
//       Access: Public
//  Description:
////////////////////////////////////////////////////////////////////
INLINE TypeIndex InterrogateFunctionWrapper::
get_return_type() const {
  return _return_type;
}

////////////////////////////////////////////////////////////////////
//     Function: InterrogateFunctionWrapper::caller_manages_return_value
//       Access: Public
//  Description:
////////////////////////////////////////////////////////////////////
INLINE bool InterrogateFunctionWrapper::
caller_manages_return_value() const {
  return (_flags & F_caller_manages) != 0;
}

////////////////////////////////////////////////////////////////////
//     Function: InterrogateFunctionWrapper::get_return_value_destructor
//       Access: Public
//  Description:
////////////////////////////////////////////////////////////////////
INLINE FunctionIndex InterrogateFunctionWrapper::
get_return_value_destructor() const {
  return _return_value_destructor;
}

////////////////////////////////////////////////////////////////////
//     Function: InterrogateFunctionWrapper::number_of_parameters
//       Access: Public
//  Description:
////////////////////////////////////////////////////////////////////
INLINE int InterrogateFunctionWrapper::
number_of_parameters() const {
  return _parameters.size();
}

////////////////////////////////////////////////////////////////////
//     Function: InterrogateFunctionWrapper::parameter_get_type
//       Access: Public
//  Description:
////////////////////////////////////////////////////////////////////
INLINE TypeIndex InterrogateFunctionWrapper::
parameter_get_type(int n) const {
  if (n >= 0 && n < (int)_parameters.size()) {
    return _parameters[n]._type;
  }
  return 0;
}

////////////////////////////////////////////////////////////////////
//     Function: InterrogateFunctionWrapper::parameter_has_name
//       Access: Public
//  Description:
////////////////////////////////////////////////////////////////////
INLINE bool InterrogateFunctionWrapper::
parameter_has_name(int n) const {
  if (n >= 0 && n < (int)_parameters.size()) {
    return (_parameters[n]._parameter_flags & PF_has_name) != 0;
  }
  return false;
}

////////////////////////////////////////////////////////////////////
//     Function: InterrogateFunctionWrapper::parameter_get_name
//       Access: Public
//  Description:
////////////////////////////////////////////////////////////////////
INLINE const string &InterrogateFunctionWrapper::
parameter_get_name(int n) const {
  static string bogus_string;
  if (n >= 0 && n < (int)_parameters.size()) {
    return _parameters[n]._name;
  }
  return bogus_string;
}

////////////////////////////////////////////////////////////////////
//     Function: InterrogateFunctionWrapper::parameter_is_this
//       Access: Public
//  Description:
////////////////////////////////////////////////////////////////////
INLINE bool InterrogateFunctionWrapper::
parameter_is_this(int n) const {
  if (n >= 0 && n < (int)_parameters.size()) {
    return (_parameters[n]._parameter_flags & PF_is_this) != 0;
  }
  return false;
}

////////////////////////////////////////////////////////////////////
//     Function: InterrogateFunctionWrapper::get_unique_name
//       Access: Public
//  Description:
////////////////////////////////////////////////////////////////////
INLINE const string &InterrogateFunctionWrapper::
get_unique_name() const {
  return _unique_name;
}

////////////////////////////////////////////////////////////////////
//     Function: InterrogateFunctionWrapper::has_comment
//       Access: Public
//  Description:
////////////////////////////////////////////////////////////////////
INLINE bool InterrogateFunctionWrapper::
has_comment() const {
  return !_comment.empty();
}

////////////////////////////////////////////////////////////////////
//     Function: InterrogateFunctionWrapper::get_comment
//       Access: Public
//  Description:
////////////////////////////////////////////////////////////////////
INLINE const string &InterrogateFunctionWrapper::
get_comment() const {
  return _comment;
}

INLINE ostream &
operator << (ostream &out, const InterrogateFunctionWrapper &wrapper) {
  wrapper.output(out);
  return out;
}

INLINE istream &
operator >> (istream &in, InterrogateFunctionWrapper &wrapper) {
  wrapper.input(in);
  return in;
}

INLINE ostream &
operator << (ostream &out, const InterrogateFunctionWrapper::Parameter &p) {
  p.output(out);
  return out;
}

INLINE istream &
operator >> (istream &in, InterrogateFunctionWrapper::Parameter &p) {
  p.input(in);
  return in;
}

