// Filename: graphicsWindow.cxx
// Created by:  mike (09Jan97)
//
////////////////////////////////////////////////////////////////////
//
////////////////////////////////////////////////////////////////////
// Includes
////////////////////////////////////////////////////////////////////
#include "graphicsWindow.h"
#include "graphicsPipe.h"
#include "config_display.h"

#include <mouseButton.h>
#include <keyboardButton.h>

#include <map>

////////////////////////////////////////////////////////////////////
// Static variables
////////////////////////////////////////////////////////////////////
TypeHandle GraphicsWindow::_type_handle;
TypeHandle GraphicsWindow::WindowProps::_type_handle;
TypeHandle GraphicsWindow::WindowPipe::_type_handle;

GraphicsWindow::WindowFactory *GraphicsWindow::_factory = NULL;

#ifndef CPPPARSER
PStatCollector GraphicsWindow::_app_pcollector("App");
PStatCollector GraphicsWindow::_show_code_pcollector("App:Show code");
PStatCollector GraphicsWindow::_swap_pcollector("Draw:Swap buffers");
PStatCollector GraphicsWindow::_clear_pcollector("Draw:Clear");
PStatCollector GraphicsWindow::_show_fps_pcollector("Draw:Show fps");
PStatCollector GraphicsWindow::_make_current_pcollector("Draw:Make current");
#endif

////////////////////////////////////////////////////////////////////
//     Function: GraphicsWindow::Properties::Constructor
//       Access: Public
//  Description:
////////////////////////////////////////////////////////////////////
GraphicsWindow::Properties::
Properties() {
  _xorg = 0;
  _yorg = 0;
  _xsize = 512;
  _ysize = 512;
  _title = "";
  _border = true;
  _fullscreen = false;
  _mask = W_RGBA | W_DOUBLE | W_DEPTH; 
  _want_depth_bits = 1;
  _want_color_bits = 1;
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
  MemoryUsage::update_type(this, this);
  _pipe = pipe;

  _draw_callback = NULL;
  _idle_callback = NULL;
  _resize_callback = NULL;
  _frame_number = 0;
}

////////////////////////////////////////////////////////////////////
//     Function: GraphicsWindow::Constructor
//       Access: Public
//  Description: 
////////////////////////////////////////////////////////////////////
GraphicsWindow::
GraphicsWindow(GraphicsPipe *pipe,
               const GraphicsWindow::Properties& props) : Configurable() {
  MemoryUsage::update_type(this, this);
  _pipe = pipe;
  _props = props;

  _draw_callback = NULL;
  _idle_callback = NULL;
  _resize_callback = NULL;
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
//       Access: Public
//  Description: 
////////////////////////////////////////////////////////////////////
GraphicsWindow::
~GraphicsWindow() {
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
//     Function: GraphicsWindow::flag_redisplay
//       Access: Public, Virtual
//  Description: 
////////////////////////////////////////////////////////////////////
void GraphicsWindow::
flag_redisplay() {
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
//     Function: GraphicsWindow::register_resize_function
//       Access: Public, Virtual
//  Description: 
////////////////////////////////////////////////////////////////////
void GraphicsWindow::
register_resize_function(GraphicsWindow::vfnii f) {
  _resize_function = f;
}

////////////////////////////////////////////////////////////////////
//     Function: GraphicsWindow::main_loop
//       Access: Public, Virtual
//  Description: Yields the application over to the window entirely
//               for rendering.  The window will wait for keyboard and
//               mouse input, and repeatedly call update() on itself.
//               For some kinds of window API's (notably glut), this
//               is the only way to use the window--see
//               supports_update().
//
//               This function does not return.
////////////////////////////////////////////////////////////////////
void GraphicsWindow::
main_loop() {
  while (true) {
    update();
  }
}

////////////////////////////////////////////////////////////////////
//     Function: GraphicsWindow::supports_update
//       Access: Public, Virtual
//  Description: Returns true if this particular kind of
//               GraphicsWindow supports use of the update() function
//               to update the graphics one frame at a time, so that
//               the window does not need to be the program's main
//               loop.  Returns false if the only way to update the
//               window is to call main_loop().
////////////////////////////////////////////////////////////////////
bool GraphicsWindow::
supports_update() const {
  return false;
}

////////////////////////////////////////////////////////////////////
//     Function: GraphicsWindow::update
//       Access: Public, Virtual
//  Description: 
////////////////////////////////////////////////////////////////////
void GraphicsWindow::
update() {
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
  _frame_number++;
}

////////////////////////////////////////////////////////////////////
//     Function: GraphicsWindow::resized
//       Access: Public, Virtual
//  Description: Called whenever the window gets the resize event.
////////////////////////////////////////////////////////////////////
void GraphicsWindow::
resized(const int x, const int y) {
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
//               the interface provded in GraphicsLayer.
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

  return new DisplayRegion(xsize, ysize); 
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
