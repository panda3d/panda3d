/**
 * PANDA 3D SOFTWARE
 * Copyright (c) Carnegie Mellon University.  All rights reserved.
 *
 * All use of this software is subject to the terms of the revised BSD
 * license.  You should have received a copy of this license along
 * with this source code in a file named "LICENSE."
 *
 * @file parasiteBuffer.cxx
 * @author drose
 * @date 2004-02-27
 */

#include "parasiteBuffer.h"
#include "texture.h"

TypeHandle ParasiteBuffer::_type_handle;

/**
 * Normally, the ParasiteBuffer constructor is not called directly; these are
 * created instead via the GraphicsEngine::make_parasite() function.
 */
ParasiteBuffer::
ParasiteBuffer(GraphicsOutput *host, const std::string &name,
               int x_size, int y_size, int flags) :
  GraphicsOutput(host->get_engine(), host->get_pipe(),
                 name, host->get_fb_properties(),
                 WindowProperties::size(x_size, y_size), flags,
                 host->get_gsg(), host, false)
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
    _size = host->get_size();
  } else {
    _size.set(x_size, y_size);
  }

  _has_size = true;
  _overlay_display_region->compute_pixels(_size.get_x(), _size.get_y());
  _is_valid = true;

  set_inverted(host->get_gsg()->get_copy_texture_inverted());
}

/**
 *
 */
ParasiteBuffer::
~ParasiteBuffer() {
  _is_valid = false;
}

/**
 * Returns true if the window is ready to be rendered into, false otherwise.
 */
bool ParasiteBuffer::
is_active() const {
  return GraphicsOutput::is_active() && _host->is_active();
}

/**
 * This is called by the GraphicsEngine to request that the buffer resize
 * itself.  Although calls to get the size will return the new value, much of
 * the actual resizing work doesn't take place until the next begin_frame.
 * Not all buffers are resizeable.
 */
void ParasiteBuffer::
set_size(int x, int y) {
  if ((_creation_flags & GraphicsPipe::BF_resizeable) == 0) {
    nassert_raise("Cannot resize buffer unless it is created with BF_resizeable flag");
    return;
  }
  set_size_and_recalc(x, y);
}

/**
 *
 */
void ParasiteBuffer::
set_size_and_recalc(int x, int y) {
  if (!(_creation_flags & GraphicsPipe::BF_size_track_host)) {
    if (_creation_flags & GraphicsPipe::BF_size_power_2) {
      x = Texture::down_to_power_2(x);
      y = Texture::down_to_power_2(y);
    }
    if (_creation_flags & GraphicsPipe::BF_size_square) {
      x = y = std::min(x, y);
    }
  }

  GraphicsOutput::set_size_and_recalc(x, y);
}

/**
 * Returns true if a frame has been rendered and needs to be flipped, false
 * otherwise.
 */
bool ParasiteBuffer::
flip_ready() const {
  nassertr(_host != nullptr, false);
  return _host->flip_ready();
}

/**
 * This function will be called within the draw thread after end_frame() has
 * been called on all windows, to initiate the exchange of the front and back
 * buffers.
 *
 * This should instruct the window to prepare for the flip at the next video
 * sync, but it should not wait.
 *
 * We have the two separate functions, begin_flip() and end_flip(), to make it
 * easier to flip all of the windows at the same time.
 */
void ParasiteBuffer::
begin_flip() {
  nassertv(_host != nullptr);
  _host->begin_flip();
}

/**
 * This function will be called within the draw thread after end_frame() has
 * been called on all windows, to initiate the exchange of the front and back
 * buffers.
 *
 * This should instruct the window to prepare for the flip when it is command
 * but not actually flip
 *
 */
void ParasiteBuffer::
ready_flip() {
  nassertv(_host != nullptr);
  _host->ready_flip();
}

/**
 * This function will be called within the draw thread after begin_flip() has
 * been called on all windows, to finish the exchange of the front and back
 * buffers.
 *
 * This should cause the window to wait for the flip, if necessary.
 */
void ParasiteBuffer::
end_flip() {
  nassertv(_host != nullptr);
  _host->end_flip();
  _flip_ready = false;
}

/**
 * This function will be called within the draw thread before beginning
 * rendering for a given frame.  It should do whatever setup is required, and
 * return true if the frame should be rendered, or false if it should be
 * skipped.
 */
bool ParasiteBuffer::
begin_frame(FrameMode mode, Thread *current_thread) {
  begin_frame_spam(mode);

  if (!_host->begin_frame(FM_parasite, current_thread)) {
    return false;
  }

  if (_creation_flags & GraphicsPipe::BF_size_track_host) {
    if (_host->get_size() != _size) {
      set_size_and_recalc(_host->get_x_size(),
                          _host->get_y_size());
    }
  } else {
    if (_host->get_x_size() < get_x_size() ||
        _host->get_y_size() < get_y_size()) {
      set_size_and_recalc(std::min(get_x_size(), _host->get_x_size()),
                          std::min(get_y_size(), _host->get_y_size()));
    }
  }

  clear_cube_map_selection();
  return true;
}

/**
 * This function will be called within the draw thread after rendering is
 * completed for a given frame.  It should do whatever finalization is
 * required.
 */
void ParasiteBuffer::
end_frame(FrameMode mode, Thread *current_thread) {
  end_frame_spam(mode);

  nassertv(_gsg != nullptr);

  _host->end_frame(FM_parasite, current_thread);

  if (mode == FM_refresh) {
    return;
  }

  if (mode == FM_render) {
    promote_to_copy_texture();
    copy_to_textures();
    clear_cube_map_selection();
  }
}

/**
 * This is normally called only from within make_texture_buffer().  When
 * called on a ParasiteBuffer, it returns the host of that buffer; but when
 * called on some other buffer, it returns the buffer itself.
 */
GraphicsOutput *ParasiteBuffer::
get_host() {
  return _host;
}
