/**
 * PANDA 3D SOFTWARE
 * Copyright (c) Carnegie Mellon University.  All rights reserved.
 *
 * All use of this software is subject to the terms of the revised BSD
 * license.  You should have received a copy of this license along
 * with this source code in a file named "LICENSE."
 *
 * @file collisionHandler.cxx
 * @author drose
 * @date 2002-03-16
 */

#include "collisionHandler.h"

TypeHandle CollisionHandler::_type_handle;

/**
 * Will be called by the CollisionTraverser before a new traversal is begun.
 * It instructs the handler to reset itself in preparation for a number of
 * CollisionEntries to be sent.
 */
void CollisionHandler::
begin_group() {
}

/**
 * Called between a begin_group() .. end_group() sequence for each collision
 * that is detected.
 */
void CollisionHandler::
add_entry(CollisionEntry *) {
}

/**
 * Called by the CollisionTraverser at the completion of all collision
 * detections for this traversal.  It should do whatever finalization is
 * required for the handler.
 *
 * The return value is normally true, but if this returns value, the
 * CollisionTraverser will remove the handler from its list, allowing the
 * CollisionHandler itself to determine when it is no longer needed.
 */
bool CollisionHandler::
end_group() {
  return true;
}
