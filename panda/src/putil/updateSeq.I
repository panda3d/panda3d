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
UpdateSeq(const UpdateSeq &copy) : _seq(copy._seq.load(std::memory_order_relaxed)) {
}

/**
 *
 */
INLINE UpdateSeq::
UpdateSeq(const UpdateSeq &&from) noexcept : _seq(from._seq.load(std::memory_order_relaxed)) {
}

/**
 *
 */
INLINE UpdateSeq &UpdateSeq::
operator = (const UpdateSeq &copy) {
  _seq.store(copy._seq.load(std::memory_order_relaxed), std::memory_order_relaxed);
  return *this;
}

/**
 * Resets the UpdateSeq to the 'initial' state.
 */
INLINE void UpdateSeq::
clear() {
  _seq.store((unsigned int)SC_initial, std::memory_order_relaxed);
}

/**
 * Returns true if the UpdateSeq is in the 'initial' state.
 */
INLINE bool UpdateSeq::
is_initial() const {
  return _seq.load(std::memory_order_relaxed) == (unsigned int)SC_initial;
}

/**
 * Returns true if the UpdateSeq is in the 'old' state.
 */
INLINE bool UpdateSeq::
is_old() const {
  return _seq.load(std::memory_order_relaxed) == (unsigned int)SC_old;
}

/**
 * Returns true if the UpdateSeq is in the 'fresh' state.
 */
INLINE bool UpdateSeq::
is_fresh() const {
  return _seq.load(std::memory_order_relaxed) == (unsigned int)SC_fresh;
}

/**
 * Returns true if the UpdateSeq is in any special states, i.e.  'initial',
 * 'old', or 'fresh'.
 */
INLINE bool UpdateSeq::
is_special() const {
  // This relies on the assumption that (~0 + 1) == 0.
  return ((_seq.load(std::memory_order_relaxed) + 1u) <= 2u);
}

/**
 *
 */
INLINE bool UpdateSeq::
operator == (const UpdateSeq &other) const {
  return _seq.load(std::memory_order_relaxed) == other._seq.load(std::memory_order_relaxed);
}

/**
 *
 */
INLINE bool UpdateSeq::
operator != (const UpdateSeq &other) const {
  return _seq.load(std::memory_order_relaxed) != other._seq.load(std::memory_order_relaxed);
}

/**
 *
 */
INLINE bool UpdateSeq::
operator < (const UpdateSeq &other) const {
  return priv_lt(_seq.load(std::memory_order_relaxed), other._seq.load(std::memory_order_relaxed));
}

/**
 *
 */
INLINE bool UpdateSeq::
operator <= (const UpdateSeq &other) const {
  return priv_le(_seq.load(std::memory_order_relaxed), other._seq.load(std::memory_order_relaxed));
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
  unsigned int old_seq = _seq.load(std::memory_order_relaxed);
  unsigned int new_seq;
  do {
    new_seq = old_seq + 1;
    if (priv_is_special(new_seq)) {
      // Oops, wraparound.  We don't want to confuse the new value with our
      // special cases.
      new_seq = (unsigned int)SC_old + 1;
    }
  } while (!_seq.compare_exchange_weak(old_seq, new_seq,
                                       std::memory_order_relaxed,
                                       std::memory_order_relaxed));

  UpdateSeq temp;
  temp._seq.store(new_seq, std::memory_order_relaxed);
  return temp;
}

/**
 *
 */
INLINE UpdateSeq UpdateSeq::
operator ++ (int) {
  unsigned int old_seq = _seq.load(std::memory_order_relaxed);
  unsigned int new_seq;
  do {
    new_seq = old_seq + 1;
    if (priv_is_special(new_seq)) {
      // Oops, wraparound.  We don't want to confuse the new value with our
      // special cases.
      new_seq = (unsigned int)SC_old + 1;
    }
  } while (!_seq.compare_exchange_weak(old_seq, new_seq,
                                       std::memory_order_relaxed,
                                       std::memory_order_relaxed));

  UpdateSeq temp;
  temp._seq.store(old_seq, std::memory_order_relaxed);
  return temp;
}

/**
 * Returns the internal integer value associated with the UpdateSeq.  Useful
 * for debugging only.
 */
INLINE unsigned int UpdateSeq::
get_seq() const {
  return _seq.load(std::memory_order_relaxed);
}

/**
 *
 */
INLINE void UpdateSeq::
output(std::ostream &out) const {
  unsigned int seq = _seq.load(std::memory_order_relaxed);
  switch (seq) {
  case (unsigned int)SC_initial:
    out << "initial";
    break;

  case (unsigned int)SC_old:
    out << "old";
    break;

  case (unsigned int)SC_fresh:
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
priv_is_special(unsigned int seq) {
  // This relies on the assumption that (~0 + 1) == 0.
  return ((seq + 1) <= 2);
}

/**
 * The private implementation of operator < ().
 */
INLINE bool UpdateSeq::
priv_lt(unsigned int a, unsigned int b) {
  // The special cases of SC_initial or SC_old are less than all other non-
  // special numbers, and SC_initial is less than SC_old.  The special case of
  // SC_fresh is greater than all other non-special numbers.  For all other
  // cases, we use a circular comparision such that n < m iff (signed)(n - m)
  // < 0.
  return
    (priv_is_special(a) || priv_is_special(b)) ? (a < b) :
    ((signed int)(a - b) < 0);
}

/**
 * The private implementation of operator <= ().
 */
INLINE bool UpdateSeq::
priv_le(unsigned int a, unsigned int b) {
  return (a == b) || priv_lt(a, b);
}

INLINE std::ostream &operator << (std::ostream &out, const UpdateSeq &value) {
  value.output(out);
  return out;
}
