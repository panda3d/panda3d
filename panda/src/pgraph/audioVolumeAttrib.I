// Filename: audioVolumeAttrib.I
// Created by:  darren (15Dec06)
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
//     Function: AudioVolumeAttrib::Copy Constructor
//       Access: Protected
//  Description: Use AudioVolumeAttrib::make() to construct a new
//               AudioVolumeAttrib object.
////////////////////////////////////////////////////////////////////
INLINE AudioVolumeAttrib::
AudioVolumeAttrib(const AudioVolumeAttrib &copy) :
  _off(copy._off),
  _has_volume(copy._has_volume),
  _volume(copy._volume)
{
}

////////////////////////////////////////////////////////////////////
//     Function: AudioVolumeAttrib::is_off
//       Access: Published
//  Description: Returns true if the AudioVolumeAttrib will ignore any
//               color scales inherited from above, false otherwise.
//               This is not the same thing as !has_scale(); a
//               AudioVolumeAttrib may have the "off" flag set and also
//               have another scale specified.
////////////////////////////////////////////////////////////////////
INLINE bool AudioVolumeAttrib::
is_off() const {
  return _off;
}

////////////////////////////////////////////////////////////////////
//     Function: AudioVolumeAttrib::has_volume
//       Access: Published
//  Description: Returns true if the AudioVolumeAttrib has a
//               non-identity volume, false otherwise (in which case it
//               might be an off attrib or an identity attrib).
////////////////////////////////////////////////////////////////////
INLINE bool AudioVolumeAttrib::
has_volume() const {
  return _has_volume;
}

////////////////////////////////////////////////////////////////////
//     Function: AudioVolumeAttrib::get_volume
//       Access: Published
//  Description: Returns the volume to be applied to sounds.
////////////////////////////////////////////////////////////////////
INLINE PN_stdfloat AudioVolumeAttrib::
get_volume() const {
  return _volume;
}
