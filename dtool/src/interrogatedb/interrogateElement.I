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

////////////////////////////////////////////////////////////////////
//     Function: InterrogateElement::Constructor
//       Access: Public
//  Description:
////////////////////////////////////////////////////////////////////
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
}

////////////////////////////////////////////////////////////////////
//     Function: InterrogateElement::Copy Constructor
//       Access: Public
//  Description:
////////////////////////////////////////////////////////////////////
INLINE InterrogateElement::
InterrogateElement(const InterrogateElement &copy) {
  (*this) = copy;
}

////////////////////////////////////////////////////////////////////
//     Function: InterrogateElement::Copy Assignment Operator
//       Access: Public
//  Description:
////////////////////////////////////////////////////////////////////
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
}

////////////////////////////////////////////////////////////////////
//     Function: InterrogateElement::is_global
//       Access: Public
//  Description: Returns true if the element is marked as 'global'.
//               This means only that it should appear in the global
//               element list.
////////////////////////////////////////////////////////////////////
INLINE bool InterrogateElement::
is_global() const {
  return (_flags & F_global) != 0;
}

////////////////////////////////////////////////////////////////////
//     Function: InterrogateElement::has_scoped_name
//       Access: Public
//  Description:
////////////////////////////////////////////////////////////////////
INLINE bool InterrogateElement::
has_scoped_name() const {
  return !_scoped_name.empty();
}

////////////////////////////////////////////////////////////////////
//     Function: InterrogateElement::get_scoped_name
//       Access: Public
//  Description:
////////////////////////////////////////////////////////////////////
INLINE const string &InterrogateElement::
get_scoped_name() const {
  return _scoped_name;
}

////////////////////////////////////////////////////////////////////
//     Function: InterrogateElement::has_comment
//       Access: Public
//  Description:
////////////////////////////////////////////////////////////////////
INLINE bool InterrogateElement::
has_comment() const {
  return !_comment.empty();
}

////////////////////////////////////////////////////////////////////
//     Function: InterrogateElement::get_comment
//       Access: Public
//  Description:
////////////////////////////////////////////////////////////////////
INLINE const string &InterrogateElement::
get_comment() const {
  return _comment;
}

////////////////////////////////////////////////////////////////////
//     Function: InterrogateElement::get_type
//       Access: Public
//  Description:
////////////////////////////////////////////////////////////////////
INLINE TypeIndex InterrogateElement::
get_type() const {
  return _type;
}

////////////////////////////////////////////////////////////////////
//     Function: InterrogateElement::has_getter
//       Access: Public
//  Description:
////////////////////////////////////////////////////////////////////
INLINE bool InterrogateElement::
has_getter() const {
  return (_flags & F_has_getter) != 0;
}

////////////////////////////////////////////////////////////////////
//     Function: InterrogateElement::get_getter
//       Access: Public
//  Description:
////////////////////////////////////////////////////////////////////
INLINE FunctionIndex InterrogateElement::
get_getter() const {
  return _getter;
}

////////////////////////////////////////////////////////////////////
//     Function: InterrogateElement::has_setter
//       Access: Public
//  Description:
////////////////////////////////////////////////////////////////////
INLINE bool InterrogateElement::
has_setter() const {
  return (_flags & F_has_setter) != 0;
}

////////////////////////////////////////////////////////////////////
//     Function: InterrogateElement::get_setter
//       Access: Public
//  Description:
////////////////////////////////////////////////////////////////////
INLINE FunctionIndex InterrogateElement::
get_setter() const {
  return _setter;
}

////////////////////////////////////////////////////////////////////
//     Function: InterrogateElement::has_has_function
//       Access: Public
//  Description:
////////////////////////////////////////////////////////////////////
INLINE bool InterrogateElement::
has_has_function() const {
  return (_flags & F_has_has_function) != 0;
}

////////////////////////////////////////////////////////////////////
//     Function: InterrogateElement::get_has_function
//       Access: Public
//  Description:
////////////////////////////////////////////////////////////////////
INLINE FunctionIndex InterrogateElement::
get_has_function() const {
  return _has_function;
}

////////////////////////////////////////////////////////////////////
//     Function: InterrogateElement::has_clear_function
//       Access: Public
//  Description:
////////////////////////////////////////////////////////////////////
INLINE bool InterrogateElement::
has_clear_function() const {
  return (_flags & F_has_clear_function) != 0;
}

////////////////////////////////////////////////////////////////////
//     Function: InterrogateElement::get_clear_function
//       Access: Public
//  Description:
////////////////////////////////////////////////////////////////////
INLINE FunctionIndex InterrogateElement::
get_clear_function() const {
  return _clear_function;
}


INLINE ostream &
operator << (ostream &out, const InterrogateElement &element) {
  element.output(out);
  return out;
}

INLINE istream &
operator >> (istream &in, InterrogateElement &element) {
  element.input(in);
  return in;
}
