// Filename: audioSound.cxx
// Created by:  skyler (June 6, 2001)
// Prior system by: cary
//
////////////////////////////////////////////////////////////////////
//
// PANDA 3D SOFTWARE
// Copyright (c) 2001 - 2004, Disney Enterprises, Inc.  All rights reserved
//
// All use of this software is subject to the terms of the Panda 3d
// Software license.  You should have received a copy of this license
// along with this source code; you will also find a current copy of
// the license at http://etc.cmu.edu/panda3d/docs/license/ .
//
// To contact the maintainers of this program write to
// panda3d-general@lists.sourceforge.net .
//
////////////////////////////////////////////////////////////////////

#include "audioSound.h"

TypeHandle AudioSound::_type_handle;

////////////////////////////////////////////////////////////////////
//     Function: AudioSound::Destructor
//       Access: Published, Virtual
//  Description:
////////////////////////////////////////////////////////////////////
AudioSound::
~AudioSound() {
}

////////////////////////////////////////////////////////////////////
//     Function: AudioSound::Constructor
//       Access: Protected
//  Description:
////////////////////////////////////////////////////////////////////
AudioSound::
AudioSound() {
  // Intentionally blank.
}


void AudioSound::
set_3d_attributes(float px, float py, float pz, float vx, float vy, float vz) {
  // Intentionally blank.
}

void AudioSound::
get_3d_attributes(float *px, float *py, float *pz, float *vx, float *vy, float *vz) {
  // Intentionally blank.
}
