// Filename: ffmpegVideo.I
// Created by: jyelon (01Aug2007)
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
//     Function: FfmpegVideo::get_time_correction
//       Access: Private
//  Description: The timestamps of packets in AVI files are often
//               really inaccurate.  This returns a small heuristic
//               correction based on analysis of the audio packets.
////////////////////////////////////////////////////////////////////

INLINE double FfmpegVideo::
get_time_correction() {
  // The idea here is simple.  For audio packets it
  // is possible to calculate an accurate time by counting
  // the audio samples.  It is then possible to measure
  // the difference between the reported time and the
  // actual time of the audio packet.  We average this 
  // difference over the last few audio packets.  We then
  // assume that the video packets are off by the same amount.

  double correction = 0.0;
  for (int i=0; i<_time_correction_window; i++) {
    correction += _time_corrections[i];
  }
  correction /= _time_correction_window;
  return correction;
}

////////////////////////////////////////////////////////////////////
//     Function: FfmpegVideo::update_time_correction
//       Access: Private
//  Description: The timestamps of packets in AVI files are often
//               really inaccurate.  This records some data which
//               is needed to calculate a heuristic correction factor.
////////////////////////////////////////////////////////////////////
INLINE void FfmpegVideo::
update_time_correction(double diff) {
  _time_corrections[_time_correction_next % _time_correction_window] = diff;
  _time_correction_next += 1;
}

