// Filename: aviTexture.I
// Created by:  zacpavlov (19Aug05)
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
//     Function: AviTexture::obtain_ram
//       Access: Private
//  Description: Grabs a fresh ram buffer to mess with
//
////////////////////////////////////////////////////////////////////
INLINE bool AviTexture::
obtain_ram() {
  _buf = modify_ram_image();

  if (_buf) {
    return true;
  }
  return false;
}


////////////////////////////////////////////////////////////////////
//     Function: AviTexture::get_time
//       Access: Published
//  Description: returns the ratio between 0 and 1 that the movie is
//               positioned at. Disabled for camera input
//
////////////////////////////////////////////////////////////////////
INLINE float AviTexture::
get_time() {
  if (!_isCamera) {
    return _time;
  }
  return 0.0f;
}

////////////////////////////////////////////////////////////////////
//     Function: AviTexture::set_time
//       Access: Published
//  Description: sets the time on the opencv video reader. Disabled 
//               for camera input
//
////////////////////////////////////////////////////////////////////
INLINE void AviTexture::
set_time(float t) {
  if (_isCamera) {
    return;
  }
  if (t >= 0 && t <= 1) { 
    _time=t;
  } 
  if (_capture) {
    cvSetCaptureProperty(_capture,CV_CAP_PROP_POS_AVI_RATIO,t);
    _current_frame = cvGetCaptureProperty(_capture,CV_CAP_PROP_POS_FRAMES);
  }
}

////////////////////////////////////////////////////////////////////
//     Function: AviTexture::set_fps
//       Access: Published
//  Description: sets the fps for the movie, and affects frame receives
//               from camera. Note, since frame update is controlled
//               by python, this item is sort of arbitrary
////////////////////////////////////////////////////////////////////
INLINE void AviTexture::
set_fps(float fps) {
  if (fps >= 0) {
    _fps = fps;
  } else {
    return;
  }
  if (_isCamera && _capture) {
    cvSetCaptureProperty(_capture,CV_CAP_PROP_FPS,int(fps));
  }
}

////////////////////////////////////////////////////////////////////
//     Function: AviTexture::get_fps
//       Access: Published
//  Description: returns the fps as recieved from movie
////////////////////////////////////////////////////////////////////
INLINE float AviTexture::
get_fps() {
  return _fps;
}

////////////////////////////////////////////////////////////////////
//     Function: AviTexture::get_total_frames
//       Access: Published
//  Description: returns the total amount of frames
////////////////////////////////////////////////////////////////////
INLINE int AviTexture::
get_total_frames() {
  return _total_frames;
}

////////////////////////////////////////////////////////////////////
//     Function: AviTexture::get_current_frame
//       Access: Published
//  Description: returns current frame (base 0)
////////////////////////////////////////////////////////////////////
INLINE int AviTexture::
get_current_frame() {
  return _current_frame;
}

////////////////////////////////////////////////////////////////////
//     Function: AviTexture::set_current_frame
//       Access: Published
//  Description: sets the current frame (base 0)
////////////////////////////////////////////////////////////////////
INLINE void AviTexture::
set_current_frame(int frame) {
  if (frame>=0 && frame<_total_frames) {
    _current_frame = frame;
    cvSetCaptureProperty(_capture,CV_CAP_PROP_POS_FRAMES,_current_frame);
    _time = cvGetCaptureProperty(_capture,CV_CAP_PROP_POS_AVI_RATIO);
  }
}
