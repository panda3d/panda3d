// Filename: graphicsOutput.cxx
// Created by:  drose (06Feb04)
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

#include "graphicsOutput.h"
#include "graphicsPipe.h"
#include "graphicsEngine.h"
#include "graphicsWindow.h"
#include "config_display.h"
#include "mutexHolder.h"
#include "hardwareChannel.h"
#include "renderBuffer.h"
#include "pStatTimer.h"

TypeHandle GraphicsOutput::_type_handle;

PStatCollector GraphicsOutput::_make_current_pcollector("Draw:Make current");
PStatCollector GraphicsOutput::_copy_texture_pcollector("Draw:Copy texture");

////////////////////////////////////////////////////////////////////
//     Function: GraphicsOutput::Constructor
//       Access: Protected
//  Description: Normally, the GraphicsOutput constructor is not
//               called directly; these are created instead via the
//               GraphicsEngine::make_window() function.
////////////////////////////////////////////////////////////////////
GraphicsOutput::
GraphicsOutput(GraphicsPipe *pipe, GraphicsStateGuardian *gsg, 
               const string &name) {
#ifdef DO_MEMORY_USAGE
  MemoryUsage::update_type(this, this);
#endif
  _pipe = pipe;
  _gsg = gsg;
  _name = name;
  _x_size = 0;
  _y_size = 0;
  _has_size = false;
  _is_valid = false;
  _copy_texture = false;
  _flip_ready = false;
  _needs_context = true;
  _sort = 0;
  _active = true;
  _one_shot = false;
  _delete_flag = false;

  int mode = gsg->get_properties().get_frame_buffer_mode();
  if ((mode & FrameBufferProperties::FM_buffer) == FrameBufferProperties::FM_single_buffer) {
    // Single buffered; we must draw into the front buffer.
    _draw_buffer_type = RenderBuffer::T_front;
  }

  _display_regions_stale = false;

  // By default, each new GraphicsOutput is set up to clear color and
  // depth.
  set_clear_color_active(true);
  set_clear_depth_active(true);
}

////////////////////////////////////////////////////////////////////
//     Function: GraphicsOutput::Copy Constructor
//       Access: Private
//  Description:
////////////////////////////////////////////////////////////////////
GraphicsOutput::
GraphicsOutput(const GraphicsOutput &) {
  nassertv(false);
}

////////////////////////////////////////////////////////////////////
//     Function: GraphicsOutput::Copy Assignment Operator
//       Access: Private
//  Description:
////////////////////////////////////////////////////////////////////
void GraphicsOutput::
operator = (const GraphicsOutput &) {
  nassertv(false);
}

////////////////////////////////////////////////////////////////////
//     Function: GraphicsOutput::Destructor
//       Access: Published, Virtual
//  Description:
////////////////////////////////////////////////////////////////////
GraphicsOutput::
~GraphicsOutput() {
  // The window should be closed by the time we destruct.
  nassertv(!is_valid());

  // We shouldn't have a GraphicsPipe pointer anymore.
  nassertv(_pipe == (GraphicsPipe *)NULL);

  // We don't have to destruct our child channels explicitly, since
  // they are all reference-counted and will go away when their
  // pointers do.  However, we do need to zero out their pointers to
  // us.
  Channels::iterator ci;
  for (ci = _channels.begin(); ci != _channels.end(); ++ci) {
    (*ci)->_window = NULL;
  }
}

////////////////////////////////////////////////////////////////////
//     Function: GraphicsOutput::detach_texture
//       Access: Published
//  Description: Disassociates the texture from the GraphicsOutput.
//               It will no longer be filled as the frame renders, and
//               it may be used (with its current contents) as a
//               texture in its own right.
////////////////////////////////////////////////////////////////////
void GraphicsOutput::
detach_texture() {
  MutexHolder holder(_lock);
  _texture = NULL;
  _copy_texture = false;
}

////////////////////////////////////////////////////////////////////
//     Function: GraphicsOutput::setup_copy_texture
//       Access: Published
//  Description: Creates a new Texture object, suitable for copying
//               the contents of this buffer into, and stores it in
//               _texture.  This also disassociates the previous
//               texture (if any).
////////////////////////////////////////////////////////////////////
void GraphicsOutput::
setup_copy_texture(const string &name) {
  MutexHolder holder(_lock);

  _texture = new Texture();
  _texture->set_name(name);
  _texture->set_wrapu(Texture::WM_clamp);
  _texture->set_wrapv(Texture::WM_clamp);

  // We should match the texture format up with the framebuffer
  // format.  Easier said than done!
  if (_gsg != (GraphicsStateGuardian *)NULL) {
    int mode = _gsg->get_properties().get_frame_buffer_mode();
    PixelBuffer *pb = _texture->_pbuffer;

    if (mode & FrameBufferProperties::FM_alpha) {
      pb->set_format(PixelBuffer::F_rgba8);
      pb->set_num_components(4);
      pb->set_component_width(1);
      pb->set_image_type(PixelBuffer::T_unsigned_byte);

    } else {
      pb->set_format(PixelBuffer::F_rgb8);
      pb->set_num_components(3);
      pb->set_component_width(1);
      pb->set_image_type(PixelBuffer::T_unsigned_byte);
    }
  }

  _copy_texture = true;
}

////////////////////////////////////////////////////////////////////
//     Function: GraphicsOutput::set_active
//       Access: Published
//  Description: Sets the active flag associated with the
//               GraphicsOutput.  If the GraphicsOutput is marked
//               inactive, nothing is rendered.
////////////////////////////////////////////////////////////////////
void GraphicsOutput::
set_active(bool active) {
  _active = active;
}

////////////////////////////////////////////////////////////////////
//     Function: GraphicsOutput::is_active
//       Access: Published, Virtual
//  Description: Returns true if the window is ready to be rendered
//               into, false otherwise.
////////////////////////////////////////////////////////////////////
bool GraphicsOutput::
is_active() const {
  return _active && is_valid();
}

////////////////////////////////////////////////////////////////////
//     Function: GraphicsOutput::set_sort
//       Access: Published
//  Description: Adjusts the sorting order of this particular
//               GraphicsOutput, relative to other GraphicsOutputs.
////////////////////////////////////////////////////////////////////
void GraphicsOutput::
set_sort(int sort) {
  if (_sort != sort) {
    if (_gsg != (GraphicsStateGuardian *)NULL &&
        _gsg->get_engine() != (GraphicsEngine *)NULL) {
      _gsg->get_engine()->set_window_sort(this, sort);
    }
  }
}

////////////////////////////////////////////////////////////////////
//     Function: GraphicsOutput::get_channel
//       Access: Public
//  Description: Returns a GraphicsChannel pointer that can be used to
//               access the indicated channel number.  All windows
//               have at least one channel, channel 0, which
//               corresponds to the entire window.  If the hardware
//               supports it, some kinds of windows may also have a
//               number of hardware channels available at indices
//               1..n, which will correspond to a subregion of the
//               window.
//
//               This function returns a GraphicsChannel pointer if a
//               channel is available, or NULL if it is not.  If
//               called twice with the same index number, it will
//               return the same pointer.
////////////////////////////////////////////////////////////////////
GraphicsChannel *GraphicsOutput::
get_channel(int index) {
  MutexHolder holder(_lock);
  nassertr(index >= 0, NULL);

  if (index < (int)_channels.size()) {
    if (_channels[index] != (GraphicsChannel *)NULL) {
      return _channels[index];
    }
  }

  // This channel has never been requested before; define it.

  PT(GraphicsChannel) chan;
  if (index == 0) {
    // Channel 0 is the default channel: the entire screen.
    chan = new GraphicsChannel(this);
  } else {
    // Any other channel is some hardware-specific channel.
    GraphicsPipe *pipe = _pipe;
    if (pipe != (GraphicsPipe *)NULL) {
      chan = _pipe->get_hw_channel(this, index);
      if (chan == (GraphicsChannel *)NULL) {
        display_cat.error()
          << "GraphicsOutput::get_channel() - got a NULL channel" << endl;
      } else {
        if (chan->get_window() != this) {
          chan = NULL;
        }
      }
    }
  }

  if (chan != (GraphicsChannel *)NULL) {
    declare_channel(index, chan);
  }

  return chan;
}

////////////////////////////////////////////////////////////////////
//     Function: GraphicsOutput::remove_channel
//       Access: Public
//  Description: Deletes a GraphicsChannel that was previously created
//               via a call to get_channel().  Note that the channel
//               is not actually deleted until all pointers to it are
//               cleared.
////////////////////////////////////////////////////////////////////
void GraphicsOutput::
remove_channel(int index) {
  MutexHolder holder(_lock);
  if (index >= 0 && index < (int)_channels.size()) {
    _channels[index].clear();
  }
}

////////////////////////////////////////////////////////////////////
//     Function: GraphicsOutput::get_max_channel_index
//       Access: Public
//  Description: Returns the largest channel index number yet created,
//               plus 1.  All channels associated with this window
//               will have an index number in the range [0,
//               get_max_channel_index()).  This function, in
//               conjunction with is_channel_defined(), below, may be
//               used to determine the complete set of channels
//               associated with the window.
////////////////////////////////////////////////////////////////////
int GraphicsOutput::
get_max_channel_index() const {
  int result;
  {
    MutexHolder holder(_lock);
    result = _channels.size();
  }
  return result;
}

////////////////////////////////////////////////////////////////////
//     Function: GraphicsOutput::is_channel_defined
//       Access: Public
//  Description: Returns true if the channel with the given index
//               number has already been defined, false if it hasn't.
//               If this returns true, calling get_channel() on the
//               given index number will return the channel pointer.
//               If it returns false, calling get_channel() will
//               create and return a new channel pointer.
////////////////////////////////////////////////////////////////////
bool GraphicsOutput::
is_channel_defined(int index) const {
  bool result;
  {
    MutexHolder holder(_lock);
    if (index < 0 || index >= (int)_channels.size()) {
      result = false;
    } else {
      result = (_channels[index] != (GraphicsChannel *)NULL);
    }
  }
  return result;
}

////////////////////////////////////////////////////////////////////
//     Function: GraphicsOutput::get_num_display_regions
//       Access: Published
//  Description: Returns the number of active DisplayRegions that have
//               been created within the various layers and channels
//               of the window.
////////////////////////////////////////////////////////////////////
int GraphicsOutput::
get_num_display_regions() const {
  determine_display_regions();
  int result;
  {
    MutexHolder holder(_lock);
    result = _display_regions.size();
  }
  return result;
}

////////////////////////////////////////////////////////////////////
//     Function: GraphicsOutput::get_display_region
//       Access: Published
//  Description: Returns the nth active DisplayRegion of those that
//               have been created within the various layers and
//               channels of the window.  This may return NULL if n is
//               out of bounds; particularly likely if the number of
//               display regions has changed since the last call to
//               get_num_display_regions().
////////////////////////////////////////////////////////////////////
DisplayRegion *GraphicsOutput::
get_display_region(int n) const {
  determine_display_regions();
  DisplayRegion *result;
  {
    MutexHolder holder(_lock);
    if (n >= 0 && n < (int)_display_regions.size()) {
      result = _display_regions[n];
    } else {
      result = NULL;
    }
  }
  return result;
}

////////////////////////////////////////////////////////////////////
//     Function: GraphicsOutput::make_texture_buffer
//       Access: Published
//  Description: Creates and returns an offscreen buffer for rendering
//               into, the result of which will be a texture suitable
//               for applying to geometry within the scene rendered
//               into this window.
//
//               This will attempt to be smart about maximizing render
//               performance while minimizing framebuffer waste.  It
//               might return a GraphicsBuffer set to render directly
//               into a texture, if possible; or it might return a
//               ParasiteBuffer that renders into this window.  The
//               return value is NULL if the buffer could not be
//               created for some reason.
//
//               Assuming the return value is not NULL, the texture
//               that is represents the scene rendered to the new
//               buffer can be accessed by buffer->get_texture().
//               When you are done using the buffer, you should remove
//               it with a call to GraphicsEngine::remove_window().
////////////////////////////////////////////////////////////////////
GraphicsOutput *GraphicsOutput::
make_texture_buffer(const string &name, int x_size, int y_size) {
  GraphicsStateGuardian *gsg = get_gsg();
  GraphicsEngine *engine = gsg->get_engine();
  GraphicsOutput *host = get_host();

  // The new buffer should be drawn before this buffer is drawn.  If
  // the user requires more control than this, he can set the sort
  // value himself.
  int sort = get_sort() - 1;

  if (show_buffers) {
    // If show_buffers is true, just go ahead and call make_buffer(),
    // since it all amounts to the same thing anyway--this will
    // actually create a new GraphicsWindow.
    return engine->make_buffer(gsg, name, sort, x_size, y_size, true);
  }

  GraphicsOutput *buffer = NULL;

  // If the user so indicated in the Configrc file, try to create a
  // parasite buffer first.  We can only do this if the requested size
  // fits within the available framebuffer size.
  if (prefer_parasite_buffer && 
      (x_size <= host->get_x_size() && y_size <= host->get_y_size())) {
    buffer = engine->make_parasite(host, name, sort, x_size, y_size);
    if (buffer != (GraphicsOutput *)NULL) {
      return buffer;
    }
  }

  // Attempt to create a single-buffered offscreen buffer.
  if (prefer_single_buffer) {
    FrameBufferProperties sb_props = gsg->get_properties();
    int orig_mode = sb_props.get_frame_buffer_mode();
    int sb_mode = (orig_mode & ~FrameBufferProperties::FM_buffer) | FrameBufferProperties::FM_single_buffer;
    sb_props.set_frame_buffer_mode(sb_mode);
    
    if (sb_mode != orig_mode) {
      PT(GraphicsStateGuardian) sb_gsg = 
        engine->make_gsg(gsg->get_pipe(), sb_props, gsg);
      if (sb_gsg != (GraphicsStateGuardian *)NULL) {
        buffer = engine->make_buffer(sb_gsg, name, sort, x_size, y_size, true);
        if (buffer != (GraphicsOutput *)NULL) {
          // Check the buffer for goodness.
          engine->open_windows();
          if (buffer->is_valid()) {
            return buffer;
          }

          // No good; delete the buffer and keep trying.
          bool removed = engine->remove_window(buffer);
          nassertr(removed, NULL);
          buffer = (GraphicsOutput *)NULL;
        }
      }
    }
  }

  // All right, attempt to create an offscreen buffer, using the same
  // GSG.  This will be a double-buffered offscreen buffer, if the
  // source window is double-buffered.
  buffer = engine->make_buffer(gsg, name, sort, x_size, y_size, true);
  if (buffer != (GraphicsOutput *)NULL) {
    engine->open_windows();
    if (buffer->is_valid()) {
      return buffer;
    }
    
    bool removed = engine->remove_window(buffer);
    nassertr(removed, NULL);
    buffer = (GraphicsOutput *)NULL;
  }

  // Looks like we have to settle for a parasite buffer.
  if (x_size <= host->get_x_size() && y_size <= host->get_y_size()) {
    return engine->make_parasite(host, name, sort, x_size, y_size);
  }

  return NULL;
}

////////////////////////////////////////////////////////////////////
//     Function: GraphicsOutput::make_scratch_display_region
//       Access: Public
//  Description: Allocates and returns a temporary DisplayRegion that
//               may be used to render offscreen into.  This
//               DisplayRegion is not associated with any layer.
//
//               To allocate a normal DisplayRegion for rendering, use
//               the interface provided in GraphicsLayer.
////////////////////////////////////////////////////////////////////
PT(DisplayRegion) GraphicsOutput::
make_scratch_display_region(int x_size, int y_size) {
#ifndef NDEBUG
  if (x_size > _x_size || y_size > _y_size) {
    display_cat.error()
      << "make_scratch_display_region(): requested region of size " 
      << x_size << ", " << y_size << " is larger than window of size "
      << _x_size << ", " << _y_size << ".\n";
    x_size = min(x_size, _x_size);
    y_size = min(y_size, _y_size);
  }
#endif

  PT(DisplayRegion) region = new DisplayRegion(this, x_size, y_size);
  region->copy_clear_settings(*this);
  return region;
}

////////////////////////////////////////////////////////////////////
//     Function: GraphicsOutput::get_host
//       Access: Public, Virtual
//  Description: This is normally called only from within
//               make_texture_buffer().  When called on a
//               ParasiteBuffer, it returns the host of that buffer;
//               but when called on some other buffer, it returns the
//               buffer itself.
////////////////////////////////////////////////////////////////////
GraphicsOutput *GraphicsOutput::
get_host() {
  return this;
}
 
////////////////////////////////////////////////////////////////////
//     Function: GraphicsOutput::request_open
//       Access: Public, Virtual
//  Description: This is called by the GraphicsEngine to request that
//               the window (or whatever) open itself or, in general,
//               make itself valid, at the next call to
//               process_events().
////////////////////////////////////////////////////////////////////
void GraphicsOutput::
request_open() {
}
 
////////////////////////////////////////////////////////////////////
//     Function: GraphicsOutput::request_close
//       Access: Public, Virtual
//  Description: This is called by the GraphicsEngine to request that
//               the window (or whatever) close itself or, in general,
//               make itself invalid, at the next call to
//               process_events().  By that time we promise the gsg
//               pointer will be cleared.
////////////////////////////////////////////////////////////////////
void GraphicsOutput::
request_close() {
}
 
////////////////////////////////////////////////////////////////////
//     Function: GraphicsOutput::set_close_now
//       Access: Public, Virtual
//  Description: This is called by the GraphicsEngine to insist that
//               the output be closed immediately.  This is only
//               called from the window thread.
////////////////////////////////////////////////////////////////////
void GraphicsOutput::
set_close_now() {
}

////////////////////////////////////////////////////////////////////
//     Function: GraphicsOutput::reset_window
//       Access: Protected, Virtual
//  Description: resets the window framebuffer from its derived
//               children. Does nothing here.
////////////////////////////////////////////////////////////////////
void GraphicsOutput::
reset_window(bool swapchain) {
  display_cat.info()
    << "Resetting " << get_type() << "\n";
}
 
////////////////////////////////////////////////////////////////////
//     Function: GraphicsOutput::begin_frame
//       Access: Public, Virtual
//  Description: This function will be called within the draw thread
//               before beginning rendering for a given frame.  It
//               should do whatever setup is required, and return true
//               if the frame should be rendered, or false if it
//               should be skipped.
////////////////////////////////////////////////////////////////////
bool GraphicsOutput::
begin_frame() {
  if (_gsg == (GraphicsStateGuardian *)NULL) {
    return false;
  }

  if (needs_context()) {
    if (!make_context()) {
      return false;
    }
  }

  // Okay, we already have a GSG, so activate it.
  make_current();
  return _gsg->begin_frame();
}

////////////////////////////////////////////////////////////////////
//     Function: GraphicsOutput::clear
//       Access: Public
//  Description: Clears the entire framebuffer before rendering,
//               according to the settings of get_color_clear_active()
//               and get_depth_clear_active() (inherited from
//               DrawableRegion).
//
//               This function is called only within the draw thread.
////////////////////////////////////////////////////////////////////
void GraphicsOutput::
clear() {
  if (is_any_clear_active()) {
    nassertv(_gsg != (GraphicsStateGuardian *)NULL);

    PT(DisplayRegion) win_dr =
      make_scratch_display_region(_x_size, _y_size);
    DisplayRegionStack old_dr = _gsg->push_display_region(win_dr);
    _gsg->clear(this);
    _gsg->pop_display_region(old_dr);
  }
}

////////////////////////////////////////////////////////////////////
//     Function: GraphicsOutput::end_frame
//       Access: Public, Virtual
//  Description: This function will be called within the draw thread
//               after rendering is completed for a given frame.  It
//               should do whatever finalization is required.
////////////////////////////////////////////////////////////////////
void GraphicsOutput::
end_frame() {
  nassertv(_gsg != (GraphicsStateGuardian *)NULL);
  _gsg->end_frame();

  // By default, we copy the framebuffer to the texture at the end of
  // the frame.  GraphicsBuffer objects that are set up to render
  // directly into texture memory don't need to do this; they will set
  // _copy_texture to false.
  if (_copy_texture) {
    PStatTimer timer(_copy_texture_pcollector);
    nassertv(has_texture());
    DisplayRegion dr(this, _x_size, _y_size);
    RenderBuffer buffer = _gsg->get_render_buffer(get_draw_buffer_type());
    _gsg->copy_texture(get_texture(), &dr, buffer);
  }

  // If we're not single-buffered, we're now ready to flip.
  if (!_gsg->get_properties().is_single_buffered()) {
    _flip_ready = true;
  }

  if (_one_shot && !show_buffers) {
    // In one-shot mode, we request the GraphicsEngine to delete the
    // window after we have rendered a frame.  But when show-buffers
    // mode is enabled, we don't do this, to give the user a chance to
    // see the output.
    _active = false;
    _delete_flag = true;
  }
}

////////////////////////////////////////////////////////////////////
//     Function: GraphicsOutput::make_context
//       Access: Public, Virtual
//  Description: If _needs_context is true, this will be called
//               in the draw thread prior to rendering into the
//               window.  It should attempt to create a graphics
//               context, and return true if successful, false
//               otherwise.  If it returns false the window will be
//               considered failed.
////////////////////////////////////////////////////////////////////
bool GraphicsOutput::
make_context() {
  _needs_context = false;
  return true;
}

////////////////////////////////////////////////////////////////////
//     Function: GraphicsOutput::make_current
//       Access: Public, Virtual
//  Description: This function will be called within the draw thread
//               during begin_frame() to ensure the graphics context
//               is ready for drawing.
////////////////////////////////////////////////////////////////////
void GraphicsOutput::
make_current() {
}

////////////////////////////////////////////////////////////////////
//     Function: GraphicsOutput::release_gsg
//       Access: Public
//  Description: Releases the current GSG pointer, if it is currently
//               held, and resets the GSG to NULL.  The window will be
//               permanently unable to render; this is normally called
//               only just before destroying the window.  This should
//               only be called from within the draw thread.
////////////////////////////////////////////////////////////////////
void GraphicsOutput::
release_gsg() {
  _gsg.clear();
}

////////////////////////////////////////////////////////////////////
//     Function: GraphicsOutput::begin_flip
//       Access: Public, Virtual
//  Description: This function will be called within the draw thread
//               after end_frame() has been called on all windows, to
//               initiate the exchange of the front and back buffers.
//
//               This should instruct the window to prepare for the
//               flip at the next video sync, but it should not wait.
//
//               We have the two separate functions, begin_flip() and
//               end_flip(), to make it easier to flip all of the
//               windows at the same time.
////////////////////////////////////////////////////////////////////
void GraphicsOutput::
begin_flip() {
}

////////////////////////////////////////////////////////////////////
//     Function: GraphicsOutput::end_flip
//       Access: Public, Virtual
//  Description: This function will be called within the draw thread
//               after begin_flip() has been called on all windows, to
//               finish the exchange of the front and back buffers.
//
//               This should cause the window to wait for the flip, if
//               necessary.
////////////////////////////////////////////////////////////////////
void GraphicsOutput::
end_flip() {
  _flip_ready = false;
}

////////////////////////////////////////////////////////////////////
//     Function: GraphicsOutput::process_events
//       Access: Public, Virtual
//  Description: Do whatever processing in the window thread is
//               appropriate for this output object each frame.
//
//               This function is called only within the window
//               thread.
////////////////////////////////////////////////////////////////////
void GraphicsOutput::
process_events() {
}

////////////////////////////////////////////////////////////////////
//     Function: GraphicsOutput::declare_channel
//       Access: Protected
//  Description: An internal function to add the indicated
//               newly-created channel to the list at the indicated
//               channel number.
//
//               The caller must grab and hold _lock before making
//               this call.
////////////////////////////////////////////////////////////////////
void GraphicsOutput::
declare_channel(int index, GraphicsChannel *chan) {
  nassertv(index >= 0);
  if (index >= (int)_channels.size()) {
    _channels.reserve(index);
    while (index >= (int)_channels.size()) {
      _channels.push_back(NULL);
    }
  }

  nassertv(index < (int)_channels.size());
  _channels[index] = chan;
}

////////////////////////////////////////////////////////////////////
//     Function: GraphicsOutput::do_determine_display_regions
//       Access: Private
//  Description: Recomputes the list of active DisplayRegions within
//               the window.
////////////////////////////////////////////////////////////////////
void GraphicsOutput::
do_determine_display_regions() {
  MutexHolder holder(_lock);
  _display_regions_stale = false;
  _display_regions.clear();
  Channels::const_iterator ci;
  for (ci = _channels.begin(); ci != _channels.end(); ++ci) {
    GraphicsChannel *chan = (*ci);
    if (chan->is_active()) {
      GraphicsChannel::GraphicsLayers::const_iterator li;
      for (li = chan->_layers.begin(); li != chan->_layers.end(); ++li) {
        GraphicsLayer *layer = (*li);
        if (layer->is_active()) {
          GraphicsLayer::DisplayRegions::const_iterator dri;
          for (dri = layer->_display_regions.begin(); 
               dri != layer->_display_regions.end(); 
               ++dri) {
            DisplayRegion *dr = (*dri);
            if (dr->is_active()) {
              _display_regions.push_back(dr);
            }
          }
        }
      }
    }
  }
}
