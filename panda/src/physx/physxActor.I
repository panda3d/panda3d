/**
 * PANDA 3D SOFTWARE
 * Copyright (c) Carnegie Mellon University.  All rights reserved.
 *
 * All use of this software is subject to the terms of the revised BSD
 * license.  You should have received a copy of this license along
 * with this source code in a file named "LICENSE."
 *
 * @file physxActor.I
 * @author enn0x
 * @date 2009-09-14
 */

/**
 *
 */
INLINE PhysxActor::
PhysxActor() : PhysxObject() {

}

/**
 *
 */
INLINE PhysxActor::
~PhysxActor() {

}

/**
 *
 */
INLINE void PhysxActor::
ls() const {

  ls(nout);
}

/**
 *
 */
INLINE void PhysxActor::
ls(std::ostream &out, int indent_level) const {

  indent(out, indent_level) << get_type().get_name()
                            << " " << _name
                            << " (at 0x" << this << ")";

  if (!_np.is_empty()) {
    out << " NP:" << _np;
  }

  if (_controller) {
    out << " C:0x" << _controller;
  }

  out << "\n";

  _shapes.ls(out, indent_level);
}
