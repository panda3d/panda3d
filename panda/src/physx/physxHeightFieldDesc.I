// Filename: physxHeightFieldDesc.I
// Created by:  enn0x (15Oct09)
//
////////////////////////////////////////////////////////////////////
//
// PANDA 3D SOFTWARE
// Copyright (c) Carnegie Mellon University.  All rights reserved.
//
// All use of this software is subject to the terms of the revised BSD
// license.  You should have received a copy of this license along
// with this source code in a file named "LICENSE."
//
////////////////////////////////////////////////////////////////////



////////////////////////////////////////////////////////////////////
//     Function: PhysxHeightFieldDesc::Constructor
//       Access: Published
//  Description:
////////////////////////////////////////////////////////////////////
INLINE PhysxHeightFieldDesc::
PhysxHeightFieldDesc() {

  _samples = NULL;
  _desc.setToDefault();
}

////////////////////////////////////////////////////////////////////
//     Function: PhysxHeightFieldDesc::Destructor
//       Access: Published
//  Description:
////////////////////////////////////////////////////////////////////
INLINE PhysxHeightFieldDesc::
~PhysxHeightFieldDesc() {

}

////////////////////////////////////////////////////////////////////
//     Function: PhysxHeightFieldDesc::set_to_default
//       Access: Published
//  Description: (re)sets the structure to the default.
////////////////////////////////////////////////////////////////////
INLINE void PhysxHeightFieldDesc::
set_to_default() {

  _desc.setToDefault();
}

////////////////////////////////////////////////////////////////////
//     Function: PhysxHeightFieldDesc::is_valid
//       Access: Published
//  Description: Returns true if the descriptor is valid.
////////////////////////////////////////////////////////////////////
INLINE bool PhysxHeightFieldDesc::
is_valid() const {

  return _desc.isValid();
}

////////////////////////////////////////////////////////////////////
//     Function: PhysxHeightFieldDesc::set_size
//       Access: Private
//  Description:
////////////////////////////////////////////////////////////////////
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

////////////////////////////////////////////////////////////////////
//     Function: PhysxHeightFieldDesc::unset_size
//       Access: Private
//  Description: Releases the memory allocated for the height field
//               samples.
////////////////////////////////////////////////////////////////////
INLINE void PhysxHeightFieldDesc::
unset_size() {

  if (_samples) {
    _desc.nbRows = (NxU32) 0;
    _desc.nbColumns = (NxU32) 0;
    _desc.samples = NULL;
    delete[] _samples;
  }
}

