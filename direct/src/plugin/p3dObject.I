/**
 * PANDA 3D SOFTWARE
 * Copyright (c) Carnegie Mellon University.  All rights reserved.
 *
 * All use of this software is subject to the terms of the revised BSD
 * license.  You should have received a copy of this license along
 * with this source code in a file named "LICENSE."
 *
 * @file p3dObject.I
 * @author drose
 * @date 2009-06-30
 */

/**
 *
 */
inline P3DObject::
P3DObject() {
  _class = &_object_class;

  // The reference count is initially 1.
  _ref_count = 1;
}

/**
 *
 */
inline P3DObject::
P3DObject(const P3DObject &copy) {
  assert(copy._class == &_object_class);
  _class = copy._class;
  _ref_count = 1;
}
