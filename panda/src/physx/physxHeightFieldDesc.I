/**
 * PANDA 3D SOFTWARE
 * Copyright (c) Carnegie Mellon University.  All rights reserved.
 *
 * All use of this software is subject to the terms of the revised BSD
 * license.  You should have received a copy of this license along
 * with this source code in a file named "LICENSE."
 *
 * @file physxHeightFieldDesc.I
 * @author enn0x
 * @date 2009-10-15
 */

/**
 *
 */
INLINE PhysxHeightFieldDesc::
PhysxHeightFieldDesc() {

  _samples = nullptr;
  _desc.setToDefault();
}

/**
 *
 */
INLINE PhysxHeightFieldDesc::
~PhysxHeightFieldDesc() {

}

/**
 * (re)sets the structure to the default.
 */
INLINE void PhysxHeightFieldDesc::
set_to_default() {

  _desc.setToDefault();
}

/**
 * Returns true if the descriptor is valid.
 */
INLINE bool PhysxHeightFieldDesc::
is_valid() const {

  return _desc.isValid();
}

/**
 *
 */
INLINE void PhysxHeightFieldDesc::
set_size(unsigned int num_rows, unsigned int num_columns) {

  if (_samples) {
    unset_size();
  }

  _desc.format = NX_HF_S16_TM;
  _desc.nbRows = (NxU32) num_rows;
  _desc.nbColumns = (NxU32) num_columns;
  _desc.sampleStride = sizeof(NxU32);

  _samples = new NxU32[_desc.nbColumns * _desc.nbRows];
  _desc.samples = _samples;

}

/**
 * Releases the memory allocated for the height field samples.
 */
INLINE void PhysxHeightFieldDesc::
unset_size() {

  if (_samples) {
    _desc.nbRows = (NxU32) 0;
    _desc.nbColumns = (NxU32) 0;
    _desc.samples = nullptr;
    delete[] _samples;
  }
}
