// Filename: namable.I
// Created by:  drose (16Feb00)
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
//     Function: Namable::Constructor
//       Access: Public
//  Description:
////////////////////////////////////////////////////////////////////
INLINE Namable::
Namable(const string &initial_name) :
  _name(initial_name)
{
}

////////////////////////////////////////////////////////////////////
//     Function: Namable::Copy Constructor
//       Access: Public
//  Description:
////////////////////////////////////////////////////////////////////
INLINE Namable::
Namable(const Namable &copy) :
  _name(copy._name)
{
}

////////////////////////////////////////////////////////////////////
//     Function: Namable::Copy Assignment Operator
//       Access: Public
//  Description:
////////////////////////////////////////////////////////////////////
INLINE Namable &Namable::
operator = (const Namable &other) {
  _name = other._name;
  return *this;
}

#ifdef USE_MOVE_SEMANTICS
////////////////////////////////////////////////////////////////////
//     Function: Namable::Copy Assignment Operator
//       Access: Public
//  Description:
////////////////////////////////////////////////////////////////////
INLINE Namable &Namable::
operator = (Namable &&other) NOEXCEPT {
  _name = MOVE(other._name);
  return *this;
}
#endif

////////////////////////////////////////////////////////////////////
//     Function: Namable::set_name
//       Access: Public
//  Description:
////////////////////////////////////////////////////////////////////
INLINE void Namable::
set_name(const string &name) {
  _name = name;
}

////////////////////////////////////////////////////////////////////
//     Function: Namable::clear_name
//       Access: Public
//  Description: Resets the Namable's name to empty.
////////////////////////////////////////////////////////////////////
INLINE void Namable::
clear_name() {
  _name = "";
}

////////////////////////////////////////////////////////////////////
//     Function: Namable::has_name
//       Access: Public
//  Description: Returns true if the Namable has a nonempty name set,
//               false if the name is empty.
////////////////////////////////////////////////////////////////////
INLINE bool Namable::
has_name() const {
  return !_name.empty();
}

////////////////////////////////////////////////////////////////////
//     Function: Namable::get_name
//       Access: Public
//  Description:
////////////////////////////////////////////////////////////////////
INLINE const string &Namable::
get_name() const {
  return _name;
}

////////////////////////////////////////////////////////////////////
//     Function: Namable::output
//       Access: Public
//  Description: Outputs the Namable.  This function simply writes the
//               name to the output stream; most Namable derivatives
//               will probably redefine this.
////////////////////////////////////////////////////////////////////
INLINE void Namable::
output(ostream &out) const {
  out << get_name();
}


INLINE ostream &operator << (ostream &out, const Namable &n) {
  n.output(out);
  return out;
}

////////////////////////////////////////////////////////////////////
//     Function: NamableOrderByName::Function operator
//       Access: Public
//  Description:
////////////////////////////////////////////////////////////////////
INLINE bool NamableOrderByName::
operator ()(const Namable *n1, const Namable *n2) const {
  return (n1->get_name() < n2->get_name());
}
