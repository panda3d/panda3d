// Filename: parasiteBuffer.cxx
// Created by:  drose (27Feb04)
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

#include "parasiteBuffer.h"
#include "texture.h"

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
               int x_size, int y_size, int flags) :
  GraphicsOutput(host->get_engine(), host->get_pipe(), 
                 name, host->get_fb_properties(),
                 WindowProperties::size(x_size, y_size), flags, 
                 host->get_gsg(), host)
{
#ifdef DO_MEMORY_USAGE
  MemoryUsage::update_type(this, this);
#endif
  
  if (display_cat.is_debug()) {
    display_cat.debug()
      << "Creating new parasite buffer " << get_name()
      << " on " << _host->get_name() << "\n";
  }

  _creation_flags = flags;
  
  if (flags & GraphicsPipe::BF_size_track_host) {
    x_size = host->get_x_size();
    y_size = host->get_y_size();
  }
  
  _x_size = x_size;
  _y_size = y_size;
  _has_size = true;
  _default_display_region->compute_pixels(_x_size, _y_size);
  _is_valid = true;
  
  set_inverted(host->get_gsg()->get_copy_texture_inverted());
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
//     Function: ParasiteBuffer::set_size
//       Access: Public, Virtual
//  Description: This is called by the GraphicsEngine to request that
//               the buffer resize itself.  Although calls to get the
//               size will return the new value, much of the actual
//               resizing work doesn't take place until the next
//               begin_frame.  Not all buffers are resizeable.
////////////////////////////////////////////////////////////////////
void ParasiteBuffer::
set_size(int x, int y) {
  if ((_creation_flags & GraphicsPipe::BF_resizeable) == 0) {
    nassert_raise("Cannot resize buffer unless it is created with BF_resizeable flag");
    return;
  }
  set_size_and_recalc(x, y);
}

////////////////////////////////////////////////////////////////////
//     Function: ParasiteBuffer::set_size_and_recalc
//       Access: Public
//  Description: 
////////////////////////////////////////////////////////////////////
void ParasiteBuffer::
set_size_and_recalc(int x, int y) {
  if (!(_creation_flags & GraphicsPipe::BF_size_track_host)) {
    if (_creation_flags & GraphicsPipe::BF_size_power_2) {
      x = Texture::down_to_power_2(x);
      y = Texture::down_to_power_2(y);
    }
    if (_creation_flags & GraphicsPipe::BF_size_square) {
      x = y = min(x, y);
    }
  }

  GraphicsOutput::set_size_and_recalc(x, y);
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
//     Function: ParasiteBuffer::begin_frame
//       Access: Public, Virtual
//  Description: This function will be called within the draw thread
//               before beginning rendering for a given frame.  It
//               should do whatever setup is required, and return true
//               if the frame should be rendered, or false if it
//               should be skipped.
////////////////////////////////////////////////////////////////////
bool ParasiteBuffer::
begin_frame(FrameMode mode, Thread *current_thread) {
  begin_frame_spam(mode);

  if (!_host->begin_frame(FM_parasite, current_thread)) {
    return false;
  }

  if (_creation_flags & GraphicsPipe::BF_size_track_host) {
    if ((_host->get_x_size() != _x_size)||
        (_host->get_y_size() != _y_size)) {
      set_size_and_recalc(_host->get_x_size(),
                          _host->get_y_size());
    }
  } else {
    if (_host->get_x_size() < _x_size ||
        _host->get_y_size() < _y_size) {
      set_size_and_recalc(min(_x_size, _host->get_x_size()),
                          min(_y_size, _host->get_y_size()));
    }
  }
  
  clear_cube_map_selection();
  return true;
}

////////////////////////////////////////////////////////////////////
//     Function: ParasiteBuffer::end_frame
//       Access: Public, Virtual
//  Description: This function will be called within the draw thread
//               after rendering is completed for a given frame.  It
//               should do whatever finalization is required.
////////////////////////////////////////////////////////////////////
void ParasiteBuffer::
end_frame(FrameMode mode, Thread *current_thread) {
  end_frame_spam(mode);

  nassertv(_gsg != (GraphicsStateGuardian *)NULL);

  _host->end_frame(FM_parasite, current_thread);

  if (mode == FM_refresh) {
    return;
  }
  
  if (mode == FM_render) {
    for (int i=0; i<count_textures(); i++) {
      if (get_rtm_mode(i) == RTM_bind_or_copy) {
        _textures[i]._rtm_mode = RTM_copy_texture;
      }
    }
    copy_to_textures();
    if (_one_shot) {
      prepare_for_deletion();
    }
    clear_cube_map_selection();
  }
}

