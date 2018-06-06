/**
 * PANDA 3D SOFTWARE
 * Copyright (c) Carnegie Mellon University.  All rights reserved.
 *
 * All use of this software is subject to the terms of the revised BSD
 * license.  You should have received a copy of this license along
 * with this source code in a file named "LICENSE."
 *
 * @file physxSoftBody.I
 * @author enn0x
 * @date 2010-09-13
 */

/**
 *
 */
INLINE PhysxSoftBody::
PhysxSoftBody() : PhysxObject() {

}

/**
 *
 */
INLINE PhysxSoftBody::
~PhysxSoftBody() {

}

/**
 *
 */
INLINE void PhysxSoftBody::
ls() const {

  ls(nout);
}

/**
 *
 */
INLINE void PhysxSoftBody::
ls(std::ostream &out, int indent_level) const {

  indent(out, indent_level) << get_type().get_name()
                            << " " << _name
                            << " (at 0x" << this << ")";

  if (_node) {
    out << " N:0x" << _node;
  }

  out << "\n";
}
