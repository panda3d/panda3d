// Filename: cLwoClip.I
// Created by:  drose (27Apr01)
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
//     Function: CLwoClip::get_index
//       Access: Public
//  Description: Returns the index number of this clip.  Each clip in
//               a Lightwave object file should have a unique index
//               number.
////////////////////////////////////////////////////////////////////
int CLwoClip::
get_index() const {
  return _clip->_index;
}

////////////////////////////////////////////////////////////////////
//     Function: CLwoClip::is_still_image
//       Access: Public
//  Description: Returns true if this clip represents a still image,
//               as opposed to an animated image.  If this is true,
//               _filename will contain the image filename.
////////////////////////////////////////////////////////////////////
bool CLwoClip::
is_still_image() const {
  return _still_image;
}
