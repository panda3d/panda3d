/**
 * PANDA 3D SOFTWARE
 * Copyright (c) Carnegie Mellon University.  All rights reserved.
 *
 * All use of this software is subject to the terms of the revised BSD
 * license.  You should have received a copy of this license along
 * with this source code in a file named "LICENSE."
 *
 * @file interrogateElement.I
 * @author drose
 * @date 2000-08-11
 */

/**
 *
 */
INLINE InterrogateElement::
InterrogateElement(InterrogateModuleDef *def) :
  InterrogateComponent(def)
{
  _flags = 0;
  _type = 0;
  _getter = 0;
  _setter = 0;
  _has_function = 0;
  _clear_function = 0;
  _del_function = 0;
  _insert_function = 0;
  _getkey_function = 0;
  _length_function = 0;
  _make_property = nullptr;
}

/**
 *
 */
INLINE InterrogateElement::
InterrogateElement(const InterrogateElement &copy) {
  (*this) = copy;
}

/**
 *
 */
INLINE void InterrogateElement::
operator = (const InterrogateElement &copy) {
  InterrogateComponent::operator = (copy);
  _flags = copy._flags;
  _scoped_name = copy._scoped_name;
  _comment = copy._comment;
  _type = copy._type;
  _getter = copy._getter;
  _setter = copy._setter;
  _has_function = copy._has_function;
  _clear_function = copy._clear_function;
  _del_function = copy._del_function;
  _insert_function = copy._insert_function;
  _getkey_function = copy._getkey_function;
  _length_function = copy._length_function;
  _make_property = copy._make_property;
}

/**
 * Returns true if the element is marked as 'global'. This means only that it
 * should appear in the global element list.
 */
INLINE bool InterrogateElement::
is_global() const {
  return (_flags & F_global) != 0;
}

/**
 *
 */
INLINE bool InterrogateElement::
has_scoped_name() const {
  return !_scoped_name.empty();
}

/**
 *
 */
INLINE const std::string &InterrogateElement::
get_scoped_name() const {
  return _scoped_name;
}

/**
 *
 */
INLINE bool InterrogateElement::
has_comment() const {
  return !_comment.empty();
}

/**
 *
 */
INLINE const std::string &InterrogateElement::
get_comment() const {
  return _comment;
}

/**
 *
 */
INLINE TypeIndex InterrogateElement::
get_type() const {
  return _type;
}

/**
 *
 */
INLINE bool InterrogateElement::
has_getter() const {
  return (_flags & F_has_getter) != 0;
}

/**
 *
 */
INLINE FunctionIndex InterrogateElement::
get_getter() const {
  return _getter;
}

/**
 *
 */
INLINE bool InterrogateElement::
has_setter() const {
  return (_flags & F_has_setter) != 0;
}

/**
 *
 */
INLINE FunctionIndex InterrogateElement::
get_setter() const {
  return _setter;
}

/**
 *
 */
INLINE bool InterrogateElement::
has_has_function() const {
  return (_flags & F_has_has_function) != 0;
}

/**
 *
 */
INLINE FunctionIndex InterrogateElement::
get_has_function() const {
  return _has_function;
}

/**
 *
 */
INLINE bool InterrogateElement::
has_clear_function() const {
  return (_flags & F_has_clear_function) != 0;
}

/**
 *
 */
INLINE FunctionIndex InterrogateElement::
get_clear_function() const {
  return _clear_function;
}

/**
 *
 */
INLINE bool InterrogateElement::
has_del_function() const {
  return (_flags & F_has_del_function) != 0;
}

/**
 *
 */
INLINE FunctionIndex InterrogateElement::
get_del_function() const {
  return _del_function;
}

/**
 *
 */
INLINE bool InterrogateElement::
has_insert_function() const {
  return (_flags & F_has_insert_function) != 0;
}

/**
 *
 */
INLINE FunctionIndex InterrogateElement::
get_insert_function() const {
  return _insert_function;
}

/**
 *
 */
INLINE bool InterrogateElement::
has_getkey_function() const {
  return (_flags & F_has_getkey_function) != 0;
}

/**
 *
 */
INLINE FunctionIndex InterrogateElement::
get_getkey_function() const {
  return _getkey_function;
}

/**
 *
 */
INLINE bool InterrogateElement::
is_sequence() const {
  return (_flags & F_sequence) != 0;
}

/**
 *
 */
INLINE FunctionIndex InterrogateElement::
get_length_function() const {
  return _length_function;
}

/**
 *
 */
INLINE bool InterrogateElement::
is_mapping() const {
  return (_flags & F_mapping) != 0;
}


INLINE std::ostream &
operator << (std::ostream &out, const InterrogateElement &element) {
  element.output(out);
  return out;
}

INLINE std::istream &
operator >> (std::istream &in, InterrogateElement &element) {
  element.input(in);
  return in;
}
