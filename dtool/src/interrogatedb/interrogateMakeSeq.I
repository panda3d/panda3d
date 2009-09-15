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
  _class = 0;
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
  _class = copy._class;
  _seq_name = copy._seq_name;
  _num_name = copy._num_name;
  _element_name = copy._element_name;
}

////////////////////////////////////////////////////////////////////
//     Function: InterrogateMakeSeq::get_class
//       Access: Public
//  Description: Return the class that owns the make_seq.
////////////////////////////////////////////////////////////////////
INLINE TypeIndex InterrogateMakeSeq::
get_class() const {
  return _class;
}

////////////////////////////////////////////////////////////////////
//     Function: InterrogateMakeSeq::get_seq_name
//       Access: Public
//  Description:
////////////////////////////////////////////////////////////////////
INLINE const string &InterrogateMakeSeq::
get_seq_name() const {
  return _seq_name;
}

////////////////////////////////////////////////////////////////////
//     Function: InterrogateMakeSeq::get_num_name
//       Access: Public
//  Description:
////////////////////////////////////////////////////////////////////
INLINE const string &InterrogateMakeSeq::
get_num_name() const {
  return _num_name;
}

////////////////////////////////////////////////////////////////////
//     Function: InterrogateMakeSeq::get_element_name
//       Access: Public
//  Description:
////////////////////////////////////////////////////////////////////
INLINE const string &InterrogateMakeSeq::
get_element_name() const {
  return _element_name;
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
