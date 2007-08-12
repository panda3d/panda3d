// Filename: ffmpegAudio.cxx
// Created by: jyelon (01Aug2007)
//
////////////////////////////////////////////////////////////////////
//
// PANDA 3D SOFTWARE
// Copyright (c) 2001 - 2007, Disney Enterprises, Inc.  All rights reserved
//
// All use of this software is subject to the terms of the Panda 3d
// Software license.  You should have received a copy of this license
// along with this source code; you will also find a current copy of
// the license at http://etc.cmu.edu/panda3d/docs/license/ .
//
// To contact the maintainers of this program write to
// panda3d-general@lists.sourceforge.net 
//
////////////////////////////////////////////////////////////////////

#include "ffmpegAudio.h"

TypeHandle FfmpegAudio::_type_handle;

////////////////////////////////////////////////////////////////////
//     Function: FfmpegAudio::Constructor
//       Access: Protected
//  Description: xxx
////////////////////////////////////////////////////////////////////
FfmpegAudio::
FfmpegAudio(CPT(FfmpegMovie) source, double offset) :
  MovieAudio((const FfmpegMovie *)source),
  _sourcep(source)
{
}

////////////////////////////////////////////////////////////////////
//     Function: FfmpegAudio::Destructor
//       Access: Protected, Virtual
//  Description: xxx
////////////////////////////////////////////////////////////////////
FfmpegAudio::
~FfmpegAudio() {
}

////////////////////////////////////////////////////////////////////
//     Function: FfmpegAudio::read_samples
//       Access: Public, Virtual
//  Description: Read audio samples from the stream.  N is the
//               number of samples you wish to read.  Your buffer
//               must be equal in size to N * channels.  
//               Multiple-channel audio will be interleaved. 
////////////////////////////////////////////////////////////////////
void FfmpegAudio::
read_samples(int n, PN_int16 *data) {

  // This is the null implementation, which generates pure silence.
  // Normally, this method will be overridden by a subclass.

  if (n <= 0) {
    return;
  }

  for (int i=0; i<n; i++) {
    data[i] = 0;
  }
  _samples_read += n;
}

