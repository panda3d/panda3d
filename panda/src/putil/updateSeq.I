// Filename: updateSeq.I
// Created by:  drose (30Sep99)
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
//     Function: UpdateSeq::Default Constructor
//       Access: Public
//  Description: Creates an UpdateSeq in the 'initial' state.
////////////////////////////////////////////////////////////////////
INLINE UpdateSeq::
UpdateSeq() {
  _seq = (unsigned int)SC_initial;
}

////////////////////////////////////////////////////////////////////
//     Function: UpdateSeq::initial (named constructor)
//       Access: Public, Static
//  Description: Returns an UpdateSeq in the 'initial' state.
////////////////////////////////////////////////////////////////////
INLINE UpdateSeq UpdateSeq::
initial() {
  return UpdateSeq();
}

////////////////////////////////////////////////////////////////////
//     Function: UpdateSeq::old (named constructor)
//       Access: Public, Static
//  Description: Returns an UpdateSeq in the 'old' state.
////////////////////////////////////////////////////////////////////
INLINE UpdateSeq UpdateSeq::
old() {
  UpdateSeq result;
  result._seq = (unsigned int)SC_old;
  return result;
}

////////////////////////////////////////////////////////////////////
//     Function: UpdateSeq::fresh (named constructor)
//       Access: Public, Static
//  Description: Returns an UpdateSeq in the 'fresh' state.
////////////////////////////////////////////////////////////////////
INLINE UpdateSeq UpdateSeq::
fresh() {
  UpdateSeq result;
  result._seq = (unsigned int)SC_fresh;
  return result;
}

////////////////////////////////////////////////////////////////////
//     Function: UpdateSeq::Copy Constructor
//       Access: Public
//  Description:
////////////////////////////////////////////////////////////////////
INLINE UpdateSeq::
UpdateSeq(const UpdateSeq &copy) {
  _seq = copy._seq;
}

////////////////////////////////////////////////////////////////////
//     Function: UpdateSeq::Copy Assignment operator
//       Access: Public
//  Description:
////////////////////////////////////////////////////////////////////
INLINE UpdateSeq &UpdateSeq::
operator = (const UpdateSeq &copy) {
  _seq = copy._seq;
  return *this;
}

////////////////////////////////////////////////////////////////////
//     Function: UpdateSeq::clear
//       Access: Public
//  Description: Resets the UpdateSeq to the 'initial' state.
////////////////////////////////////////////////////////////////////
INLINE void UpdateSeq::
clear() {
  _seq = (unsigned int)SC_initial;
}

////////////////////////////////////////////////////////////////////
//     Function: UpdateSeq::is_initial
//       Access: Public
//  Description: Returns true if the UpdateSeq is in the 'initial'
//               state.
////////////////////////////////////////////////////////////////////
INLINE bool UpdateSeq::
is_initial() const {
  return _seq == (unsigned int)SC_initial;
}

////////////////////////////////////////////////////////////////////
//     Function: UpdateSeq::is_old
//       Access: Public
//  Description: Returns true if the UpdateSeq is in the 'old' state.
////////////////////////////////////////////////////////////////////
INLINE bool UpdateSeq::
is_old() const {
  return _seq == (unsigned int)SC_old;
}

////////////////////////////////////////////////////////////////////
//     Function: UpdateSeq::is_fresh
//       Access: Public
//  Description: Returns true if the UpdateSeq is in the 'fresh'
//               state.
////////////////////////////////////////////////////////////////////
INLINE bool UpdateSeq::
is_fresh() const {
  return _seq == (unsigned int)SC_fresh;
}

////////////////////////////////////////////////////////////////////
//     Function: UpdateSeq::is_special
//       Access: Public
//  Description: Returns true if the UpdateSeq is in any special
//               states, i.e. 'initial', 'old', or 'fresh'.
////////////////////////////////////////////////////////////////////
INLINE bool UpdateSeq::
is_special() const {
  switch (_seq) {
  case (unsigned int)SC_initial:
  case (unsigned int)SC_old:
  case (unsigned int)SC_fresh:
    return true;

  default:
    return false;
  }
}

////////////////////////////////////////////////////////////////////
//     Function: UpdateSeq::Equality operator
//       Access: Public
//  Description:
////////////////////////////////////////////////////////////////////
INLINE bool UpdateSeq::
operator == (const UpdateSeq &other) const {
  return (_seq == other._seq);
}

////////////////////////////////////////////////////////////////////
//     Function: UpdateSeq::Inequality operator
//       Access: Public
//  Description:
////////////////////////////////////////////////////////////////////
INLINE bool UpdateSeq::
operator != (const UpdateSeq &other) const {
  return (_seq != other._seq);
}

////////////////////////////////////////////////////////////////////
//     Function: UpdateSeq::Comparison operator
//       Access: Public
//  Description:
////////////////////////////////////////////////////////////////////
INLINE bool UpdateSeq::
operator < (const UpdateSeq &other) const {
  // The special cases of SC_initial or SC_old are less than all other
  // non-special numbers, and SC_initial is less than SC_old.  The
  // special case of SC_fresh is greater than all other non-special
  // numbers.  For all other cases, we use a circular comparision such
  // that n < m iff (signed)(n - m) < 0.
  return
    (is_special() || other.is_special()) ? (_seq < other._seq) :
    ((signed int)(_seq - other._seq) < 0);
}

////////////////////////////////////////////////////////////////////
//     Function: UpdateSeq::Comparison operator
//       Access: Public
//  Description:
////////////////////////////////////////////////////////////////////
INLINE bool UpdateSeq::
operator <= (const UpdateSeq &other) const {
  return (*this) == other || (*this) < other;
}

////////////////////////////////////////////////////////////////////
//     Function: UpdateSeq::Preincrement operator
//       Access: Public
//  Description:
////////////////////////////////////////////////////////////////////
INLINE UpdateSeq UpdateSeq::
operator ++ () {
  ++_seq;
  if (is_special()) {
    // Oops, wraparound.  We don't want to confuse the new value
    // with our special cases.
    _seq = (unsigned int)SC_old + 1;
  }

  return *this;
}

////////////////////////////////////////////////////////////////////
//     Function: UpdateSeq::Postincrement operator
//       Access: Public
//  Description:
////////////////////////////////////////////////////////////////////
INLINE UpdateSeq UpdateSeq::
operator ++ (int) {
  UpdateSeq temp = (*this);
  operator ++ ();
  return temp;
}

////////////////////////////////////////////////////////////////////
//     Function: UpdateSeq::output
//       Access: Public
//  Description:
////////////////////////////////////////////////////////////////////
INLINE void UpdateSeq::
output(ostream &out) const {
  switch (_seq) {
  case SC_initial:
    out << "initial";
    break;

  case SC_old:
    out << "old";
    break;

  case SC_fresh:
    out << "fresh";
    break;

  default:
    out << _seq;
  }
}

INLINE ostream &operator << (ostream &out, const UpdateSeq &value) {
  value.output(out);
  return out;
}
