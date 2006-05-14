// Filename: openCVTexture.I
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
//     Function: FFMpegTexture::VideoStream::is_valid
//       Access: Public
//  Description: Returns true if this stream is open and ready, false
//               otherwise.
////////////////////////////////////////////////////////////////////
INLINE bool FFMpegTexture::VideoStream::
is_valid() const {
  return (pFormatCtx!=NULL && pCodecCtx!=NULL);
}

////////////////////////////////////////////////////////////////////
//     Function: FFMpegTexture::VideoStream::is_from_file
//       Access: Public
//  Description: Returns true if this stream takes its input from a
//               video file, false otherwise.
////////////////////////////////////////////////////////////////////
INLINE bool FFMpegTexture::VideoStream::
is_from_file() const {
  return 1;
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
