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
//       Access: Public
//  Description: This constructor returns a null video stream --- a
//               stream of plain blue and white frames that last one
//               second each. To get more interesting video, you need
//               to construct a subclass of this class.
////////////////////////////////////////////////////////////////////
MovieVideo::
MovieVideo(CPT(Movie) source) :
  Namable(source->get_name()),
  _source(source),
  _aborted(false),
  _last_start(-1.0),
  _next_start(0.0)
{
}

////////////////////////////////////////////////////////////////////
//     Function: MovieVideo::Destructor
//       Access: Public, Virtual
//  Description: 
////////////////////////////////////////////////////////////////////
MovieVideo::
~MovieVideo() {
  if (_conversion_buffer != 0) {
    delete[] _conversion_buffer;
  }
}

////////////////////////////////////////////////////////////////////
//     Function: MovieVideo::allocate_conversion_buffer
//       Access: Private
//  Description: The generic implementations of fetch_into_texture
//               and fetch_into_alpha require the use of a conversion
//               buffer.  This allocates the buffer.
////////////////////////////////////////////////////////////////////
void MovieVideo::
allocate_conversion_buffer() {
  if (_conversion_buffer == 0) {
    _conversion_buffer = new unsigned char[size_x() * size_y() * 4];
  }
}
  
////////////////////////////////////////////////////////////////////
//     Function: MovieVideo::fetch_into_texture
//       Access: Published, Virtual
//  Description: Reads frames from the stream until the specified 
//               time is reached.  The last frame read is stored in
//               the supplied texture.
//
//               See fetch_into_buffer for more details.
////////////////////////////////////////////////////////////////////
void MovieVideo::
fetch_into_texture(double time, Texture *t, int page) {

  // This generic implementation is layered on fetch_into_buffer.
  // It will work for any derived class, so it is never necessary to
  // redefine this.  However, it may be possible to make a faster
  // implementation that uses fewer intermediate copies, depending
  // on the capabilities of the underlying codec software.

  nassertv(t->get_x_size() >= size_x());
  nassertv(t->get_y_size() >= size_y());
  nassertv((t->get_num_components() == 3) || (t->get_num_components() == 4));
  nassertv(t->get_component_width() == 1);
  nassertv(page < t->get_z_size());
  
  PTA_uchar img = t->modify_ram_image();
  
  unsigned char *data = img.p() + page * t->get_expected_ram_page_size();

  if (t->get_x_size() == size_x()) {
    fetch_into_buffer(time, data, t->get_num_components() == 4);
  } else {
    allocate_conversion_buffer();
    fetch_into_buffer(time, _conversion_buffer, t->get_num_components() == 4);
    int src_stride = size_x() * t->get_num_components();
    int dst_stride = t->get_x_size() * t->get_num_components();
    unsigned char *p = _conversion_buffer;
    for (int y=0; y<size_y(); y++) {
      memcpy(data, p, src_stride);
      data += dst_stride;
      p += src_stride;
    }
  }
}

////////////////////////////////////////////////////////////////////
//     Function: MovieVideo::fetch_into_texture_alpha
//       Access: Published, Virtual
//  Description: Reads frames from the stream until the specified 
//               time is reached.  The last frame read is stored in
//               the alpha channel of the supplied texture.  The
//               RGB channels of the texture are not touched.
//
//               See fetch_into_buffer for more details.
////////////////////////////////////////////////////////////////////
void MovieVideo::
fetch_into_texture_alpha(double time, Texture *t, int page, int alpha_src) {

  // This generic implementation is layered on fetch_into_buffer.
  // It will work for any derived class, so it is never necessary to
  // redefine this.  However, it may be possible to make a faster
  // implementation that uses fewer intermediate copies, depending
  // on the capabilities of the underlying codec software.

  nassertv(t->get_x_size() >= size_x());
  nassertv(t->get_y_size() >= size_y());
  nassertv(t->get_num_components() == 4);
  nassertv(t->get_component_width() == 1);
  nassertv(page < t->get_z_size());
  nassertv((alpha_src >= 0) && (alpha_src <= 4));

  allocate_conversion_buffer();
  
  fetch_into_buffer(time, _conversion_buffer, true);
  
  PTA_uchar img = t->modify_ram_image();
  
  unsigned char *data = img.p() + page * t->get_expected_ram_page_size();
  
  int src_stride = size_x() * 4;
  int dst_stride = t->get_x_size() * 4;
  if (alpha_src == 0) {
    unsigned char *p = _conversion_buffer;
    for (int y=0; y<size_y(); y++) {
      for (int x=0; x<size_x(); x++) {
        data[x*4+3] = (p[x*4+0] + p[x*4+1] + p[x*4+2]) / 3;
      }
      data += dst_stride;
      p += src_stride;
    }
  } else {
    alpha_src -= 1;
    unsigned char *p = _conversion_buffer;
    for (int y=0; y<size_y(); y++) {
      for (int x=0; x<size_x(); x++) {
        data[x*4+3] = p[x*4+alpha_src];
      }
      data += dst_stride;
      p += src_stride;
    }
  }
}

////////////////////////////////////////////////////////////////////
//     Function: MovieVideo::fetch_into_texture_rgb
//       Access: Published, Virtual
//  Description: Reads frames from the stream until the specified 
//               time is reached.  The last frame read is stored in
//               the RGB channels of the supplied texture.  The alpha
//               channel of the texture is not touched.
//
//               See fetch_into_buffer for more details.
////////////////////////////////////////////////////////////////////
void MovieVideo::
fetch_into_texture_rgb(double time, Texture *t, int page) {

  // This generic implementation is layered on fetch_into_buffer.
  // It will work for any derived class, so it is never necessary to
  // redefine this.  However, it may be possible to make a faster
  // implementation that uses fewer intermediate copies, depending
  // on the capabilities of the underlying codec software.

  nassertv(t->get_x_size() >= size_x());
  nassertv(t->get_y_size() >= size_y());
  nassertv(t->get_num_components() == 4);
  nassertv(t->get_component_width() == 1);
  nassertv(page < t->get_z_size());

  allocate_conversion_buffer();
  
  fetch_into_buffer(time, _conversion_buffer, true);
  
  PTA_uchar img = t->modify_ram_image();
  
  unsigned char *data = img.p() + page * t->get_expected_ram_page_size();
  
  int src_stride = size_x() * 4;
  int dst_stride = t->get_x_size() * 4;
  unsigned char *p = _conversion_buffer;
  for (int y=0; y<size_y(); y++) {
    for (int x=0; x<size_x(); x++) {
      data[x*4+0] = p[x*4+0];
      data[x*4+1] = p[x*4+1];
      data[x*4+2] = p[x*4+2];
    }
    data += dst_stride;
    p += src_stride;
  }
}

////////////////////////////////////////////////////////////////////
//     Function: MovieVideo::fetch_into_buffer
//       Access: Published, Virtual
//  Description: Reads frames from the stream until the specified 
//               time is reached.  The last frame read is stored in
//               the supplied RGB8 or RGBA8 buffer.
//
//               The fetch methods do not seek: they just reads frames
//               from the stream.  Therefore, they cannot move backward.
//               If you specify a time value less than next_start, they
//               will just read one frame and return.
//
//               To truly seek, you must call get_video with an
//               offset.  This is sometimes inaccurate, because AVI
//               file indices often contain errors.  Therefore, it
//               is sometimes advantageous to use fetch methods to 
//               just read frames from the stream until you get
//               to the target location.
////////////////////////////////////////////////////////////////////
void MovieVideo::
fetch_into_buffer(double time, unsigned char *data, bool rgba) {
  
  // The following is the implementation of the null video stream, ie,
  // a stream of blinking red and blue frames.  This method must be
  // overridden by the subclass.

  if (time < _next_start) time = _next_start;
  _last_start = floor(time);
  _next_start = _last_start + 1;

  if (_last_start >= length()) {
    data[0] = 0;
    data[1] = 0;
    data[2] = 0;
  } else if (((int)_last_start) & 1) {
    data[0] = 255;
    data[1] = 128;
    data[2] = 128;
  } else {
    data[0] = 128;
    data[1] = 128;
    data[2] = 255;
  }
  if (rgba) {
    data[3] = 255;
  }
}

