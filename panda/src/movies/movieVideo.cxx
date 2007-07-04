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
//               stream of plain blue frames that last 1 second each.
//               To get more interesting video, you need to construct
//               a subclass of this class.
////////////////////////////////////////////////////////////////////
MovieVideo::
MovieVideo(const string &name, double len) :
  Namable(name),
  _size_x(1),
  _size_y(1),
  _approx_len(len),
  _frame_start(0.0),
  _frame_end(1.0)
{
  if (len <= 0.0) {
    _approx_len = 1.0;
  }
  if (_frame_end > _approx_len) {
    _frame_end = _approx_len;
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
//     Function: MovieVideo::load_image
//       Access: Published, Virtual
//  Description: Copy the current frame into a texture's ram image.
////////////////////////////////////////////////////////////////////
void MovieVideo::
load_image(Texture *t) {

  // The following is the implementation of the null video
  // stream --- a stream of solid blue frames.  Normally,
  // this method will be overridden by the subclass.
  
  if (_ram_image==0) {
    _ram_image = PTA_uchar::empty_array(4);
    _ram_image.set_element(0,128);
    _ram_image.set_element(1,128);
    _ram_image.set_element(2,255);
    _ram_image.set_element(3,255);
  }
  t->setup_texture(Texture::TT_2d_texture, 1, 1, 1,
                   Texture::T_unsigned_byte, Texture::F_rgba);
  t->set_ram_image(_ram_image);
}

////////////////////////////////////////////////////////////////////
//     Function: MovieVideo::next_frame
//       Access: Published, Virtual
//  Description: Advances to the next frame.
////////////////////////////////////////////////////////////////////
void MovieVideo::
next_frame() {

  // The following is the implementation of the null video
  // stream --- a stream of solid blue frames.  Normally,
  // this method will be overridden by the subclass.

  _frame_start = _frame_end;
  _frame_end = _frame_end + 1.0;
  if (_frame_end > _approx_len) {
    _frame_end = _approx_len;
  }
}
