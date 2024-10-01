/**
 * PANDA 3D SOFTWARE
 * Copyright (c) Carnegie Mellon University.  All rights reserved.
 *
 * All use of this software is subject to the terms of the revised BSD
 * license.  You should have received a copy of this license along
 * with this source code in a file named "LICENSE."
 *
 * @file navTriVertGroup.h
 * @author Maxwell175
 * @date 2022-07-14
 */


#ifndef NAVTRIVERTGROUP_H
#define NAVTRIVERTGROUP_H

#include "pandabase.h"
#include "pandaSystem.h"
#include "luse.h"

class NavTriVertGroup {
PUBLISHED:
  INLINE NavTriVertGroup(LVector3 a, LVector3 b, LVector3 c);

  INLINE LVector3 get_a() const;
  INLINE LVector3 get_b() const;
  INLINE LVector3 get_c() const;

  INLINE void set_a(LVector3 a);
  INLINE void set_b(LVector3 b);
  INLINE void set_c(LVector3 c);

  MAKE_PROPERTY(a, get_a, set_a);
  MAKE_PROPERTY(b, get_b, set_b);
  MAKE_PROPERTY(c, get_c, set_c);

  INLINE bool operator==(const NavTriVertGroup &other) const;
  INLINE bool operator<(const NavTriVertGroup &other) const;

public:
  LVector3 a;
  LVector3 b;
  LVector3 c;
};

typedef pvector<NavTriVertGroup> NavTriVertGroups;

#include "navTriVertGroup.I"

#endif // NAVTRIVERTGROUP_H
