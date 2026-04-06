/**
 * PANDA 3D SOFTWARE
 * Copyright (c) Carnegie Mellon University.  All rights reserved.
 *
 * All use of this software is subject to the terms of the revised BSD
 * license.  You should have received a copy of this license along
 * with this source code in a file named "LICENSE."
 *
 * @file cullHandler.I
 * @author drose
 * @date 2002-03-04
 */

/**
 * Draws the indicated CullableObject, with full support for decals if they
 * are attached to the object.  The appropriate state is set on the GSG before
 * drawing the object.
 */
INLINE void CullHandler::
draw(CullableObject *object, GraphicsStateGuardianBase *gsg,
     bool force, Thread *current_thread) {
  object->draw(gsg, force, current_thread);
}
