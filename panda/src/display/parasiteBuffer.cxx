// Filename: parasiteBuffer.cxx
// Created by:  drose (27Feb04)
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


#include "parasiteBuffer.h"

TypeHandle ParasiteBuffer::_type_handle;

////////////////////////////////////////////////////////////////////
//     Function: ParasiteBuffer::Constructor
//       Access: Public
//  Description: Normally, the ParasiteBuffer constructor is not
//               called directly; these are created instead via the
//               GraphicsEngine::make_parasite() function.
////////////////////////////////////////////////////////////////////
ParasiteBuffer::
ParasiteBuffer(GraphicsOutput *host, const string &name,
               int x_size, int y_size) :
  GraphicsOutput(host->get_pipe(), host->get_gsg(), name),
  _host(host)
{
#ifdef DO_MEMORY_USAGE
  MemoryUsage::update_type(this, this);
#endif
  
  if (display_cat.is_debug()) {
    display_cat.debug()
      << "Creating new parasite buffer " << get_name()
      << " on " << _host->get_name() << "\n";
  }

  if ((x_size == 0)&&(y_size == 0)) {
    _track_host_size = true;
    x_size = host->get_x_size();
    y_size = host->get_y_size();
  } else {
    _track_host_size = false;
  }
  
  _x_size = x_size;
  _y_size = y_size;
  _has_size = true;
  _default_display_region->compute_pixels(_x_size, _y_size);
  _is_valid = true;
  
  nassertv(_x_size <= host->get_x_size() && _y_size <= host->get_y_size());
}

////////////////////////////////////////////////////////////////////
//     Function: ParasiteBuffer::Destructor
//       Access: Published, Virtual
//  Description: 
////////////////////////////////////////////////////////////////////
ParasiteBuffer::
~ParasiteBuffer() {
  _is_valid = false;
}

////////////////////////////////////////////////////////////////////
//     Function: ParasiteBuffer::is_active
//       Access: Published, Virtual
//  Description: Returns true if the window is ready to be rendered
//               into, false otherwise.
////////////////////////////////////////////////////////////////////
bool ParasiteBuffer::
is_active() const {
  return _active && _host->is_active();
}

////////////////////////////////////////////////////////////////////
//     Function: ParasiteBuffer::get_host
//       Access: Public, Virtual
//  Description: This is normally called only from within
//               make_texture_buffer().  When called on a
//               ParasiteBuffer, it returns the host of that buffer;
//               but when called on some other buffer, it returns the
//               buffer itself.
////////////////////////////////////////////////////////////////////
GraphicsOutput *ParasiteBuffer::
get_host() {
  return _host;
}

////////////////////////////////////////////////////////////////////
//     Function: ParasiteBuffer::make_current
//       Access: Public, Virtual
//  Description: This function will be called within the draw thread
//               during begin_frame() to ensure the graphics context
//               is ready for drawing.
////////////////////////////////////////////////////////////////////
void ParasiteBuffer::
make_current() {
  _host->make_current();
}

////////////////////////////////////////////////////////////////////
//     Function: ParasiteBuffer::auto_resize
//       Access: Public, Virtual
//  Description: This function will be called within the draw thread
//               to make sure this buffer is the right size.  If not,
//               it will be resized.
////////////////////////////////////////////////////////////////////
void ParasiteBuffer::
auto_resize() {
  if (_track_host_size) {
    _host->auto_resize();
    if ((_host->get_x_size() != _x_size)||
        (_host->get_y_size() != _y_size)) {
      set_size_and_recalc(_host->get_x_size(),
                          _host->get_y_size());
    }
  }
}
