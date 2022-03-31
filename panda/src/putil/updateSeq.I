/**
 * PANDA 3D SOFTWARE
 * Copyright (c) Carnegie Mellon University.  All rights reserved.
 *
 * All use of this software is subject to the terms of the revised BSD
 * license.  You should have received a copy of this license along
 * with this source code in a file named "LICENSE."
 *
 * @file updateSeq.I
 * @author drose
 * @date 1999-09-30
 */

/**
 * Creates an UpdateSeq in the given state.
 */
constexpr UpdateSeq::
UpdateSeq(unsigned int seq) : _seq(seq) {
}

/**
 * Creates an UpdateSeq in the 'initial' state.
 */
constexpr UpdateSeq::
UpdateSeq() : _seq((unsigned int)SC_initial) {
}

/**
 *
 */
INLINE UpdateSeq::
UpdateSeq(const UpdateSeq &copy) : _seq(AtomicAdjust::get(copy._seq)) {
}

/**
 *
 */
constexpr UpdateSeq::
UpdateSeq(const UpdateSeq &&from) noexcept : _seq(from._seq) {
}

/**
 *
 */
INLINE UpdateSeq &UpdateSeq::
operator = (const UpdateSeq &copy) {
  AtomicAdjust::set(_seq, AtomicAdjust::get(copy._seq));
  return *this;
}

/**
 * Resets the UpdateSeq to the 'initial' state.
 */
INLINE void UpdateSeq::
clear() {
  AtomicAdjust::set(_seq, (AtomicAdjust::Integer)SC_initial);
}

/**
 * Returns true if the UpdateSeq is in the 'initial' state.
 */
INLINE bool UpdateSeq::
is_initial() const {
  return AtomicAdjust::get(_seq) == (AtomicAdjust::Integer)SC_initial;
}

/**
 * Returns true if the UpdateSeq is in the 'old' state.
 */
INLINE bool UpdateSeq::
is_old() const {
  return AtomicAdjust::get(_seq) == (AtomicAdjust::Integer)SC_old;
}

/**
 * Returns true if the UpdateSeq is in the 'fresh' state.
 */
INLINE bool UpdateSeq::
is_fresh() const {
  return AtomicAdjust::get(_seq) == (AtomicAdjust::Integer)SC_fresh;
}

/**
 * Returns true if the UpdateSeq is in any special states, i.e.  'initial',
 * 'old', or 'fresh'.
 */
INLINE bool UpdateSeq::
is_special() const {
  // This relies on the assumption that (~0 + 1) == 0.
  return (((unsigned int)AtomicAdjust::get(_seq) + 1u) <= 2u);
}

/**
 *
 */
INLINE bool UpdateSeq::
operator == (const UpdateSeq &other) const {
  return AtomicAdjust::get(_seq) == AtomicAdjust::get(other._seq);
}

/**
 *
 */
INLINE bool UpdateSeq::
operator != (const UpdateSeq &other) const {
  return AtomicAdjust::get(_seq) != AtomicAdjust::get(other._seq);
}

/**
 *
 */
INLINE bool UpdateSeq::
operator < (const UpdateSeq &other) const {
  return priv_lt(AtomicAdjust::get(_seq), AtomicAdjust::get(other._seq));
}

/**
 *
 */
INLINE bool UpdateSeq::
operator <= (const UpdateSeq &other) const {
  return priv_le(AtomicAdjust::get(_seq), AtomicAdjust::get(other._seq));
}

/**
 *
 */
INLINE bool UpdateSeq::
operator > (const UpdateSeq &other) const {
  return (other < (*this));
}

/**
 *
 */
INLINE bool UpdateSeq::
operator >= (const UpdateSeq &other) const {
  return (other <= (*this));
}

/**
 *
 */
INLINE UpdateSeq UpdateSeq::
operator ++ () {
  AtomicAdjust::Integer old_seq = AtomicAdjust::get(_seq);
  AtomicAdjust::Integer new_seq = old_seq + 1;
  if (priv_is_special(new_seq)) {
    // Oops, wraparound.  We don't want to confuse the new value with our
    // special cases.
    new_seq = (AtomicAdjust::Integer)SC_old + 1;
  }

#ifdef HAVE_THREADS
  AtomicAdjust::Integer result = AtomicAdjust::compare_and_exchange(_seq, old_seq, new_seq);
  while (result != old_seq) {
    // Some other thread beat us to it; try again.
    old_seq = AtomicAdjust::get(_seq);
    new_seq = old_seq + 1;
    if (priv_is_special(new_seq)) {
      // Oops, wraparound.  We don't want to confuse the new value with our
      // special cases.
      new_seq = (AtomicAdjust::Integer)SC_old + 1;
    }
    result = AtomicAdjust::compare_and_exchange(_seq, old_seq, new_seq);
  }
#else
  _seq = new_seq;
#endif  // HAVE_THREADS

  return *this;
}

/**
 *
 */
INLINE UpdateSeq UpdateSeq::
operator ++ (int) {
  AtomicAdjust::Integer old_seq = AtomicAdjust::get(_seq);
  AtomicAdjust::Integer new_seq = old_seq + 1;
  if (priv_is_special(new_seq)) {
    // Oops, wraparound.  We don't want to confuse the new value with our
    // special cases.
    new_seq = (AtomicAdjust::Integer)SC_old + 1;
  }

#ifdef HAVE_THREADS
  AtomicAdjust::Integer result = AtomicAdjust::compare_and_exchange(_seq, old_seq, new_seq);
  while (result != old_seq) {
    // Some other thread beat us to it; try again.
    old_seq = AtomicAdjust::get(_seq);
    new_seq = old_seq + 1;
    if (priv_is_special(new_seq)) {
      // Oops, wraparound.  We don't want to confuse the new value with our
      // special cases.
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

/**
 * Returns the internal integer value associated with the UpdateSeq.  Useful
 * for debugging only.
 */
INLINE AtomicAdjust::Integer UpdateSeq::
get_seq() const {
  return _seq;
}

/**
 *
 */
INLINE void UpdateSeq::
output(std::ostream &out) const {
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

/**
 * The private implementation of is_special().
 */
INLINE bool UpdateSeq::
priv_is_special(AtomicAdjust::Integer seq) {
  // This relies on the assumption that (~0 + 1) == 0.
  return (((unsigned int)seq + 1) <= 2);
}

/**
 * The private implementation of operator < ().
 */
INLINE bool UpdateSeq::
priv_lt(AtomicAdjust::Integer a, AtomicAdjust::Integer b) {
  // The special cases of SC_initial or SC_old are less than all other non-
  // special numbers, and SC_initial is less than SC_old.  The special case of
  // SC_fresh is greater than all other non-special numbers.  For all other
  // cases, we use a circular comparision such that n < m iff (signed)(n - m)
  // < 0.
  return
    (priv_is_special(a) || priv_is_special(b)) ? ((unsigned int)a < (unsigned int)b) :
    ((signed int)(a - b) < 0);
}

/**
 * The private implementation of operator <= ().
 */
INLINE bool UpdateSeq::
priv_le(AtomicAdjust::Integer a, AtomicAdjust::Integer b) {
  return (a == b) || priv_lt(a, b);
}

INLINE std::ostream &operator << (std::ostream &out, const UpdateSeq &value) {
  value.output(out);
  return out;
}
