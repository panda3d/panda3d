// Filename: cLwoClip.I
// Created by:  drose (27Apr01)
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
