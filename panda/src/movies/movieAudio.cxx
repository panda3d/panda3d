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
// panda3d-general@lists.sourceforge.net 
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
MovieAudio(const string &name, CPT(Movie) source) :
  Namable(name),
  _source(source),
  _samples_read(0)
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
//               must be equal in size to N * channels.  
//               Multiple-channel audio will be interleaved. 
////////////////////////////////////////////////////////////////////
void MovieAudio::
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

