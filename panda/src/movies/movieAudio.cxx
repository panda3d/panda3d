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
//  Description: This constructor returns a null audio stream --- a
//               stream of total silence, at 8000 samples per second.
//               To get more interesting audio, you need to construct
//               a subclass of this class.
////////////////////////////////////////////////////////////////////
MovieAudio::
MovieAudio(const string &name, double len) :
  Namable(name),
  _rate(8000),
  _channels(1),
  _samples_read(0),
  _approx_len(len)
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
//               It is legal to pass a null pointer, in this case,
//               the samples are discarded.
////////////////////////////////////////////////////////////////////
int MovieAudio::
read_samples(int n, PN_int16 *data) {

  // This is the null implementation, which generates pure silence.
  // Normally, this method will be overridden by a subclass.

  if (n < 0) {
    return 0;
  }

  // Convert length to an integer sample count.
  // This could generate an integer-overflow, in this case,
  // just set the integer length to maxint (74 hrs).
  int ilen;
  if (_approx_len < 268000.0) {
    ilen = _approx_len * 8000.0;
  } else {
    ilen = 0x7FFFFFFF;
  }

  // Generate and return samples.
  int remain = ilen - _samples_read;
  if (n > remain) {
    n = remain;
  }
  if (data) {
    for (int i=0; i<n; i++) {
      data[i] = 0;
    }
  }
  _samples_read += n;
  return n;
}

