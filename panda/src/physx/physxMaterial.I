/**
 * PANDA 3D SOFTWARE
 * Copyright (c) Carnegie Mellon University.  All rights reserved.
 *
 * All use of this software is subject to the terms of the revised BSD
 * license.  You should have received a copy of this license along
 * with this source code in a file named "LICENSE."
 *
 * @file physxMaterial.I
 * @author enn0x
 * @date 2009-09-21
 */

/**
 *
 */
INLINE PhysxMaterial::
PhysxMaterial() : PhysxObject() {

}

/**
 *
 */
INLINE PhysxMaterial::
~PhysxMaterial() {

}

/**
 *
 */
INLINE void PhysxMaterial::
ls() const {

  ls(nout);
}

/**
 *
 */
INLINE void PhysxMaterial::
ls(std::ostream &out, int indent_level) const {

  indent(out, indent_level) << get_type().get_name()
                            << " (at 0x" << this << ")\n";
}
