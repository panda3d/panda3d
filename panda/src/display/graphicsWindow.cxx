// Filename: graphicsWindow.cxx
// Created by:  mike (09Jan97)
//
////////////////////////////////////////////////////////////////////
//
// PANDA 3D SOFTWARE
// Copyright (c) 2001, Disney Enterprises, Inc.  All rights reserved
//
// All use of this software is subject to the terms of the Panda 3d
// Software license.  You should have received a copy of this license
// along with this source code; you will also find a current copy of
// the license at http://www.panda3d.org/license.txt .
//
// To contact the maintainers of this program write to
// panda3d@yahoogroups.com .
//
////////////////////////////////////////////////////////////////////

#include "graphicsWindow.h"
#include "graphicsPipe.h"
#include "config_display.h"

#include <mouseButton.h>
#include <keyboardButton.h>

#include "pmap.h"

////////////////////////////////////////////////////////////////////
// Static variables
////////////////////////////////////////////////////////////////////
TypeHandle GraphicsWindow::_type_handle;
TypeHandle GraphicsWindow::WindowProps::_type_handle;
TypeHandle GraphicsWindow::WindowPipe::_type_handle;

GraphicsWindow::WindowFactory *GraphicsWindow::_factory = NULL;

#ifndef CPPPARSER
// We must compile these lines, even if DO_PSTATS is not defined,
// because the symbols for them are declared in the header file.
// Otherwise they will be undefined symbols at link time.  However,
// there's no runtime overhead to speak of for declaring these, so
// there's no harm in compiling them all the time.
PStatCollector GraphicsWindow::_app_pcollector("App");
PStatCollector GraphicsWindow::_show_code_pcollector("App:Show code");
PStatCollector GraphicsWindow::_swap_pcollector("Swap buffers");
PStatCollector GraphicsWindow::_clear_pcollector("Draw:Clear");
PStatCollector GraphicsWindow::_show_fps_pcollector("Draw:Show fps");
PStatCollector GraphicsWindow::_make_current_pcollector("Draw:Make current");
#endif

////////////////////////////////////////////////////////////////////
//     Function: GraphicsWindow::Properties::Constructor
//       Access: Published
//  Description:
////////////////////////////////////////////////////////////////////
GraphicsWindow::Properties::
Properties() {
  _xorg = 0;
  _yorg = 0;
  _xsize = 512;
  _ysize = 512;
  _title = "Panda";
  _border = true;
  _fullscreen = false;
  _mask = W_RGBA | W_DOUBLE | W_DEPTH;
  _want_depth_bits = 1;
  _want_color_bits = 1;
  _bCursorIsVisible=true;
}

////////////////////////////////////////////////////////////////////
//     Function: GraphicsWindow::Callback::draw
//       Access: Public
//  Description:
////////////////////////////////////////////////////////////////////
void GraphicsWindow::Callback::
draw(bool) {
  display_cat.error()
    << "Callback::draw() - no class defined for this" << endl;
}

////////////////////////////////////////////////////////////////////
//     Function: GraphicsWindow::Callback::idle
//       Access: Public
//  Description:
////////////////////////////////////////////////////////////////////
void GraphicsWindow::Callback::
idle() {
  display_cat.error()
    << "Callback::idle() - no class defined for this" << endl;
}

GraphicsWindow::WindowProps::~WindowProps(void) {}

TypeHandle GraphicsWindow::WindowProps::get_class_type(void) {
  return _type_handle;
}

void GraphicsWindow::WindowProps::init_type(void) {
  WindowParam::init_type();
  register_type(_type_handle, "GraphicsWindow::WindowProps",
                WindowParam::get_class_type());
}

TypeHandle GraphicsWindow::WindowProps::get_type(void) const {
  return get_class_type();
}

TypeHandle GraphicsWindow::WindowProps::force_init_type(void) {
  init_type();
  return get_class_type();
}

GraphicsWindow::WindowPipe::~WindowPipe(void) {}

TypeHandle GraphicsWindow::WindowPipe::get_class_type(void) {
  return _type_handle;
}

void GraphicsWindow::WindowPipe::init_type(void) {
  WindowParam::init_type();
  register_type(_type_handle, "GraphicsWindow::WindowPipe",
                WindowParam::get_class_type());
}

TypeHandle GraphicsWindow::WindowPipe::get_type(void) const {
  return get_class_type();
}

TypeHandle GraphicsWindow::WindowPipe::force_init_type(void) {
  init_type();
  return get_class_type();
}

////////////////////////////////////////////////////////////////////
//     Function: GraphicsWindow::Constructor
//       Access: Public
//  Description:
////////////////////////////////////////////////////////////////////
GraphicsWindow::
GraphicsWindow(GraphicsPipe *pipe) : Configurable() {
#ifdef DO_MEMORY_USAGE
  MemoryUsage::update_type(this, this);
#endif
  _pipe = pipe;

  _draw_callback = NULL;
  _idle_callback = NULL;
  _frame_number = 0;
  _is_synced = false;
  _window_active = true;
  _display_regions_stale = false;

  // By default, windows are set up to clear color and depth.
  set_clear_color_active(true);
  set_clear_depth_active(true);
}

////////////////////////////////////////////////////////////////////
//     Function: GraphicsWindow::Constructor
//       Access: Public
//  Description:
////////////////////////////////////////////////////////////////////
GraphicsWindow::
GraphicsWindow(GraphicsPipe *pipe,
               const GraphicsWindow::Properties &props) : Configurable() {
#ifdef DO_MEMORY_USAGE
  MemoryUsage::update_type(this, this);
#endif
  _pipe = pipe;
  _props = props;

  _draw_callback = NULL;
  _idle_callback = NULL;
  _frame_number = 0;
  _is_synced = false;
  _window_active = true;
  _display_regions_stale = false;

  // By default, windows are set up to clear color and depth.
  set_clear_color_active(true);
  set_clear_depth_active(true);
}

////////////////////////////////////////////////////////////////////
//     Function: GraphicsWindow::Copy Constructor
//       Access: Public
//  Description:
////////////////////////////////////////////////////////////////////
GraphicsWindow::
GraphicsWindow(const GraphicsWindow&) {
  display_cat.error()
    << "GraphicsWindows should not be copied" << endl;
}

////////////////////////////////////////////////////////////////////
//     Function: GraphicsWindow::Copy Assignment Operator
//       Access: Public
//  Description:
////////////////////////////////////////////////////////////////////
GraphicsWindow& GraphicsWindow::
operator=(const GraphicsWindow&) {
  display_cat.error()
  << "GraphicsWindows should not be assigned" << endl;
  return *this;
}

////////////////////////////////////////////////////////////////////
//     Function: GraphicsWindow::Destructor
//       Access: Public, Virtual
//  Description:
////////////////////////////////////////////////////////////////////
GraphicsWindow::
~GraphicsWindow() {
  // First, call close_window().  This tells our GSG to let go of its
  // pointer to us, and also eventually calls do_close_window().
  // However, do_close_window() is a virtual function that might be
  // extended in a derived class, but we don't have any derived
  // virtual functions by the time the destructor is called.
  
  // Therefore, if a derived class has redefined do_close_window(), it
  // should also call close_window() in its own destructor.
  close_window();


  // We don't have to destruct our child channels explicitly, since
  // they are all reference-counted and will go away when their
  // pointers do.  However, we do need to zero out their pointers to
  // us.
  Channels::iterator ci;
  for (ci = _channels.begin(); ci != _channels.end(); ++ci) {
    (*ci)->_window = NULL;
  }

  // We don't need to remove ourself from the pipe's list of windows.
  // We must have already been removed, or we wouldn't be destructing!
}

////////////////////////////////////////////////////////////////////
//     Function: GraphicsWindow::get_channel
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
GraphicsChannel *GraphicsWindow::
get_channel(int index) {
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
    nassertr(_pipe != NULL, NULL);
    chan = _pipe->get_hw_channel(this, index);
    if (chan == NULL) {
      display_cat.error()
        << "GraphicsWindow::get_channel() - got a NULL channel" << endl;
    } else {
      if (chan->get_window() != this) {
        chan = NULL;
      }
    }
  }

  if (chan != (GraphicsChannel *)NULL) {
    declare_channel(index, chan);
  }

  return chan;
}

////////////////////////////////////////////////////////////////////
//     Function: GraphicsWindow::remove_channel
//       Access: Public
//  Description: Deletes a GraphicsChannel that was previously created
//               via a call to get_channel().  Note that the channel
//               is not actually deleted until all pointers to it are
//               cleared.
////////////////////////////////////////////////////////////////////
void GraphicsWindow::
remove_channel(int index) {
  if (index >= 0 && index < (int)_channels.size()) {
    _channels[index].clear();
  }
}

////////////////////////////////////////////////////////////////////
//     Function: GraphicsWindow::get_max_channel_index
//       Access: Public
//  Description: Returns the largest channel index number yet created,
//               plus 1.  All channels associated with this window
//               will have an index number in the range [0,
//               get_max_channel_index()).  This function, in
//               conjunction with is_channel_defined(), below, may be
//               used to determine the complete set of channels
//               associated with the window.
////////////////////////////////////////////////////////////////////
int GraphicsWindow::
get_max_channel_index() const {
  return _channels.size();
}

////////////////////////////////////////////////////////////////////
//     Function: GraphicsWindow::is_channel_defined
//       Access: Public
//  Description: Returns true if the channel with the given index
//               number has already been defined, false if it hasn't.
//               If this returns true, calling get_channel() on the
//               given index number will return the channel pointer.
//               If it returns false, calling get_channel() will
//               create and return a new channel pointer.
////////////////////////////////////////////////////////////////////
bool GraphicsWindow::
is_channel_defined(int index) const {
  if (index < 0 || index >= (int)_channels.size()) {
    return false;
  }
  return (_channels[index] != (GraphicsChannel *)NULL);
}

////////////////////////////////////////////////////////////////////
//     Function: GraphicsWindow::declare_channel
//       Access: Protected
//  Description: An internal function to add the indicated
//               newly-created channel to the list at the indicated
//               channel number.
////////////////////////////////////////////////////////////////////
void GraphicsWindow::
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
//     Function: GraphicsWindow::do_determine_display_regions
//       Access: Private
//  Description: Recomputes the list of active DisplayRegions within
//               the window.
////////////////////////////////////////////////////////////////////
void GraphicsWindow::
do_determine_display_regions() {
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

  _display_regions_stale = false;
}

////////////////////////////////////////////////////////////////////
//     Function: GraphicsWindow::resized
//       Access: Public, Virtual
//  Description: Called whenever the window gets the resize event.
////////////////////////////////////////////////////////////////////
void GraphicsWindow::
resized(const unsigned int x, const unsigned int y) {
  Channels::iterator ci;
  for (ci = _channels.begin(); ci != _channels.end(); ++ci) {
    GraphicsChannel *chan = (*ci);
    chan->window_resized(x, y);
  }
  _props._xsize = x;
  _props._ysize = y;
}

////////////////////////////////////////////////////////////////////
//     Function: GraphicsWindow::make_scratch_display_region
//       Access: Public
//  Description: Allocates and returns a temporary DisplayRegion that
//               may be used to render offscreen into.  This
//               DisplayRegion is not associated with any layer.
//
//               To allocate a normal DisplayRegion for rendering, use
//               the interface provided in GraphicsLayer.
////////////////////////////////////////////////////////////////////
PT(DisplayRegion) GraphicsWindow::
make_scratch_display_region(int xsize, int ysize) const {
  if (xsize > _props._xsize) {
    display_cat.error()
      << "GraphicsWindow::make_scratch_display_region() - x size is larger "
      << "than window x size" << endl;
    xsize = _props._xsize;
  }
  if (ysize > _props._ysize) {
    display_cat.error()
      << "GraphicsWindow::make_scratch_display_region() - y size is larger "
      << "than window y size" << endl;
    ysize = _props._ysize;
  }

  PT(DisplayRegion) region = new DisplayRegion(xsize, ysize);
  region->copy_clear_settings(*this);
  return region;
}

////////////////////////////////////////////////////////////////////
//     Function: GraphicsWindow::make_current
//       Access: Public, Virtual
//  Description: Makes the window's graphics context the currently
//               active context that will be next rendered into by the
//               GSG, if this makes sense for the particular type of
//               GraphicsWindow.
////////////////////////////////////////////////////////////////////
void GraphicsWindow::
make_current(void) {
}

////////////////////////////////////////////////////////////////////
//     Function: GraphicsWindow::unmake_current
//       Access: Public, Virtual
//  Description:
////////////////////////////////////////////////////////////////////
void GraphicsWindow::
unmake_current(void) {
}

////////////////////////////////////////////////////////////////////
//     Function: GraphicsWindow::flag_redisplay
//       Access: Public, Virtual
//  Description:
////////////////////////////////////////////////////////////////////
void GraphicsWindow::
flag_redisplay() {
}

////////////////////////////////////////////////////////////////////
//     Function: GraphicsWindow::register_draw_function
//       Access: Public, Virtual
//  Description:
////////////////////////////////////////////////////////////////////
void GraphicsWindow::
register_draw_function(GraphicsWindow::vfn f) {
  _draw_function = f;
}

////////////////////////////////////////////////////////////////////
//     Function: GraphicsWindow::register_idle_function
//       Access: Public, Virtual
//  Description:
////////////////////////////////////////////////////////////////////
void GraphicsWindow::
register_idle_function(GraphicsWindow::vfn f) {
  _idle_function = f;
}

////////////////////////////////////////////////////////////////////
//     Function: GraphicsWindow::begin_frame
//       Access: Public, Virtual
//  Description: This function will be called by the GSG before
//               beginning processing for a given frame.  It should do
//               whatever setup is required.
////////////////////////////////////////////////////////////////////
void GraphicsWindow::
begin_frame() {
  _gsg->begin_frame();
}

////////////////////////////////////////////////////////////////////
//     Function: GraphicsWindow::clear
//       Access: Public
//  Description: Clears the entire framebuffer before rendering,
//               according to the settings of get_color_clear_active()
//               and get_depth_clear_active() (inherited from
//               ClearableRegion).
////////////////////////////////////////////////////////////////////
void GraphicsWindow::
clear() {
  if (is_any_clear_active()) {
    nassertv(_gsg != (GraphicsStateGuardian *)NULL);

    PT(DisplayRegion) win_dr =
      make_scratch_display_region(get_width(), get_height());
    DisplayRegionStack old_dr = _gsg->push_display_region(win_dr);
    _gsg->clear(this);
    _gsg->pop_display_region(old_dr);
  }
}

////////////////////////////////////////////////////////////////////
//     Function: GraphicsWindow::end_frame
//       Access: Public, Virtual
//  Description: This function will be called by the GSG after
//               processing is completed for a given frame.  It should
//               do whatever finalization is required.
////////////////////////////////////////////////////////////////////
void GraphicsWindow::
end_frame() {
  _gsg->end_frame();
  _frame_number++;
}

////////////////////////////////////////////////////////////////////
//     Function: GraphicsWindow::process_events
//       Access: Public, Virtual
//  Description: Do whatever processing is necessary to ensure that
//               the window responds to user events.
////////////////////////////////////////////////////////////////////
void GraphicsWindow::
process_events() {
}

////////////////////////////////////////////////////////////////////
//     Function: GraphicsWindow::deactivate_window
//       Access: Public, Virtual
//  Description: Indicates the window should stop rendering
//               temporarily, and does whatever else is associated
//               with that.  This is normally called only by the
//               GraphicsWindow itself, or by the GSG.
////////////////////////////////////////////////////////////////////
void GraphicsWindow::
deactivate_window() {
  _window_active = false;
}

////////////////////////////////////////////////////////////////////
//     Function: GraphicsWindow::reactivate_window
//       Access: Public, Virtual
//  Description: Restores the normal window rendering behavior after a
//               previous call to deactivate_window().
////////////////////////////////////////////////////////////////////
void GraphicsWindow::
reactivate_window() {
  _window_active = true;
}

////////////////////////////////////////////////////////////////////
//     Function: GraphicsWindow::make_gsg
//       Access: Protected
//  Description: Creates a new GSG for the window and stores it in the
//               _gsg pointer.
////////////////////////////////////////////////////////////////////
void GraphicsWindow::
make_gsg() {
  FactoryParams params;
  params.add_param(new GraphicsStateGuardian::GsgWindow(this));

  _gsg = GraphicsStateGuardian::get_factory().
    make_instance(get_gsg_type(), params);

  nassertv(_gsg != (GraphicsStateGuardian *)NULL);
}

////////////////////////////////////////////////////////////////////
//     Function: GraphicsWindow::release_gsg
//       Access: Protected
//  Description: Releases the current GSG pointer, if it is currently
//               held.  This invalidates the window and marks it
//               closed; it should not be called unless the code to
//               close the window is also called.
////////////////////////////////////////////////////////////////////
void GraphicsWindow::
release_gsg() {
  if (_gsg != (GraphicsStateGuardian *)NULL) {
    // First, we save the GSG pointer and then NULL it out.  That way,
    // if the GSG happens to call close_window() while it is closing
    // itself, it won't be recursive (because we'll already be marked
    // closed).
    PT(GraphicsStateGuardian) gsg = _gsg;
    _gsg.clear();

    // Now we tell the GSG it's time to sleep.
    gsg->close_gsg();
  }
}

////////////////////////////////////////////////////////////////////
//     Function: GraphicsWindow::do_close_window
//       Access: Protected, Virtual
//  Description: An internal function to release whatever system
//               resources are held by the window and actually close
//               it.  This is called by close_window().  
//
//               If a derived class redefines this function, it should
//               also arrange to call close_window() (or its
//               equivalent) from its own destructor, since we cannot
//               call a virtual function from this destructor.
////////////////////////////////////////////////////////////////////
void GraphicsWindow::
do_close_window() {
  display_cat.info()
    << "Closing " << get_type() << "\n";
}

////////////////////////////////////////////////////////////////////
//     Function: GraphicsWindow::get_factory
//       Access: Public, Static
//  Description: Returns the factory object that can be used to
//               register new kinds of GraphicsWindow objects that may
//               be created.
////////////////////////////////////////////////////////////////////
GraphicsWindow::WindowFactory &GraphicsWindow::
get_factory() {
  if (_factory == (WindowFactory *)NULL) {
    _factory = new WindowFactory;
  }
  return (*_factory);
}

void GraphicsWindow::read_priorities(void) {
  WindowFactory &factory = get_factory();
  if (factory.get_num_preferred() == 0) {
    Config::ConfigTable::Symbol::iterator i;
    for (i = preferred_window_begin(); i != preferred_window_end(); ++i) {
      ConfigString type_name = (*i).Val();
      TypeHandle type = TypeRegistry::ptr()->find_type(type_name);
      if (type == TypeHandle::none()) {
        display_cat.warning()
          << "Unknown type requested for window preference: " << type_name
          << "\n";
      } else {
        display_cat.debug()
          << "Specifying type " << type << " for window preference.\n";
        factory.add_preferred(type);
      }
    }
  }
}

////////////////////////////////////////////////////////////////////
//     Function: GraphicsWindow::swap
//       Access: Public
//  Description: Swaps buffers explicitely as synchronization 
//               mechanism. 
////////////////////////////////////////////////////////////////////
void GraphicsWindow::
swap() {
  display_cat.warning() << "swap() unimplemented by " << get_type() << endl;
}

////////////////////////////////////////////////////////////////////
//     Function: GraphicsWindow::resize
//       Access: Public
//  Description: Resizes the window to the given size.
//               Should try to preserve current window bitdepths,
//               if possible.  If it is not possible to resize window to
//               the given size, return false and maintain current
//               window size.
////////////////////////////////////////////////////////////////////
bool GraphicsWindow::
resize(unsigned int xsize,unsigned int ysize) {
  display_cat.warning() << "resize() unimplemented by " << get_type() << endl;  
  return false;
}

unsigned int GraphicsWindow::
verify_window_sizes(unsigned int numsizes,unsigned int *dimen) {
  // see if window sizes are supported (i.e. in fullscrn mode)
  // dimen is an array containing contiguous x,y pairs specifying
  // possible display sizes, it is numsizes*2 long.  fn will zero
  // out any invalid x,y size pairs.  return value is number of valid 
  // sizes that were found.
  // 
  // note: it might be better to implement some sort of query
  //       interface that returns an array of supported sizes,
  //       but this way is somewhat simpler and will do the job 
  //       on most cards, assuming they handle the std sizes the app
  //       knows about.

  // Also note this doesnt guarantee resize() will work, you still need to check its return value.

  display_cat.warning() << "verify_window_sizes() unimplemented by " << get_type() << endl; 
  return numsizes;
}

int GraphicsWindow::
get_depth_bitwidth(void) {
    display_cat.warning() << "get_depth_bitwidth() unimplemented by " << get_type() << endl; 
    return -1;
}
