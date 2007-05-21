// Filename: graphicsStateGuardianBase.cxx
// Created by:  drose (06Oct99)
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

#include "graphicsStateGuardianBase.h"
#include "mutexHolder.h"
#include <algorithm>

GraphicsStateGuardianBase::GSGs GraphicsStateGuardianBase::_gsgs;
GraphicsStateGuardianBase *GraphicsStateGuardianBase::_default_gsg;
TypeHandle GraphicsStateGuardianBase::_type_handle;

////////////////////////////////////////////////////////////////////
//     Function: GraphicsStateGuardianBase::get_default_gsg
//       Access: Published, Static
//  Description: Returns a pointer to the "default" GSG.  This is
//               typically the first GSG created in an application; in
//               a single-window application, it will be the only GSG.
//               This GSG is used to determine default optimization
//               choices for loaded geometry.
//
//               The return value may be NULL if a GSG has not been
//               created.
////////////////////////////////////////////////////////////////////
GraphicsStateGuardianBase *GraphicsStateGuardianBase::
get_default_gsg() {
  MutexHolder holder(_lock);
  return _default_gsg;
}

////////////////////////////////////////////////////////////////////
//     Function: GraphicsStateGuardianBase::set_default_gsg
//       Access: Published, Static
//  Description: Specifies a particular GSG to use as the "default"
//               GSG.  See get_default_gsg().
////////////////////////////////////////////////////////////////////
void GraphicsStateGuardianBase::
set_default_gsg(GraphicsStateGuardianBase *default_gsg) {
  MutexHolder holder(_lock);
  if (find(_gsgs.begin(), _gsgs.end(), default_gsg) == _gsgs.end()) {
    // The specified GSG doesn't exist or it has already destructed.
    nassertv(false);
    return;
  }

  _default_gsg = default_gsg;
}

////////////////////////////////////////////////////////////////////
//     Function: GraphicsStateGuardianBase::add_gsg
//       Access: Public, Static
//  Description: Called by a GSG after it has been initialized, to add
//               a new GSG to the available list.
////////////////////////////////////////////////////////////////////
void GraphicsStateGuardianBase::
add_gsg(GraphicsStateGuardianBase *gsg) {
  MutexHolder holder(_lock);

  if (find(_gsgs.begin(), _gsgs.end(), gsg) != _gsgs.end()) {
    // Already on the list.
    return;
  }

  _gsgs.push_back(gsg);

  if (_default_gsg == (GraphicsStateGuardianBase *)NULL) {
    _default_gsg = gsg;
  }
}

////////////////////////////////////////////////////////////////////
//     Function: GraphicsStateGuardianBase::remove_gsg
//       Access: Public, Static
//  Description: Called by a GSG destructor to remove a GSG from the
//               available list.
////////////////////////////////////////////////////////////////////
void GraphicsStateGuardianBase::
remove_gsg(GraphicsStateGuardianBase *gsg) {
  MutexHolder holder(_lock);

  GSGs::iterator gi = find(_gsgs.begin(), _gsgs.end(), gsg);
  if (gi == _gsgs.end()) {
    // Already removed, or never added.
    return;
  }

  _gsgs.erase(gi);

  if (_default_gsg == gsg) {
    if (_gsgs.empty()) {
      _default_gsg = *_gsgs.begin();
    } else {
      _default_gsg = NULL;
    }
  }
}
