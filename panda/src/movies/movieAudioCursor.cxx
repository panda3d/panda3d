// Filename: movieAudioCursor.cxx
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

#include "movieAudioCursor.h"

TypeHandle MovieAudioCursor::_type_handle;

////////////////////////////////////////////////////////////////////
//     Function: MovieAudioCursor::Constructor
//       Access: Public
//  Description: This constructor returns a null audio stream --- a
//               stream of total silence, at 8000 samples per second.
//               To get more interesting audio, you need to construct
//               a subclass of this class.
////////////////////////////////////////////////////////////////////
MovieAudioCursor::
MovieAudioCursor(MovieAudio *src) :
  _source(src),
  _audio_rate(8000),
  _audio_channels(1),
  _length(1.0E10),
  _can_seek(true),
  _can_seek_fast(true),
  _ready(0x40000000),
  _aborted(false),
  _samples_read(0)
{
}

////////////////////////////////////////////////////////////////////
//     Function: MovieAudioCursor::Destructor
//       Access: Public, Virtual
//  Description: 
////////////////////////////////////////////////////////////////////
MovieAudioCursor::
~MovieAudioCursor() {
}

////////////////////////////////////////////////////////////////////
//     Function: MovieAudioCursor::read_samples
//       Access: Public, Virtual
//  Description: Read audio samples from the stream.  N is the
//               number of samples you wish to read.  Your buffer
//               must be equal in size to N * channels.  
//               Multiple-channel audio will be interleaved. 
////////////////////////////////////////////////////////////////////
void MovieAudioCursor::
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

////////////////////////////////////////////////////////////////////
//     Function: MovieAudioCursor::seek
//       Access: Published, Virtual
//  Description: Skips to the specified offset within the file.
//
//               If the movie reports that it cannot seek, then
//               this method can still advance by reading samples
//               and discarding them.  However, to move backward,
//               can_seek must be true.
//
//               If the movie reports that it can_seek, it doesn't
//               mean that it can do so quickly.  It may have to
//               rewind the movie and then fast forward to the
//               desired location.  Only if can_seek_fast returns
//               true can seek operations be done in constant time.
//
//               Seeking may not be precise, because AVI files 
//               often have inaccurate indices.  After
//               seeking, tell will indicate that the cursor is
//               at the target location. However, in truth, the data
//               you read may come from a slightly offset location.
////////////////////////////////////////////////////////////////////
void MovieAudioCursor::
seek(double offset) {
  _last_seek = offset;
  _samples_read = 0;
}

