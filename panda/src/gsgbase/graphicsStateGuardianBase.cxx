/**
 * PANDA 3D SOFTWARE
 * Copyright (c) Carnegie Mellon University.  All rights reserved.
 *
 * All use of this software is subject to the terms of the revised BSD
 * license.  You should have received a copy of this license along
 * with this source code in a file named "LICENSE."
 *
 * @file graphicsStateGuardianBase.cxx
 * @author drose
 * @date 1999-10-06
 */

#include "graphicsStateGuardianBase.h"
#include "lightMutexHolder.h"
#include <algorithm>

GraphicsStateGuardianBase::GSGs GraphicsStateGuardianBase::_gsgs;
GraphicsStateGuardianBase *GraphicsStateGuardianBase::_default_gsg;
LightMutex GraphicsStateGuardianBase::_lock;
TypeHandle GraphicsStateGuardianBase::_type_handle;

/**
 * Returns a pointer to the "default" GSG.  This is typically the first GSG
 * created in an application; in a single-window application, it will be the
 * only GSG. This GSG is used to determine default optimization choices for
 * loaded geometry.  The return value may be NULL if a GSG has not been created.
 */
GraphicsStateGuardianBase *GraphicsStateGuardianBase::
get_default_gsg() {
  LightMutexHolder holder(_lock);
  return _default_gsg;
}

/**
 * Specifies a particular GSG to use as the "default" GSG.  See
 * get_default_gsg().
 */
void GraphicsStateGuardianBase::
set_default_gsg(GraphicsStateGuardianBase *default_gsg) {
  LightMutexHolder holder(_lock);
  if (find(_gsgs.begin(), _gsgs.end(), default_gsg) == _gsgs.end()) {
    // The specified GSG doesn't exist or it has already destructed.
    nassertv(false);
    return;
  }

  _default_gsg = default_gsg;
}

/**
 * Returns the total number of GSG's in the universe.
 */
int GraphicsStateGuardianBase::
get_num_gsgs() {
  return _gsgs.size();
}

/**
 * Returns the nth GSG in the universe.  GSG's automatically add themselves and
 * remove themselves from this list as they are created and destroyed.
 */
GraphicsStateGuardianBase *GraphicsStateGuardianBase::
get_gsg(int n) {
  nassertr(n >= 0 && n < (int)_gsgs.size(), NULL);
  return _gsgs[n];
}

/**
 * Called by a GSG after it has been initialized, to add a new GSG to the
 * available list.
 */
void GraphicsStateGuardianBase::
add_gsg(GraphicsStateGuardianBase *gsg) {
  LightMutexHolder holder(_lock);

  if (find(_gsgs.begin(), _gsgs.end(), gsg) != _gsgs.end()) {
    // Already on the list.
    return;
  }

  _gsgs.push_back(gsg);

  if (_default_gsg == (GraphicsStateGuardianBase *)NULL) {
    _default_gsg = gsg;
  }
}

/**
 * Called by a GSG destructor to remove a GSG from the available list.
 */
void GraphicsStateGuardianBase::
remove_gsg(GraphicsStateGuardianBase *gsg) {
  LightMutexHolder holder(_lock);

  GSGs::iterator gi = find(_gsgs.begin(), _gsgs.end(), gsg);
  if (gi == _gsgs.end()) {
    // Already removed, or never added.
    return;
  }

  _gsgs.erase(gi);

  if (_default_gsg == gsg) {
    if (!_gsgs.empty()) {
      _default_gsg = *_gsgs.begin();
    } else {
      _default_gsg = NULL;
    }
  }
}
