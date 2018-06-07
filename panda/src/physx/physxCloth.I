/**
 * PANDA 3D SOFTWARE
 * Copyright (c) Carnegie Mellon University.  All rights reserved.
 *
 * All use of this software is subject to the terms of the revised BSD
 * license.  You should have received a copy of this license along
 * with this source code in a file named "LICENSE."
 *
 * @file physxCloth.I
 * @author enn0x
 * @date 2010-03-30
 */

/**
 *
 */
INLINE PhysxCloth::
PhysxCloth() : PhysxObject() {

}

/**
 *
 */
INLINE PhysxCloth::
~PhysxCloth() {

}

/**
 *
 */
INLINE void PhysxCloth::
ls() const {

  ls(nout);
}

/**
 *
 */
INLINE void PhysxCloth::
ls(std::ostream &out, int indent_level) const {

  indent(out, indent_level) << get_type().get_name()
                            << " " << _name
                            << " (at 0x" << this << ")";

  if (_node) {
    out << " N:0x" << _node;
  }

  out << "\n";
}
