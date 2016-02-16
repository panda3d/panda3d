/**
 * PANDA 3D SOFTWARE
 * Copyright (c) Carnegie Mellon University.  All rights reserved.
 *
 * All use of this software is subject to the terms of the revised BSD
 * license.  You should have received a copy of this license along
 * with this source code in a file named "LICENSE."
 *
 * @file linearUserDefinedForce.I
 * @author charles
 * @date 2000-07-31
 */

/**

 */
void LinearUserDefinedForce::
set_proc(LVector3 (*proc)(const PhysicsObject *)) {
  _proc = proc;
}
