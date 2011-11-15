// Filename: inkblotVideoCursor.cxx
// Created by: jyelon (02Jul07)
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

#include "inkblotVideoCursor.h"
#include "config_movies.h"

TypeHandle InkblotVideoCursor::_type_handle;

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
//     Function: InkblotVideoCursor::Constructor
//       Access: Public
//  Description: xxx
////////////////////////////////////////////////////////////////////
InkblotVideoCursor::
InkblotVideoCursor(InkblotVideo *src) :
  MovieVideoCursor(src)
{
  _size_x = src->_specified_x;
  _size_y = src->_specified_y;
  _num_components = 3;
  _fps = src->_specified_fps;
  int padx = _size_x + 2;
  int pady = _size_y + 2;
  _cells = new unsigned char[padx * pady];
  _cells2 = new unsigned char[padx * pady];
  memset(_cells, 255, padx * pady);
  memset(_cells2, 255, padx * pady);
  _can_seek = true;
  _can_seek_fast = false;
  _current_frame = 0;
  _last_frame = -1;
}

////////////////////////////////////////////////////////////////////
//     Function: InkblotVideoCursor::Destructor
//       Access: Public, Virtual
//  Description: 
////////////////////////////////////////////////////////////////////
InkblotVideoCursor::
~InkblotVideoCursor() {
  delete[] _cells;
  delete[] _cells2;
}

////////////////////////////////////////////////////////////////////
//     Function: InkblotVideoCursor::set_time
//       Access: Published, Virtual
//  Description: See MovieVideoCursor::set_time().
////////////////////////////////////////////////////////////////////
bool InkblotVideoCursor::
set_time(double time, int loop_count) {
  int frame = (int)(time / _fps);
  if (frame == _current_frame) {
    return false;
  }

  _current_frame = frame;
  return true;
}

////////////////////////////////////////////////////////////////////
//     Function: InkblotVideoCursor::fetch_buffer
//       Access: Published, Virtual
//  Description: See MovieVideoCursor::fetch_buffer.
////////////////////////////////////////////////////////////////////
PT(MovieVideoCursor::Buffer) InkblotVideoCursor::
fetch_buffer() {
  PT(Buffer) buffer = get_standard_buffer();

  int padx = size_x() + 2;
  int pady = size_y() + 2;
  
  if (_current_frame < _last_frame) {
    // Rewind to beginning.
    memset(_cells, 255, padx * pady);
    memset(_cells2, 255, padx * pady);
    _last_frame = 0;
  }
  
  while (_last_frame <= _current_frame) {
    ++_last_frame;
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

  unsigned char *data = buffer->_block;
  for (int y=1; y<pady - 1; y++) {
    for (int x=1; x<padx - 1; x++) {
      int val = _cells[x + y*padx];
      color &c1 = colormap[(val>>4)+0];
      color &c2 = colormap[(val>>4)+1];
      int lerp = val & 15;
      data[0] = (c1.b * (16-lerp) + c2.b * lerp) / 16;
      data[1] = (c1.g * (16-lerp) + c2.g * lerp) / 16;
      data[2] = (c1.r * (16-lerp) + c2.r * lerp) / 16;
      data += 3;
    }
  }

  return buffer;
}

