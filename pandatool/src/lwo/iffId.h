// Filename: iffId.h
// Created by:  drose (23Apr01)
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

#ifndef IFFID_H
#define IFFID_H

#include "pandatoolbase.h"

#include "numeric_types.h"

////////////////////////////////////////////////////////////////////
//       Class : IffId
// Description : A four-byte chunk ID appearing in an "IFF" file.
//               This is used to identify the meaning of each chunk,
//               and can be treated either as a concrete object or as
//               a string, something like a TypeHandle.
////////////////////////////////////////////////////////////////////
class IffId {
public:
  INLINE IffId();
  INLINE IffId(const char id[4]);
  INLINE IffId(const IffId &copy);
  INLINE void operator = (const IffId &copy);

  INLINE bool operator == (const IffId &other) const;
  INLINE bool operator != (const IffId &other) const;
  INLINE bool operator < (const IffId &other) const;

  INLINE string get_name() const;

  void output(ostream &out) const;

private:
  union {
    PN_uint32 _n;
    char _c[4];
  } _id;
};

#include "iffId.I"

INLINE ostream &operator << (ostream &out, const IffId &id) {
  id.output(out);
  return out;
}

#endif
