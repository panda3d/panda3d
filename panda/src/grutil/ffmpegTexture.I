// Filename: openCVTexture.I
// Created by:  zacpavlov (19Aug05)
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
//     Function: FFMpegTexture::VideoStream::is_valid
//       Access: Public
//  Description: Returns true if this stream is open and ready, false
//               otherwise.
////////////////////////////////////////////////////////////////////
INLINE bool FFMpegTexture::VideoStream::
is_valid() const {
  return (_format_context != NULL && _codec_context != NULL);
}

////////////////////////////////////////////////////////////////////
//     Function: FFMpegTexture::VideoStream::is_from_file
//       Access: Public
//  Description: Returns true if this stream takes its input from a
//               video file, false otherwise.
////////////////////////////////////////////////////////////////////
INLINE bool FFMpegTexture::VideoStream::
is_from_file() const {
  return true;
}

////////////////////////////////////////////////////////////////////
//     Function: FFMpegTexture::VideoPage::Constructor
//       Access: Public
//  Description: 
////////////////////////////////////////////////////////////////////
INLINE FFMpegTexture::VideoPage::
VideoPage() {
}

////////////////////////////////////////////////////////////////////
//     Function: FFMpegTexture::VideoPage::Copy Constructor
//       Access: Public
//  Description: 
////////////////////////////////////////////////////////////////////
INLINE FFMpegTexture::VideoPage::
VideoPage(const FFMpegTexture::VideoPage &copy) :
  _color(copy._color)
{
}

////////////////////////////////////////////////////////////////////
//     Function: FFMpegTexture::VideoPage::Destructor
//       Access: Public
//  Description: 
////////////////////////////////////////////////////////////////////
INLINE FFMpegTexture::VideoPage::
~VideoPage() {
}
