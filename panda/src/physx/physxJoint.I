/**
 * PANDA 3D SOFTWARE
 * Copyright (c) Carnegie Mellon University.  All rights reserved.
 *
 * All use of this software is subject to the terms of the revised BSD
 * license.  You should have received a copy of this license along
 * with this source code in a file named "LICENSE."
 *
 * @file physxJoint.I
 * @author enn0x
 * @date 2009-10-02
 */

/**
 *
 */
INLINE PhysxJoint::
PhysxJoint() : PhysxObject() {

}

/**
 *
 */
INLINE void PhysxJoint::
ls() const {

  ls(nout);
}

/**
 *
 */
INLINE void PhysxJoint::
ls(std::ostream &out, int indent_level) const {

  indent(out, indent_level) << get_type().get_name()
                            << " " << _name
                            << " (at 0x" << this << ")";

  NxActor *actorPtr[2];
  ptr()->getActors(&actorPtr[0], &actorPtr[1]);

  if (actorPtr[0]) {
    out << " A1:0x" << actorPtr[0]->userData;
  }

  if (actorPtr[1]) {
    out << " A2:0x" << actorPtr[1]->userData;
  }

  out << "\n";
}
