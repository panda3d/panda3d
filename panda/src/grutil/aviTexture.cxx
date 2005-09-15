// Filename: aviTexture.cxx
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

#include "pandabase.h"

#ifdef HAVE_CV
#include "aviTexture.h"

TypeHandle AviTexture::_type_handle;

////////////////////////////////////////////////////////////////////
//     Function: AviTexture::Constructor
//       Access: Published
//  Description: Sets up the texture to read frames from a camera
////////////////////////////////////////////////////////////////////
AviTexture::
AviTexture() {
  _isCamera = true;
  _capture = cvCaptureFromCAM(0);
  _buf = NULL;
  _magicNum = 0;
  _time = 0.0f;
  _fps = 30.0f;
  _total_frames = 0;
  _current_frame = 0;
  
  if (_capture) {
    cvSetCaptureProperty(_capture, CV_CAP_PROP_FPS,_fps);
    _width = cvGetCaptureProperty(_capture, CV_CAP_PROP_FRAME_WIDTH);
    _height = cvGetCaptureProperty(_capture, CV_CAP_PROP_FRAME_HEIGHT);

    if (_width < _height) {
      gen_tex(_width);
    } else {
      gen_tex(_height);
    }
  }             
}


////////////////////////////////////////////////////////////////////
//     Function: AviTexture::Constructor
//       Access: Published
//  Description: Defines the texture as a movie texture. 
//               TODO: Make this search the panda paths
////////////////////////////////////////////////////////////////////
AviTexture::
AviTexture(const string &filename) {
  _isCamera = false;
  _capture = cvCaptureFromFile(filename.c_str());
  _buf = NULL;
  _magicNum = 0; 
  _time = 0.0f;
  _fps = 30.0f;
  _total_frames = 0;
  _current_frame = 0;   

  if (_capture) {
    _fps = cvGetCaptureProperty(_capture, CV_CAP_PROP_FPS);
    _total_frames = cvGetCaptureProperty(_capture, CV_CAP_PROP_FRAME_COUNT);
    _width = cvGetCaptureProperty(_capture, CV_CAP_PROP_FRAME_WIDTH);
    _height = cvGetCaptureProperty(_capture, CV_CAP_PROP_FRAME_HEIGHT);

    if (_width < _height) {
      gen_tex(_width);
    } else {
      gen_tex(_height);
    }
  }             
}

////////////////////////////////////////////////////////////////////
//     Function: AviTexture::Destructor
//       Access: Published
//  Description: Destructor. Release the camera or video stream
////////////////////////////////////////////////////////////////////
AviTexture::
~AviTexture() {
  cvReleaseCapture(&_capture);
}

////////////////////////////////////////////////////////////////////
//     Function: AviTexture::gen_tex
//       Access: Published
//  Description: Tries to find the largest texture that will fit
//               inside the video stream. TODO: allow for fit around
////////////////////////////////////////////////////////////////////
void AviTexture::
gen_tex(int magicNum) {
  int size = down_to_power_2(magicNum);

  setup_2d_texture(size, size, Texture::T_unsigned_byte, Texture::F_rgb8);
  _magicNum = size;
}

////////////////////////////////////////////////////////////////////
//     Function: AviTexture::update
//       Access: Published
//  Description: Grabs the next frame off the camera or avi file
//               Returns false if the capture fails or reached EOF 
////////////////////////////////////////////////////////////////////
bool AviTexture::
update() {
  int begx, endx, begy, endy;
  IplImage *frame = 0;
  int xs, ys;

  if (_capture) {
    if (_time == 1.0f) {
      return false;
    }
    frame = cvQueryFrame( _capture );

    if (frame) {
      _time = cvGetCaptureProperty(_capture, CV_CAP_PROP_POS_AVI_RATIO);
      _current_frame = cvGetCaptureProperty(_capture, CV_CAP_PROP_POS_FRAMES);
      if (!obtain_ram()) {
	return false;
      }
      begx = (_width - _magicNum) / 2.0;
      endx = _width - begx;
      begy = (_height - _magicNum) / 2.0;
      endy = _height - begy;
      
      if (_buf) {
	xs = get_x_size();
	ys = get_y_size();
	
	if (get_num_components() != 3) {
	  return false;
        }
	
	for (int i = begy; i < endy; ++i) {
	  memcpy(_buf + ((i - begy) * xs * 3),
                 frame->imageData + i * _width * 3 + begx * 3,
                 xs * 3);
        }
	
	return true;
      }
    }
  }

  return false;
}

#endif  // HAVE_CV

