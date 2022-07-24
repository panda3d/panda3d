/**
 * PANDA 3D SOFTWARE
 * Copyright (c) Carnegie Mellon University.  All rights reserved.
 *
 * All use of this software is subject to the terms of the revised BSD
 * license.  You should have received a copy of this license along
 * with this source code in a file named "LICENSE."
 *
 * @file navTrackedCollInfo.h
 * @author Maxwell175
 * @date 2022-07-23
 */


#ifndef NAVTRACKEDCOLLINFO_H
#define NAVTRACKEDCOLLINFO_H

#include "pandabase.h"
#include "nodePath.h"

class NavTrackedCollInfo {
PUBLISHED:
  INLINE NavTrackedCollInfo(NodePath node, BitMask32 mask);

  INLINE NodePath get_node() const;
  INLINE BitMask32 get_mask() const;

  MAKE_PROPERTY(node, get_node);
  MAKE_PROPERTY(mask, get_mask);

  INLINE bool operator==(const NavTrackedCollInfo &other) const;
  INLINE bool operator>(const NavTrackedCollInfo &other) const;
  INLINE bool operator<(const NavTrackedCollInfo &other) const;

public:
  NodePath node;
  BitMask32 mask;
};

typedef pvector<NavTrackedCollInfo> TrackedCollInfos;

#include "navTrackedCollInfo.I"

#endif // NAVTRACKEDCOLLINFO_H
