/**
 * PANDA 3D SOFTWARE
 * Copyright (c) Carnegie Mellon University.  All rights reserved.
 *
 * All use of this software is subject to the terms of the revised BSD
 * license.  You should have received a copy of this license along
 * with this source code in a file named "LICENSE."
 *
 * @file interrogateFunctionWrapper.I
 * @author drose
 * @date 2000-08-06
 */

/**
 *
 */
INLINE InterrogateFunctionWrapper::
InterrogateFunctionWrapper(InterrogateModuleDef *def) :
  InterrogateComponent(def)
{
  _flags = 0;
  _function = 0;
  _return_type = 0;
  _return_value_destructor = 0;
}

/**
 *
 */
INLINE InterrogateFunctionWrapper::
InterrogateFunctionWrapper(const InterrogateFunctionWrapper &copy) {
  (*this) = copy;
}

/**
 *
 */
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

/**
 * Returns the FunctionIndex of the function that this wrapper corresponds to.
 */
INLINE FunctionIndex InterrogateFunctionWrapper::
get_function() const {
  return _function;
}

/**
 *
 */
INLINE bool InterrogateFunctionWrapper::
is_callable_by_name() const {
  return (_flags & F_callable_by_name) != 0;
}

/**
 *
 */
INLINE bool InterrogateFunctionWrapper::
has_return_value() const {
  return (_flags & F_has_return) != 0;
}

/**
 *
 */
INLINE TypeIndex InterrogateFunctionWrapper::
get_return_type() const {
  return _return_type;
}

/**
 *
 */
INLINE bool InterrogateFunctionWrapper::
caller_manages_return_value() const {
  return (_flags & F_caller_manages) != 0;
}

/**
 *
 */
INLINE FunctionIndex InterrogateFunctionWrapper::
get_return_value_destructor() const {
  return _return_value_destructor;
}

/**
 *
 */
INLINE int InterrogateFunctionWrapper::
number_of_parameters() const {
  return _parameters.size();
}

/**
 *
 */
INLINE TypeIndex InterrogateFunctionWrapper::
parameter_get_type(int n) const {
  if (n >= 0 && n < (int)_parameters.size()) {
    return _parameters[n]._type;
  }
  return 0;
}

/**
 *
 */
INLINE bool InterrogateFunctionWrapper::
parameter_has_name(int n) const {
  if (n >= 0 && n < (int)_parameters.size()) {
    return (_parameters[n]._parameter_flags & PF_has_name) != 0;
  }
  return false;
}

/**
 *
 */
INLINE const std::string &InterrogateFunctionWrapper::
parameter_get_name(int n) const {
  static std::string bogus_string;
  if (n >= 0 && n < (int)_parameters.size()) {
    return _parameters[n]._name;
  }
  return bogus_string;
}

/**
 *
 */
INLINE bool InterrogateFunctionWrapper::
parameter_is_this(int n) const {
  if (n >= 0 && n < (int)_parameters.size()) {
    return (_parameters[n]._parameter_flags & PF_is_this) != 0;
  }
  return false;
}

/**
 *
 */
INLINE const std::string &InterrogateFunctionWrapper::
get_unique_name() const {
  return _unique_name;
}

/**
 *
 */
INLINE bool InterrogateFunctionWrapper::
has_comment() const {
  return !_comment.empty();
}

/**
 *
 */
INLINE const std::string &InterrogateFunctionWrapper::
get_comment() const {
  return _comment;
}

INLINE std::ostream &
operator << (std::ostream &out, const InterrogateFunctionWrapper &wrapper) {
  wrapper.output(out);
  return out;
}

INLINE std::istream &
operator >> (std::istream &in, InterrogateFunctionWrapper &wrapper) {
  wrapper.input(in);
  return in;
}

INLINE std::ostream &
operator << (std::ostream &out, const InterrogateFunctionWrapper::Parameter &p) {
  p.output(out);
  return out;
}

INLINE std::istream &
operator >> (std::istream &in, InterrogateFunctionWrapper::Parameter &p) {
  p.input(in);
  return in;
}
