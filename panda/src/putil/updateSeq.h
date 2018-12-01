/**
 * PANDA 3D SOFTWARE
 * Copyright (c) Carnegie Mellon University.  All rights reserved.
 *
 * All use of this software is subject to the terms of the revised BSD
 * license.  You should have received a copy of this license along
 * with this source code in a file named "LICENSE."
 *
 * @file updateSeq.h
 * @author drose
 * @date 1999-09-30
 */

#ifndef UPDATE_SEQ
#define UPDATE_SEQ

#include "pandabase.h"
#include "pmutex.h"
#include "mutexHolder.h"
#include "atomicAdjust.h"
#include "numeric_types.h"

/**
 * This is a sequence number that increments monotonically.  It can be used to
 * track cache updates, or serve as a kind of timestamp for any changing
 * properties.
 *
 * A special class is used instead of simply an int, so we can elegantly
 * handle such things as wraparound and special cases.  There are two special
 * cases.  Firstly, a sequence number is 'initial' when it is first created.
 * This sequence is older than any other sequence number.  Secondly, a
 * sequence number may be explicitly set to 'old'.  This is older than any
 * other sequence number except 'initial'.  Finally, we have the explicit
 * number 'fresh', which is newer than any other sequence number.  All other
 * sequences are numeric and are monotonically increasing.
 */
class EXPCL_PANDA_PUTIL UpdateSeq {
private:
  constexpr UpdateSeq(unsigned int seq);

PUBLISHED:
  constexpr UpdateSeq();
  constexpr static UpdateSeq initial() { return UpdateSeq(SC_initial); }
  constexpr static UpdateSeq old() { return UpdateSeq(SC_old); }
  constexpr static UpdateSeq fresh() { return UpdateSeq(SC_fresh); }

  INLINE UpdateSeq(const UpdateSeq &copy);
  constexpr UpdateSeq(const UpdateSeq &&from) noexcept;
  INLINE UpdateSeq &operator = (const UpdateSeq &copy);

  INLINE void clear();

  INLINE bool is_initial() const;
  INLINE bool is_old() const;
  INLINE bool is_fresh() const;
  INLINE bool is_special() const;

  INLINE bool operator == (const UpdateSeq &other) const;
  INLINE bool operator != (const UpdateSeq &other) const;
  INLINE bool operator < (const UpdateSeq &other) const;
  INLINE bool operator <= (const UpdateSeq &other) const;
  INLINE bool operator > (const UpdateSeq &other) const;
  INLINE bool operator >= (const UpdateSeq &other) const;

  INLINE UpdateSeq operator ++ ();
  INLINE UpdateSeq operator ++ (int);

  INLINE AtomicAdjust::Integer get_seq() const;
  MAKE_PROPERTY(seq, get_seq);

  INLINE void output(std::ostream &out) const;

private:
  INLINE static bool priv_is_special(AtomicAdjust::Integer seq);
  INLINE static bool priv_lt(AtomicAdjust::Integer a, AtomicAdjust::Integer b);
  INLINE static bool priv_le(AtomicAdjust::Integer a, AtomicAdjust::Integer b);

private:
  enum SpecialCases : unsigned int {
    SC_initial = 0,
    SC_old = 1,
    SC_fresh = ~(unsigned int)0,
  };

  AtomicAdjust::Integer _seq;
};

INLINE std::ostream &operator << (std::ostream &out, const UpdateSeq &value);

#include "updateSeq.I"

#endif
