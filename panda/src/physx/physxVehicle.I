/**
 * PANDA 3D SOFTWARE
 * Copyright (c) Carnegie Mellon University.  All rights reserved.
 *
 * All use of this software is subject to the terms of the revised BSD
 * license.  You should have received a copy of this license along
 * with this source code in a file named "LICENSE."
 *
 * @file physxVehicle.I
 * @author enn0x
 * @date 2010-03-23
 */

/**
 *
 */
INLINE PhysxVehicle::
PhysxVehicle() : PhysxObject() {

}

/**
 *
 */
INLINE PhysxVehicle::
~PhysxVehicle() {

}

/**
 *
 */
INLINE void PhysxVehicle::
ls() const {

  ls(nout);
}

/**
 *
 */
INLINE void PhysxVehicle::
ls(std::ostream &out, int indent_level) const {

  indent(out, indent_level) << get_type().get_name()
                            << " (at 0x" << this << ")\n";
}
