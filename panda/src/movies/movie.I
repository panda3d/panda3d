// Filename: movie.I
// Created by: jyelon (02Jul07)
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

////////////////////////////////////////////////////////////////////
//     Function: Movie::size_x
//       Access: Published
//  Description: Get the horizontal size of the movie.
////////////////////////////////////////////////////////////////////
INLINE int Movie::
size_x() const {
  return _size_x;
}

////////////////////////////////////////////////////////////////////
//     Function: Movie::size_y
//       Access: Published
//  Description: Get the vertical size of the movie.
////////////////////////////////////////////////////////////////////
INLINE int Movie::
size_y() const {
  return _size_y;
}

////////////////////////////////////////////////////////////////////
//     Function: Movie::get_num_components
//       Access: Published
//  Description: Indicates whether the movie is monochrome, color,
//               or color-plus-alpha.
//
//               You can fetch the movie's contents into any texture,
//               regardless of the texture's format.  However, it may
//               be faster if the texture matches the movie.
////////////////////////////////////////////////////////////////////
INLINE int Movie::
get_num_components() const {
  return _num_components;
}

////////////////////////////////////////////////////////////////////
//     Function: Movie::length
//       Access: Published
//  Description: Returns the length of the movie.
//
//               Some kinds of Movie, such as internet TV station, 
//               might not have a predictable length.  In that case,
//               the length will be set to a very large number: 1.0E10.
//               If the internet TV station goes offline, the video
//               or audio stream will set its abort flag.  Reaching the
//               end of the movie (ie, the specified length) normally
//               does not cause the abort flag to be set.
//
//               The video and audio streams produced by get_video and
//               get_audio are always of unlimited duration - you can
//               always read another video frame or another audio
//               sample.  This is true even if the specified length
//               is reached, or an abort is flagged. If either stream
//               runs out of data, it will synthesize blank video
//               frames and silent audio samples as necessary to
//               satisfy read requests.
//
//               Some AVI files have incorrect length values encoded
//               into them - usually, they're a second or two long or
//               short.  When playing such an AVI using the Movie class,
//               you may see a slightly truncated video, or a slightly
//               elongated video (padded with black frames).  There are
//               utilities out there to fix the length values in AVI
//               files.
//
////////////////////////////////////////////////////////////////////
INLINE double Movie::
length() const {
  return _length;
}

////////////////////////////////////////////////////////////////////
//     Function: Movie::audio_rate
//       Access: Public
//  Description: Returns the audio sample rate, in samples per sec.
////////////////////////////////////////////////////////////////////
INLINE int Movie::
audio_rate() const {
  return _audio_rate;
}

////////////////////////////////////////////////////////////////////
//     Function: Movie::audio_channels
//       Access: Public
//  Description: Returns the number of audio channels.  Ie, 1 for
//               mono, 2 for stereo.
////////////////////////////////////////////////////////////////////
INLINE int Movie::
audio_channels() const {
  return _audio_channels;
}

////////////////////////////////////////////////////////////////////
//     Function: Movie::ignores_offset
//       Access: Published
//  Description: The methods get_video and get_audio both accept
//               an offset parameter, which allow you to seek
//               to a particular location within the movie.  Not
//               all movies can do this, though.  For instance, an
//               internet TV station cannot be fast-forwarded.  In
//               these cases, the offset parameter is ignored.
////////////////////////////////////////////////////////////////////
INLINE bool Movie::
ignores_offset() const {
  return _ignores_offset;
}

////////////////////////////////////////////////////////////////////
//     Function: Movie::dummy_video
//       Access: Published
//  Description: If you open a sound file as a movie, a dummy video
//               stream will be synthesized.  Returns true in this case.
////////////////////////////////////////////////////////////////////
INLINE bool Movie::
dummy_video() const {
  return _dummy_video;
}

////////////////////////////////////////////////////////////////////
//     Function: Movie::dummy_audio
//       Access: Published
//  Description: If you open a movie with no sound, a dummy sound
//               stream will be synthesized.  Returns true in this case.
////////////////////////////////////////////////////////////////////
INLINE bool Movie::
dummy_audio() const {
  return _dummy_audio;
}

