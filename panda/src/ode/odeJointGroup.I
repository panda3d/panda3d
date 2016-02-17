/**
 * PANDA 3D SOFTWARE
 * Copyright (c) Carnegie Mellon University.  All rights reserved.
 *
 * All use of this software is subject to the terms of the revised BSD
 * license.  You should have received a copy of this license along
 * with this source code in a file named "LICENSE."
 *
 * @file odeJointGroup.I
 * @author joswilso
 * @date 2006-12-27
 */

INLINE dJointGroupID OdeJointGroup::
get_id() const {
  return _id;
}

INLINE void OdeJointGroup::
empty() const {
  dJointGroupEmpty(_id);
}

INLINE int OdeJointGroup::
compare_to(const OdeJointGroup &other) const {
  if (_id != other._id) {
    return _id < other._id ? -1 : 1;
  }
  return 0;
}
