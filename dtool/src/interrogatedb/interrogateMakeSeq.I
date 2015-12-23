// Filename: interrogateMakeSeq.I
// Created by:  drose (15Sep09)
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
//     MakeSeq: InterrogateMakeSeq::Constructor
//       Access: Public
//  Description:
////////////////////////////////////////////////////////////////////
INLINE InterrogateMakeSeq::
InterrogateMakeSeq(InterrogateModuleDef *def) :
  InterrogateComponent(def)
{
  _length_getter = 0;
  _element_getter = 0;
}

////////////////////////////////////////////////////////////////////
//     MakeSeq: InterrogateMakeSeq::Copy Constructor
//       Access: Public
//  Description:
////////////////////////////////////////////////////////////////////
INLINE InterrogateMakeSeq::
InterrogateMakeSeq(const InterrogateMakeSeq &copy) {
  (*this) = copy;
}

////////////////////////////////////////////////////////////////////
//     MakeSeq: InterrogateMakeSeq::Copy Assignment Operator
//       Access: Public
//  Description:
////////////////////////////////////////////////////////////////////
INLINE void InterrogateMakeSeq::
operator = (const InterrogateMakeSeq &copy) {
  InterrogateComponent::operator = (copy);
  _scoped_name = copy._scoped_name;
  _comment = copy._comment;
  _length_getter = copy._length_getter;
  _element_getter = copy._element_getter;
}

////////////////////////////////////////////////////////////////////
//     Function: InterrogateMakeSeq::has_scoped_name
//       Access: Public
//  Description:
////////////////////////////////////////////////////////////////////
INLINE bool InterrogateMakeSeq::
has_scoped_name() const {
  return !_scoped_name.empty();
}

////////////////////////////////////////////////////////////////////
//     Function: InterrogateMakeSeq::get_scoped_name
//       Access: Public
//  Description:
////////////////////////////////////////////////////////////////////
INLINE const string &InterrogateMakeSeq::
get_scoped_name() const {
  return _scoped_name;
}

////////////////////////////////////////////////////////////////////
//     Function: InterrogateMakeSeq::has_comment
//       Access: Public
//  Description:
////////////////////////////////////////////////////////////////////
INLINE bool InterrogateMakeSeq::
has_comment() const {
  return !_comment.empty();
}

////////////////////////////////////////////////////////////////////
//     Function: InterrogateMakeSeq::get_comment
//       Access: Public
//  Description:
////////////////////////////////////////////////////////////////////
INLINE const string &InterrogateMakeSeq::
get_comment() const {
  return _comment;
}

////////////////////////////////////////////////////////////////////
//      Element: InterrogateMakeSeq::get_length_getter
//       Access: Public
//  Description:
////////////////////////////////////////////////////////////////////
INLINE FunctionIndex InterrogateMakeSeq::
get_length_getter() const {
  return _length_getter;
}

////////////////////////////////////////////////////////////////////
//      Element: InterrogateMakeSeq::get_element_getter
//       Access: Public
//  Description:
////////////////////////////////////////////////////////////////////
INLINE FunctionIndex InterrogateMakeSeq::
get_element_getter() const {
  return _element_getter;
}

INLINE ostream &
operator << (ostream &out, const InterrogateMakeSeq &make_seq) {
  make_seq.output(out);
  return out;
}

INLINE istream &
operator >> (istream &in, InterrogateMakeSeq &make_seq) {
  make_seq.input(in);
  return in;
}
