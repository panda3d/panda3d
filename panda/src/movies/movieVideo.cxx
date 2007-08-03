// Filename: movieVideo.cxx
// Created by: jyelon (02Jul07)
//
////////////////////////////////////////////////////////////////////
//
// PANDA 3D SOFTWARE
// Copyright (c) 2001 - 2007, Disney Enterprises, Inc.  All rights reserved
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

#include "movieVideo.h"
#include "config_movies.h"

TypeHandle MovieVideo::_type_handle;

////////////////////////////////////////////////////////////////////
//     Function: MovieVideo::Constructor
//       Access: Published
//  Description: This constructor returns a null video stream --- a
//               stream of plain blue and white frames that last one
//               second each. To get more interesting video, you need
//               to construct a subclass of this class.
////////////////////////////////////////////////////////////////////
MovieVideo::
MovieVideo(const string &name, double len) :
  Namable(name),
  _size_x(1),
  _size_y(1),
  _at_end(false),
  _next_start(0.0),
  _approx_len(len)
{
  if (len < 0.0) {
    _approx_len = 0.0;
  }
  if (_approx_len == 0.0) {
    _at_end = true;
  }
}

////////////////////////////////////////////////////////////////////
//     Function: MovieVideo::Destructor
//       Access: Published, Virtual
//  Description: 
////////////////////////////////////////////////////////////////////
MovieVideo::
~MovieVideo() {
}
 
////////////////////////////////////////////////////////////////////
//     Function: MovieVideo::fetch_into
//       Access: Published, Virtual
//  Description: Load the next frame into a texture's ram image.
//               Advances the frame pointer.
////////////////////////////////////////////////////////////////////
void MovieVideo::
fetch_into(Texture *t) {

  // The following is the implementation of the null video
  // stream --- a stream of solid blue frames.  Normally,
  // this method will be overridden by the subclass.
  
  t->setup_texture(Texture::TT_2d_texture, 1, 1, 1,
                   Texture::T_unsigned_byte, Texture::F_rgba);
  PTA_uchar img = t->modify_ram_image();
  
  int frame_index = (int)_next_start;
  if (_at_end) {
    img.set_element(0,0);
    img.set_element(1,0);
    img.set_element(2,0);
    img.set_element(3,255);
  } else if (frame_index & 1) {
    img.set_element(0,128);
    img.set_element(1,128);
    img.set_element(2,255);
    img.set_element(3,255);
  } else {
    img.set_element(0,255);
    img.set_element(1,255);
    img.set_element(2,255);
    img.set_element(3,255);
  }
  
  _next_start = _next_start + 1.0;
  if (_next_start > _approx_len) {
    _at_end = true;
  }
}
