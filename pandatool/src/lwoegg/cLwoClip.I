// Filename: cLwoClip.I
// Created by:  drose (27Apr01)
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
