// Filename: hashVal.h
// Created by:  drose (14Nov00)
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
  INLINE bool operator == (const HashVal &other) const;
  INLINE uint get_value(int val) const;
  INLINE void set_value(int val, uint hash);
  INLINE void output(ostream &out) const;
  uint hv[4];
};

INLINE ostream &operator << (ostream &out, const HashVal &hv) {
  hv.output(out);
  return out;
}

#include "hashVal.I"

#endif
