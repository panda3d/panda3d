/**
 * PANDA 3D SOFTWARE
 * Copyright (c) Carnegie Mellon University.  All rights reserved.
 *
 * All use of this software is subject to the terms of the revised BSD
 * license.  You should have received a copy of this license along
 * with this source code in a file named "LICENSE."
 *
 * @file cfCommand.I
 * @author drose
 * @date 2009-02-19
 */

/**
 *
 */
INLINE CFCommand::
CFCommand() {
}

/**
 *
 */
INLINE CFDoCullCommand::
CFDoCullCommand() {
}

/**
 *
 */
INLINE CFDoCullCommand::
CFDoCullCommand(PandaNode *scene) : _scene(scene) {
}

/**
 *
 */
INLINE PandaNode *CFDoCullCommand::
get_scene() const {
  return _scene;
}
