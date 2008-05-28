// Filename: interrogateManifest.I
// Created by:  drose (11Aug00)
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
//     Function: InterrogateManifest::Constructor
//       Access: Public
//  Description:
////////////////////////////////////////////////////////////////////
INLINE InterrogateManifest::
InterrogateManifest(InterrogateModuleDef *def) :
  InterrogateComponent(def)
{
  _flags = 0;
  _int_value = 0;
  _type = 0;
  _getter = 0;
}

////////////////////////////////////////////////////////////////////
//     Function: InterrogateManifest::Copy Constructor
//       Access: Public
//  Description:
////////////////////////////////////////////////////////////////////
INLINE InterrogateManifest::
InterrogateManifest(const InterrogateManifest &copy) {
  (*this) = copy;
}

////////////////////////////////////////////////////////////////////
//     Function: InterrogateManifest::Copy Assignment Operator
//       Access: Public
//  Description:
////////////////////////////////////////////////////////////////////
INLINE void InterrogateManifest::
operator = (const InterrogateManifest &copy) {
  InterrogateComponent::operator = (copy);
  _flags = copy._flags;
  _definition = copy._definition;
  _int_value = copy._int_value;
  _type = copy._type;
  _getter = copy._getter;
}


////////////////////////////////////////////////////////////////////
//     Function: InterrogateManifest::get_definition
//       Access: Public
//  Description:
////////////////////////////////////////////////////////////////////
INLINE const string &InterrogateManifest::
get_definition() const {
  return _definition;
}

////////////////////////////////////////////////////////////////////
//     Function: InterrogateManifest::has_type
//       Access: Public
//  Description:
////////////////////////////////////////////////////////////////////
INLINE bool InterrogateManifest::
has_type() const {
  return (_flags & F_has_type) != 0;
}

////////////////////////////////////////////////////////////////////
//     Function: InterrogateManifest::get_type
//       Access: Public
//  Description:
////////////////////////////////////////////////////////////////////
INLINE TypeIndex InterrogateManifest::
get_type() const {
  return _type;
}

////////////////////////////////////////////////////////////////////
//     Function: InterrogateManifest::has_getter
//       Access: Public
//  Description:
////////////////////////////////////////////////////////////////////
INLINE bool InterrogateManifest::
has_getter() const {
  return (_flags & F_has_getter) != 0;
}

////////////////////////////////////////////////////////////////////
//     Function: InterrogateManifest::get_getter
//       Access: Public
//  Description:
////////////////////////////////////////////////////////////////////
INLINE FunctionIndex InterrogateManifest::
get_getter() const {
  return _getter;
}

////////////////////////////////////////////////////////////////////
//     Function: InterrogateManifest::has_int_value
//       Access: Public
//  Description:
////////////////////////////////////////////////////////////////////
INLINE bool InterrogateManifest::
has_int_value() const {
  return (_flags & F_has_int_value) != 0;
}

////////////////////////////////////////////////////////////////////
//     Function: InterrogateManifest::get_int_value
//       Access: Public
//  Description:
////////////////////////////////////////////////////////////////////
INLINE int InterrogateManifest::
get_int_value() const {
  return _int_value;
}


INLINE ostream &
operator << (ostream &out, const InterrogateManifest &manifest) {
  manifest.output(out);
  return out;
}

INLINE istream &
operator >> (istream &in, InterrogateManifest &manifest) {
  manifest.input(in);
  return in;
}
