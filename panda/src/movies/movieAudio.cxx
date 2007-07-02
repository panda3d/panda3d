// Filename: movieAudio.cxx
// Created by: jyelon (02Jul07)
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
// panda3d-general@lists.sourceforge.net .
//
////////////////////////////////////////////////////////////////////

#include "movieAudio.h"

TypeHandle MovieAudio::_type_handle;

////////////////////////////////////////////////////////////////////
//     Function: MovieAudio::Constructor
//       Access: Protected
//  Description: 
////////////////////////////////////////////////////////////////////
MovieAudio::
MovieAudio(const string &name) :
  Namable(name),
  _rate(44100),
  _samples_read(0),
  _approx_time_remaining(1.0E10)
{
}

////////////////////////////////////////////////////////////////////
//     Function: MovieAudio::Destructor
//       Access: Public, Virtual
//  Description: 
////////////////////////////////////////////////////////////////////
MovieAudio::
~MovieAudio() {
}

////////////////////////////////////////////////////////////////////
//     Function: MovieAudio::read_samples
//       Access: Public, Virtual
//  Description: Read audio samples from the stream.  N is the
//               number of samples you wish to read.  Your buffer
//               must be at least as large as N * channels.  
//               Multiple-channel audio will be interleaved. Returns
//               the actual number of samples read.  This will always
//               be equal to N unless end-of-stream has been reached.
////////////////////////////////////////////////////////////////////
int MovieAudio::
read_samples(int n, PN_int16 *data) {

  // This is a dummy implementation.  This method should be overridden.
  nassertr(n > 0, 0);
  for (int i=0; i<n * _channels; i++) {
    data[i] = 0;
  }
  return n;
}

////////////////////////////////////////////////////////////////////
//     Function: MovieAudio::skip_samples
//       Access: Public, Virtual
//  Description: Skip audio samples from the stream.  This is mostly
//               for debugging purposes.
////////////////////////////////////////////////////////////////////
int MovieAudio::
skip_samples(int n) {
  nassertr(n > 0, 0);
  PN_int16 *data = new PN_int16[n];
  int res = read_samples(n, data);
  delete data;
  return res;
}
 
