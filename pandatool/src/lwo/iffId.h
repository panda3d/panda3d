/**
 * PANDA 3D SOFTWARE
 * Copyright (c) Carnegie Mellon University.  All rights reserved.
 *
 * All use of this software is subject to the terms of the revised BSD
 * license.  You should have received a copy of this license along
 * with this source code in a file named "LICENSE."
 *
 * @file iffId.h
 * @author drose
 * @date 2001-04-23
 */

#ifndef IFFID_H
#define IFFID_H

#include "pandatoolbase.h"

#include "numeric_types.h"

/**
 * A four-byte chunk ID appearing in an "IFF" file.  This is used to identify
 * the meaning of each chunk, and can be treated either as a concrete object
 * or as a string, something like a TypeHandle.
 */
class IffId {
public:
  INLINE IffId();
  INLINE IffId(const char id[4]);
  INLINE IffId(const IffId &copy);
  INLINE void operator = (const IffId &copy);

  INLINE bool operator == (const IffId &other) const;
  INLINE bool operator != (const IffId &other) const;
  INLINE bool operator < (const IffId &other) const;

  INLINE std::string get_name() const;

  void output(std::ostream &out) const;

private:
  union {
    uint32_t _n;
    char _c[4];
  } _id;
};

#include "iffId.I"

INLINE std::ostream &operator << (std::ostream &out, const IffId &id) {
  id.output(out);
  return out;
}

#endif
