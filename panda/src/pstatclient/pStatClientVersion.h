// Filename: pStatClientVersion.h
// Created by:  drose (21May01)
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

#ifndef PSTATCLIENTVERSION_H
#define PSTATCLIENTVERSION_H

#include "pandabase.h"

#include <referenceCount.h>
#include "pointerTo.h"

////////////////////////////////////////////////////////////////////
//       Class : PStatClientVersion
// Description : Records the version number of a particular client.
//               Normally this will be the same as
//               get_current_pstat_major/minor_version().
////////////////////////////////////////////////////////////////////
class EXPCL_PANDA PStatClientVersion : public ReferenceCount {
public:
  PStatClientVersion();

  INLINE int get_major_version() const;
  INLINE int get_minor_version() const;

  INLINE void set_version(int major_version, int minor_version);

  INLINE bool is_at_least(int major_version, int minor_version) const;

private:
  int _major_version;
  int _minor_version;
};

#include "pStatClientVersion.I"

#endif

