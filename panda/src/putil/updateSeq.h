// Filename: updateSeq.h
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

#ifndef UPDATE_SEQ
#define UPDATE_SEQ

#include "pandabase.h"

////////////////////////////////////////////////////////////////////
//       Class : UpdateSeq
// Description : This is a sequence number that increments
//               monotonically.  It can be used to track cache
//               updates, or serve as a kind of timestamp for any
//               changing properties.
//
//               A special class is used instead of simply an int, so
//               we can elegantly handle such things as wraparound and
//               special cases.  There are two special cases.
//               Firstly, a sequence number is 'initial' when it is
//               first created.  This sequence is older than any other
//               sequence number.  Secondly, a sequence number may be
//               explicitly set to 'old'.  This is older than any
//               other sequence number except 'initial'.  Finally, we
//               have the explicit number 'fresh', which is newer
//               than any other sequence number.  All other sequences
//               are numeric and are monotonically increasing.
////////////////////////////////////////////////////////////////////
class EXPCL_PANDA UpdateSeq {
PUBLISHED:
  INLINE UpdateSeq();
  INLINE static UpdateSeq initial();
  INLINE static UpdateSeq old();
  INLINE static UpdateSeq fresh();

  INLINE UpdateSeq(const UpdateSeq &copy);
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

  INLINE void output(ostream &out) const;

private:
  enum SpecialCases {
    SC_initial = 0,
    SC_old = 1,
    SC_fresh = ~(unsigned int)0,
  };

  unsigned int _seq;
};

INLINE ostream &operator << (ostream &out, const UpdateSeq &value);

#include "updateSeq.I"

#endif
