/**
 * PANDA 3D SOFTWARE
 * Copyright (c) Carnegie Mellon University.  All rights reserved.
 *
 * All use of this software is subject to the terms of the revised BSD
 * license.  You should have received a copy of this license along
 * with this source code in a file named "LICENSE."
 *
 * @file scissorAttrib.I
 * @author drose
 * @date 2008-07-29
 */

/**
 * Constructs a ScissorAttrib that restricts rendering to the indicated frame
 * within the current DisplayRegion.  (0,0) is the lower-left corner of the
 * DisplayRegion, and (1,1) is the upper-right corner.
 */
INLINE CPT(RenderAttrib) ScissorAttrib::
make(PN_stdfloat left, PN_stdfloat right, PN_stdfloat bottom, PN_stdfloat top) {
  return make(LVecBase4(left, right, bottom, top));
}

/**
 * Returns true if the ScissorAttrib is an 'off' ScissorAttrib, indicating
 * that scissor testing is disabled.
 */
INLINE bool ScissorAttrib::
is_off() const {
  return _off;
}

/**
 * Returns the left, right, bottom, top coordinates of the scissor frame.
 * This defines a frame within the current DisplayRegion, where 0,0 is the
 * lower-left corner of the DisplayRegion, and 1,1 is the upper-right corner.
 */
INLINE const LVecBase4 &ScissorAttrib::
get_frame() const {
  return _frame;
}
