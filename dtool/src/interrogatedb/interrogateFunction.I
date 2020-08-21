/**
 * PANDA 3D SOFTWARE
 * Copyright (c) Carnegie Mellon University.  All rights reserved.
 *
 * All use of this software is subject to the terms of the revised BSD
 * license.  You should have received a copy of this license along
 * with this source code in a file named "LICENSE."
 *
 * @file interrogateFunction.I
 * @author drose
 * @date 2000-08-01
 */

/**
 * Returns true if the function is marked as 'global'. This means only that it
 * should appear in the global function list.
 */
INLINE bool InterrogateFunction::
is_global() const {
  return (_flags & F_global) != 0;
}

/**
 * Returns true if the function is virtual, for whatever that's worth.
 */
INLINE bool InterrogateFunction::
is_virtual() const {
  return (_flags & F_virtual) != 0;
}

/**
 * Returns true if the function is a class method.
 */
INLINE bool InterrogateFunction::
is_method() const {
  return (_flags & F_method) != 0;
}

/**
 * Returns true if the function is flagged as a special unary operator, like
 * operator -() with no parameters.
 */
INLINE bool InterrogateFunction::
is_unary_op() const {
  return (_flags & F_unary_op) != 0;
}

/**
 * Returns true if the function is a special typecast operator, like operator
 * bool().
 */
INLINE bool InterrogateFunction::
is_operator_typecast() const {
  return (_flags & F_operator_typecast) != 0;
}

/**
 * Return the class that owns the method, if is_method() returns true.
 */
INLINE TypeIndex InterrogateFunction::
get_class() const {
  return _class;
}

/**
 *
 */
INLINE bool InterrogateFunction::
has_scoped_name() const {
  return !_scoped_name.empty();
}

/**
 *
 */
INLINE const std::string &InterrogateFunction::
get_scoped_name() const {
  return _scoped_name;
}

/**
 *
 */
INLINE bool InterrogateFunction::
has_comment() const {
  return !_comment.empty();
}

/**
 *
 */
INLINE const std::string &InterrogateFunction::
get_comment() const {
  return _comment;
}

/**
 *
 */
INLINE bool InterrogateFunction::
has_prototype() const {
  return !_prototype.empty();
}

/**
 *
 */
INLINE const std::string &InterrogateFunction::
get_prototype() const {
  return _prototype;
}

/**
 *
 */
INLINE int InterrogateFunction::
number_of_c_wrappers() const {
  return _c_wrappers.size();
}

/**
 *
 */
INLINE FunctionWrapperIndex InterrogateFunction::
get_c_wrapper(int n) const {
  if (n >= 0 && n < (int)_c_wrappers.size()) {
    return _c_wrappers[n];
  }
  return 0;
}

/**
 *
 */
INLINE int InterrogateFunction::
number_of_python_wrappers() const {
  return _python_wrappers.size();
}

/**
 *
 */
INLINE FunctionWrapperIndex InterrogateFunction::
get_python_wrapper(int n) const {
  if (n >= 0 && n < (int)_python_wrappers.size()) {
    return _python_wrappers[n];
  }
  return 0;
}


INLINE std::ostream &
operator << (std::ostream &out, const InterrogateFunction &function) {
  function.output(out);
  return out;
}

INLINE std::istream &
operator >> (std::istream &in, InterrogateFunction &function) {
  function.input(in);
  return in;
}
