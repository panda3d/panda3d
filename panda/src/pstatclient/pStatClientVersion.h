/**
 * PANDA 3D SOFTWARE
 * Copyright (c) Carnegie Mellon University.  All rights reserved.
 *
 * All use of this software is subject to the terms of the revised BSD
 * license.  You should have received a copy of this license along
 * with this source code in a file named "LICENSE."
 *
 * @file pStatClientVersion.h
 * @author drose
 * @date 2001-05-21
 */

#ifndef PSTATCLIENTVERSION_H
#define PSTATCLIENTVERSION_H

#include "pandabase.h"

#include "referenceCount.h"
#include "pointerTo.h"

/**
 * Records the version number of a particular client.  Normally this will be
 * the same as get_current_pstat_major/minor_version().
 */
class EXPCL_PANDA_PSTATCLIENT PStatClientVersion : public ReferenceCount {
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
