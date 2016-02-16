/**
 * PANDA 3D SOFTWARE
 * Copyright (c) Carnegie Mellon University.  All rights reserved.
 *
 * All use of this software is subject to the terms of the revised BSD
 * license.  You should have received a copy of this license along
 * with this source code in a file named "LICENSE."
 *
 * @file bulletGenericConstraint.I
 * @author enn0x
 * @date 2010-03-02
 */

/**

 */
INLINE BulletGenericConstraint::
~BulletGenericConstraint() {

  delete _constraint;
}

/**

 */
INLINE CPT(TransformState) BulletGenericConstraint::
get_frame_a() const {

  return btTrans_to_TransformState(_constraint->getFrameOffsetA());
}

/**

 */
INLINE CPT(TransformState) BulletGenericConstraint::
get_frame_b() const {

  return btTrans_to_TransformState(_constraint->getFrameOffsetB());
}
