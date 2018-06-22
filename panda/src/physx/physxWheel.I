/**
 * PANDA 3D SOFTWARE
 * Copyright (c) Carnegie Mellon University.  All rights reserved.
 *
 * All use of this software is subject to the terms of the revised BSD
 * license.  You should have received a copy of this license along
 * with this source code in a file named "LICENSE."
 *
 * @file physxWheel.I
 * @author enn0x
 * @date 2010-03-23
 */

/**
 *
 */
INLINE PhysxWheel::
PhysxWheel() {

}

/**
 *
 */
INLINE PhysxWheel::
~PhysxWheel() {

}

/**
 *
 */
INLINE void PhysxWheel::
ls() const {

  ls(nout);
}

/**
 *
 */
INLINE void PhysxWheel::
ls(std::ostream &out, int indent_level) const {

  indent(out, indent_level) << get_type().get_name()
                            << " (at 0x" << this << ")\n";
}
