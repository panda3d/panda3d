// Filename: buttonHandle.I
// Created by:  drose (01Mar00)
//
////////////////////////////////////////////////////////////////////
//
// PANDA 3D SOFTWARE
// Copyright (c) 2001 - 2004, Disney Enterprises, Inc.  All rights reserved
//
// All use of this software is subject to the terms of the Panda 3d
// Software license.  You should have received a copy of this license
// along with this source code; you will also find a current copy of
// the license at http://etc.cmu.edu/panda3d/docs/license/ .
//
// To contact the maintainers of this program write to
// panda3d-general@lists.sourceforge.net .
//
////////////////////////////////////////////////////////////////////


////////////////////////////////////////////////////////////////////
//     Function: ButtonHandle::Constructor
//       Access: Published
//  Description: The default constructor must do nothing, because we
//               can't guarantee ordering of static initializers.  If
//               the constructor tried to initialize its value, it
//               might happen after the value had already been set
//               previously by another static initializer!
////////////////////////////////////////////////////////////////////
INLINE ButtonHandle::
ButtonHandle() {
}

////////////////////////////////////////////////////////////////////
//     Function: ButtonHandle::Copy Constructor
//       Access: Published
//  Description:
////////////////////////////////////////////////////////////////////
INLINE ButtonHandle::
ButtonHandle(const ButtonHandle &copy) : _index(copy._index) {
}

////////////////////////////////////////////////////////////////////
//     Function: ButtonHandle::Equality Operator
//       Access: Published
//  Description:
////////////////////////////////////////////////////////////////////
INLINE bool ButtonHandle::
operator == (const ButtonHandle &other) const {
  return (_index == other._index);
}

////////////////////////////////////////////////////////////////////
//     Function: ButtonHandle::Inequality Operator
//       Access: Published
//  Description:
////////////////////////////////////////////////////////////////////
INLINE bool ButtonHandle::
operator != (const ButtonHandle &other) const {
  return (_index != other._index);
}

////////////////////////////////////////////////////////////////////
//     Function: ButtonHandle::Ordering Operator
//       Access: Published
//  Description:
////////////////////////////////////////////////////////////////////
INLINE bool ButtonHandle::
operator < (const ButtonHandle &other) const {
  return (_index < other._index);
}

////////////////////////////////////////////////////////////////////
//     Function: ButtonHandle::has_ascii_equivalent
//       Access: Published
//  Description: Returns true if the button was created with an ASCII
//               equivalent code (e.g. for a standard keyboard
//               button).
////////////////////////////////////////////////////////////////////
INLINE bool ButtonHandle::
has_ascii_equivalent() const {
  return (_index > 0 && _index < 128);
}

////////////////////////////////////////////////////////////////////
//     Function: ButtonHandle::get_ascii_equivalent
//       Access: Published
//  Description: Returns the character code associated with the
//               button, or '\0' if no ASCII code was associated.
////////////////////////////////////////////////////////////////////
INLINE char ButtonHandle::
get_ascii_equivalent() const {
  return has_ascii_equivalent() ? (char)_index : '\0';
}

////////////////////////////////////////////////////////////////////
//     Function: ButtonHandle::matches
//       Access: Published
//  Description: Returns true if this ButtonHandle is the same as the
//               other one, or if the other one is an alias for this
//               one.  (Does not return true if this button is an
//               alias for the other one, however.)
//
//               This is a more general comparison than operator ==.
////////////////////////////////////////////////////////////////////
INLINE bool ButtonHandle::
matches(const ButtonHandle &other) const {
  return ((*this) == other ||
          (other != ButtonHandle::none() &&
           get_alias() == other));
}

////////////////////////////////////////////////////////////////////
//     Function: ButtonHandle::get_index
//       Access: Published
//  Description: Returns the integer index associated with this
//               ButtonHandle. Each different ButtonHandle will have a
//               different index.  However, you probably shouldn't be
//               using this method; you should just treat the
//               ButtonHandles as opaque classes.  This is provided
//               for the convenience of non-C++ scripting languages to
//               build a hashtable of ButtonHandles.
////////////////////////////////////////////////////////////////////
INLINE int ButtonHandle::
get_index() const {
  return _index;
}

////////////////////////////////////////////////////////////////////
//     Function: ButtonHandle::output
//       Access: Published
//  Description: 
////////////////////////////////////////////////////////////////////
INLINE void ButtonHandle::
output(ostream &out) const {
  out << get_name();
}

////////////////////////////////////////////////////////////////////
//     Function: ButtonHandle::none
//       Access: Published, Static
//  Description: Returns a special zero-valued ButtonHandle that is
//               used to indicate no button.
////////////////////////////////////////////////////////////////////
INLINE ButtonHandle ButtonHandle::
none() {
  return _none;
}
