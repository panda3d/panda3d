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
#include "config_display.h"
#include "mutexHolder.h"
#include "hardwareChannel.h"
#include "renderBuffer.h"

TypeHandle GraphicsOutput::_type_handle;

#ifndef CPPPARSER
PStatCollector GraphicsOutput::_make_current_pcollector("Draw:Make current");
#endif  // CPPPARSER

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
  _sort = 0;

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
//     Function: GraphicsOutput::is_active
//       Access: Published, Virtual
//  Description: Returns true if the window is ready to be rendered
//               into, false otherwise.
////////////////////////////////////////////////////////////////////
bool GraphicsOutput::
is_active() const {
  return is_valid();
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
    nassertv(has_texture());
    DisplayRegion dr(this, _x_size, _y_size);
    RenderBuffer buffer = _gsg->get_render_buffer(get_draw_buffer_type());
    TextureContext *tc = get_texture()->prepare_now(_gsg->get_prepared_objects(), _gsg);
    _gsg->copy_texture(tc, &dr, buffer);
  }
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
