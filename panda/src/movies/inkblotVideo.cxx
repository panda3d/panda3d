// Filename: inkblotVideo.cxx
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

#include "inkblotVideo.h"
#include "inkblotMovie.h"
#include "config_movies.h"

TypeHandle InkblotVideo::_type_handle;

////////////////////////////////////////////////////////////////////
//
// The Color-Map
//
////////////////////////////////////////////////////////////////////
struct color {
  int r,g,b;
};

static color colormap[17] = {
  { 255,0,0 },
  { 255,255,0 },
  { 0,255,0 },
  { 0,255,255 },
  { 0,0,255 },
  { 0,0,0 },
  { 255,0,0 },
  { 255,255,0 },
  { 0,255,0 },
  { 0,255,255 },
  { 0,0,255 },
  { 0,0,0 },
  { 255,0,0 },
  { 255,255,0 },
  { 0,255,0 },
  { 0,255,255 },
  { 0,0,255 },
};  

////////////////////////////////////////////////////////////////////
//     Function: InkblotVideo::Constructor
//       Access: Public
//  Description: xxx
////////////////////////////////////////////////////////////////////
InkblotVideo::
InkblotVideo(CPT(InkblotMovie) source) :
  MovieVideo((const InkblotMovie *)source)
{
  _sourcep = source;
  int padx = size_x() + 2;
  int pady = size_y() + 2;
  _cells = new unsigned char[padx * pady];
  _cells2 = new unsigned char[padx * pady];
  memset(_cells, 255, padx * pady);
  memset(_cells2, 255, padx * pady);
  _frames_read = 0;
}

////////////////////////////////////////////////////////////////////
//     Function: InkblotVideo::Destructor
//       Access: Public, Virtual
//  Description: 
////////////////////////////////////////////////////////////////////
InkblotVideo::
~InkblotVideo() {
  delete[] _cells;
  delete[] _cells2;
}

////////////////////////////////////////////////////////////////////
//     Function: InkblotVideo::fetch_into_buffer
//       Access: Published, Virtual
//  Description: See MovieVideo::fetch_into_buffer.
////////////////////////////////////////////////////////////////////
void InkblotVideo::
fetch_into_buffer(double time, unsigned char *data, bool rgba) {

  nassertv(time >= _next_start);
  
  int padx = size_x() + 2;
  int pady = size_y() + 2;
  int fps = _sourcep->get_fps();
  
  while (_next_start <= time) {
    _last_start = (_frames_read * 1.0) / fps;
    _frames_read += 1;
    _next_start = (_frames_read * 1.0) / fps;
    for (int y=1; y<pady-1; y++) {
      for (int x=1; x<padx-1; x++) {
        int tot =
          _cells[(x+1)+(y+1)*padx] +
          _cells[(x+1)+(y+0)*padx] +
          _cells[(x+1)+(y-1)*padx] +
          _cells[(x+0)+(y+1)*padx] +
          _cells[(x+0)+(y+0)*padx] +
          _cells[(x+0)+(y-1)*padx] +
          _cells[(x-1)+(y+1)*padx] +
          _cells[(x-1)+(y+0)*padx] +
          _cells[(x-1)+(y-1)*padx];
        _cells2[x + y*padx] = (tot/9)+3;
      }
    }
    unsigned char *t = _cells;
    _cells = _cells2;
    _cells2 = t;
  }

  for (int y=1; y<pady - 1; y++) {
    for (int x=1; x<padx - 1; x++) {
      int val = _cells[x + y*padx];
      color &c1 = colormap[(val>>4)+0];
      color &c2 = colormap[(val>>4)+1];
      int lerp = val & 15;
      data[0] = (c1.r * (16-lerp) + c2.r * lerp) / 16;
      data[1] = (c1.g * (16-lerp) + c2.g * lerp) / 16;
      data[2] = (c1.b * (16-lerp) + c2.b * lerp) / 16;
      if (rgba) {
        data[3] = 255;
        data += 4;
      } else {
        data += 3;
      }
    }
  }
}

