// Filename: hashVal.h
// Created by:  drose (14Nov00)
//
////////////////////////////////////////////////////////////////////
//
// PANDA 3D SOFTWARE
// Copyright (c) 2001, Disney Enterprises, Inc.  All rights reserved
//
// All use of this software is subject to the terms of the Panda 3d
// Software license.  You should have received a copy of this license
// along with this source code; you will also find a current copy of
// the license at http://www.panda3d.org/license.txt .
//
// To contact the maintainers of this program write to
// panda3d@yahoogroups.com .
//
////////////////////////////////////////////////////////////////////

#ifndef HASHVAL_H
#define HASHVAL_H

#include <pandabase.h>
#include "typedef.h"
#include <notify.h>

////////////////////////////////////////////////////////////////////
//       Class : HashVal
// Description : A sixteen-byte hash value sent to the crypt library.
////////////////////////////////////////////////////////////////////
class EXPCL_PANDAEXPRESS HashVal {
PUBLISHED:
  INLINE HashVal();
  INLINE HashVal(const HashVal &copy);

  INLINE bool operator == (const HashVal &other) const;
  INLINE bool operator != (const HashVal &other) const;
  INLINE uint get_value(int val) const;
  INLINE void set_value(int val, uint hash);
  INLINE void output(ostream &out) const;
  string as_string() const;

public:
  uint hv[4];
};

INLINE ostream &operator << (ostream &out, const HashVal &hv) {
  out << "[";
  hv.output(out);
  out << "]";
  return out;
}

#include "hashVal.I"

#endif
