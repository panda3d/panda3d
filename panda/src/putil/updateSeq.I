// Filename: updateSeq.I
// Created by:  drose (30Sep99)
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
//     Function: UpdateSeq::Default Constructor
//       Access: Published
//  Description: Creates an UpdateSeq in the 'initial' state.
////////////////////////////////////////////////////////////////////
INLINE UpdateSeq::
UpdateSeq() {
  _seq = (unsigned int)SC_initial;
}

////////////////////////////////////////////////////////////////////
//     Function: UpdateSeq::initial (named constructor)
//       Access: Published, Static
//  Description: Returns an UpdateSeq in the 'initial' state.
////////////////////////////////////////////////////////////////////
INLINE UpdateSeq UpdateSeq::
initial() {
  return UpdateSeq();
}

////////////////////////////////////////////////////////////////////
//     Function: UpdateSeq::old (named constructor)
//       Access: Published, Static
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
//       Access: Published, Static
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
//       Access: Published
//  Description:
////////////////////////////////////////////////////////////////////
INLINE UpdateSeq::
UpdateSeq(const UpdateSeq &copy) {
  _seq = AtomicAdjust::get(copy._seq);
}

////////////////////////////////////////////////////////////////////
//     Function: UpdateSeq::Copy Assignment operator
//       Access: Published
//  Description:
////////////////////////////////////////////////////////////////////
INLINE UpdateSeq &UpdateSeq::
operator = (const UpdateSeq &copy) {
  AtomicAdjust::set(_seq, AtomicAdjust::get(copy._seq));
  return *this;
}

////////////////////////////////////////////////////////////////////
//     Function: UpdateSeq::clear
//       Access: Published
//  Description: Resets the UpdateSeq to the 'initial' state.
////////////////////////////////////////////////////////////////////
INLINE void UpdateSeq::
clear() {
  AtomicAdjust::set(_seq, (AtomicAdjust::Integer)SC_initial);
}

////////////////////////////////////////////////////////////////////
//     Function: UpdateSeq::is_initial
//       Access: Published
//  Description: Returns true if the UpdateSeq is in the 'initial'
//               state.
////////////////////////////////////////////////////////////////////
INLINE bool UpdateSeq::
is_initial() const {
  return AtomicAdjust::get(_seq) == (AtomicAdjust::Integer)SC_initial;
}

////////////////////////////////////////////////////////////////////
//     Function: UpdateSeq::is_old
//       Access: Published
//  Description: Returns true if the UpdateSeq is in the 'old' state.
////////////////////////////////////////////////////////////////////
INLINE bool UpdateSeq::
is_old() const {
  return AtomicAdjust::get(_seq) == (AtomicAdjust::Integer)SC_old;
}

////////////////////////////////////////////////////////////////////
//     Function: UpdateSeq::is_fresh
//       Access: Published
//  Description: Returns true if the UpdateSeq is in the 'fresh'
//               state.
////////////////////////////////////////////////////////////////////
INLINE bool UpdateSeq::
is_fresh() const {
  return AtomicAdjust::get(_seq) == (AtomicAdjust::Integer)SC_fresh;
}

////////////////////////////////////////////////////////////////////
//     Function: UpdateSeq::is_special
//       Access: Published
//  Description: Returns true if the UpdateSeq is in any special
//               states, i.e. 'initial', 'old', or 'fresh'.
////////////////////////////////////////////////////////////////////
INLINE bool UpdateSeq::
is_special() const {
  // This relies on the assumption that (~0 + 1) == 0.
  return ((AtomicAdjust::get(_seq) + 1) <= 2);
}

////////////////////////////////////////////////////////////////////
//     Function: UpdateSeq::Equality operator
//       Access: Published
//  Description:
////////////////////////////////////////////////////////////////////
INLINE bool UpdateSeq::
operator == (const UpdateSeq &other) const {
  return AtomicAdjust::get(_seq) == AtomicAdjust::get(other._seq);
}

////////////////////////////////////////////////////////////////////
//     Function: UpdateSeq::Inequality operator
//       Access: Published
//  Description:
////////////////////////////////////////////////////////////////////
INLINE bool UpdateSeq::
operator != (const UpdateSeq &other) const {
  return AtomicAdjust::get(_seq) != AtomicAdjust::get(other._seq);
}

////////////////////////////////////////////////////////////////////
//     Function: UpdateSeq::Comparison operator
//       Access: Published
//  Description:
////////////////////////////////////////////////////////////////////
INLINE bool UpdateSeq::
operator < (const UpdateSeq &other) const {
  return priv_lt(AtomicAdjust::get(_seq), AtomicAdjust::get(other._seq));
}

////////////////////////////////////////////////////////////////////
//     Function: UpdateSeq::Comparison operator
//       Access: Published
//  Description:
////////////////////////////////////////////////////////////////////
INLINE bool UpdateSeq::
operator <= (const UpdateSeq &other) const {
  return priv_le(AtomicAdjust::get(_seq), AtomicAdjust::get(other._seq));
}

////////////////////////////////////////////////////////////////////
//     Function: UpdateSeq::Comparison operator
//       Access: Published
//  Description:
////////////////////////////////////////////////////////////////////
INLINE bool UpdateSeq::
operator > (const UpdateSeq &other) const {
  return (other < (*this));
}

////////////////////////////////////////////////////////////////////
//     Function: UpdateSeq::Comparison operator
//       Access: Published
//  Description:
////////////////////////////////////////////////////////////////////
INLINE bool UpdateSeq::
operator >= (const UpdateSeq &other) const {
  return (other <= (*this));
}

////////////////////////////////////////////////////////////////////
//     Function: UpdateSeq::Preincrement operator
//       Access: Published
//  Description:
////////////////////////////////////////////////////////////////////
INLINE UpdateSeq UpdateSeq::
operator ++ () {
  AtomicAdjust::Integer old_seq = AtomicAdjust::get(_seq);
  AtomicAdjust::Integer new_seq = old_seq + 1;
  if (priv_is_special(new_seq)) {
    // Oops, wraparound.  We don't want to confuse the new value
    // with our special cases.
    new_seq = (AtomicAdjust::Integer)SC_old + 1;
  }

#ifdef HAVE_THREADS
  AtomicAdjust::Integer result = AtomicAdjust::compare_and_exchange(_seq, old_seq, new_seq);
  while (result != old_seq) {
    // Some other thread beat us to it; try again.
    old_seq = AtomicAdjust::get(_seq);
    new_seq = old_seq + 1;
    if (priv_is_special(new_seq)) {
      // Oops, wraparound.  We don't want to confuse the new value
      // with our special cases.
      new_seq = (AtomicAdjust::Integer)SC_old + 1;
    }
    result = AtomicAdjust::compare_and_exchange(_seq, old_seq, new_seq);
  }
#else
  _seq = new_seq;
#endif  // HAVE_THREADS

  return *this;
}

////////////////////////////////////////////////////////////////////
//     Function: UpdateSeq::Postincrement operator
//       Access: Published
//  Description:
////////////////////////////////////////////////////////////////////
INLINE UpdateSeq UpdateSeq::
operator ++ (int) {
  AtomicAdjust::Integer old_seq = AtomicAdjust::get(_seq);
  AtomicAdjust::Integer new_seq = old_seq + 1;
  if (priv_is_special(new_seq)) {
    // Oops, wraparound.  We don't want to confuse the new value
    // with our special cases.
    new_seq = (AtomicAdjust::Integer)SC_old + 1;
  }

#ifdef HAVE_THREADS
  AtomicAdjust::Integer result = AtomicAdjust::compare_and_exchange(_seq, old_seq, new_seq);
  while (result != old_seq) {
    // Some other thread beat us to it; try again.
    old_seq = AtomicAdjust::get(_seq);
    new_seq = old_seq + 1;
    if (priv_is_special(new_seq)) {
      // Oops, wraparound.  We don't want to confuse the new value
      // with our special cases.
      new_seq = (AtomicAdjust::Integer)SC_old + 1;
    }
    result = AtomicAdjust::compare_and_exchange(_seq, old_seq, new_seq);
  }
#else
  _seq = new_seq;
#endif  // HAVE_THREADS

  UpdateSeq temp;
  temp._seq = old_seq;
  return temp;
}

////////////////////////////////////////////////////////////////////
//     Function: UpdateSeq::get_seq
//       Access: Published
//  Description: Returns the internal integer value associated with
//               the UpdateSeq.  Useful for debugging only.
////////////////////////////////////////////////////////////////////
INLINE AtomicAdjust::Integer UpdateSeq::
get_seq() const {
  return _seq;
}

////////////////////////////////////////////////////////////////////
//     Function: UpdateSeq::output
//       Access: Published
//  Description:
////////////////////////////////////////////////////////////////////
INLINE void UpdateSeq::
output(ostream &out) const {
  AtomicAdjust::Integer seq = AtomicAdjust::get(_seq);
  switch (seq) {
  case (AtomicAdjust::Integer)SC_initial:
    out << "initial";
    break;

  case (AtomicAdjust::Integer)SC_old:
    out << "old";
    break;

  case (AtomicAdjust::Integer)SC_fresh:
    out << "fresh";
    break;

  default:
    out << (int)seq;
  }
}

////////////////////////////////////////////////////////////////////
//     Function: UpdateSeq::priv_is_special
//       Access: Private, Static
//  Description: The private implementation of is_special().
////////////////////////////////////////////////////////////////////
INLINE bool UpdateSeq::
priv_is_special(AtomicAdjust::Integer seq) {
  // This relies on the assumption that (~0 + 1) == 0.
  return (((unsigned int)seq + 1) <= 2);
}

////////////////////////////////////////////////////////////////////
//     Function: UpdateSeq::priv_lt
//       Access: Private, Static
//  Description: The private implementation of operator < ().
////////////////////////////////////////////////////////////////////
INLINE bool UpdateSeq::
priv_lt(AtomicAdjust::Integer a, AtomicAdjust::Integer b) {
  // The special cases of SC_initial or SC_old are less than all other
  // non-special numbers, and SC_initial is less than SC_old.  The
  // special case of SC_fresh is greater than all other non-special
  // numbers.  For all other cases, we use a circular comparision such
  // that n < m iff (signed)(n - m) < 0.
  return
    (priv_is_special(a) || priv_is_special(b)) ? ((unsigned int)a < (unsigned int)b) :
    ((signed int)(a - b) < 0);
}

////////////////////////////////////////////////////////////////////
//     Function: UpdateSeq::priv_le
//       Access: Private, Static
//  Description: The private implementation of operator <= ().
////////////////////////////////////////////////////////////////////
INLINE bool UpdateSeq::
priv_le(AtomicAdjust::Integer a, AtomicAdjust::Integer b) {
  return (a == b) || priv_lt(a, b);
}

INLINE ostream &operator << (ostream &out, const UpdateSeq &value) {
  value.output(out);
  return out;
}
