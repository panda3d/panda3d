/**
 * PANDA 3D SOFTWARE
 * Copyright (c) Carnegie Mellon University.  All rights reserved.
 *
 * All use of this software is subject to the terms of the revised BSD
 * license.  You should have received a copy of this license along
 * with this source code in a file named "LICENSE."
 *
 * @file physxGroupsMask.I
 * @author enn0x
 * @date 2009-10-21
 */

/**

 */
INLINE PhysxGroupsMask::
PhysxGroupsMask() {

  _mask.bits0 = 0x00000000;
  _mask.bits1 = 0x00000000;
  _mask.bits2 = 0x00000000;
  _mask.bits3 = 0x00000000;
}

/**

 */
INLINE PhysxGroupsMask::
PhysxGroupsMask(NxGroupsMask mask)
  : _mask(mask) {

}

/**

 */
INLINE PhysxGroupsMask::
~PhysxGroupsMask() {

}

/**

 */
INLINE NxGroupsMask PhysxGroupsMask::
get_mask() const {

  return _mask;
}

/**

 */
INLINE void PhysxGroupsMask::
set_mask(NxGroupsMask mask) {

  _mask = mask;
}

/**

 */
INLINE unsigned int PhysxGroupsMask::
get_bits0() const {

  return _mask.bits0;
}

/**

 */
INLINE void PhysxGroupsMask::
set_bits0(unsigned int bits) {

  _mask.bits0 = (NxU32) bits;
}

/**

 */
INLINE unsigned int PhysxGroupsMask::
get_bits1() const {

  return _mask.bits1;
}

/**

 */
INLINE void PhysxGroupsMask::
set_bits1(unsigned int bits) {

  _mask.bits1 = (NxU32) bits;
}

/**

 */
INLINE unsigned int PhysxGroupsMask::
get_bits2() const {

  return _mask.bits2;
}

/**

 */
INLINE void PhysxGroupsMask::
set_bits2(unsigned int bits) {

  _mask.bits2 = (NxU32) bits;
}

/**

 */
INLINE unsigned int PhysxGroupsMask::
get_bits3() const {

  return _mask.bits3;
}

/**

 */
INLINE void PhysxGroupsMask::
set_bits3(unsigned int bits) {

  _mask.bits3 = (NxU32) bits;
}
