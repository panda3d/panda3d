// Filename: graphicsWindow.h
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

#ifndef GRAPHICSWINDOW_H
#define GRAPHICSWINDOW_H

#include "pandabase.h"

#include "graphicsWindowInputDevice.h"
#include "windowProperties.h"
#include "graphicsChannel.h"
#include "graphicsPipe.h"
#include "displayRegion.h"
#include "graphicsStateGuardian.h"
#include "clearableRegion.h"

#include "typedReferenceCount.h"
#include "mouseData.h"
#include "modifierButtons.h"
#include "buttonEvent.h"
#include "iterator_types.h"
#include "notify.h"
#include "pmutex.h"

#include "pvector.h"
#include "pdeque.h"

////////////////////////////////////////////////////////////////////
//       Class : GraphicsWindow
// Description : An output medium for receiving the results of
//               rendering.  Typically this is a window on the
//               computer desktop, but it may also be the entire
//               desktop or console screen (i.e. a fullscreen window),
//               or a window on another machine, or even a disk file.
//
//               The GraphicsWindow class handles all of the details
//               about creating a window and its framebuffer, and
//               managing the properties associated with the windowing
//               system, such as position and size and keyboard/mouse
//               input.  The actual rendering, and anything associated
//               with the graphics context itself, is managed by the
//               window's GraphicsStateGuardian.
////////////////////////////////////////////////////////////////////
class EXPCL_PANDA GraphicsWindow : public TypedReferenceCount, public ClearableRegion {
protected:
  GraphicsWindow(GraphicsPipe *pipe);

private:
  GraphicsWindow(const GraphicsWindow &copy);
  void operator = (const GraphicsWindow &copy);

PUBLISHED:
  virtual ~GraphicsWindow();

  WindowProperties get_properties() const;
  WindowProperties get_requested_properties() const;
  void clear_rejected_properties();
  WindowProperties get_rejected_properties() const;
  void request_properties(const WindowProperties &requested_properties);
  INLINE bool is_closed() const;
  INLINE bool is_active() const;
  INLINE bool is_fullscreen() const;

  void set_window_event(const string &window_event);
  string get_window_event() const;

  INLINE GraphicsStateGuardian *get_gsg() const;
  INLINE GraphicsPipe *get_pipe() const;

  GraphicsChannel *get_channel(int index);
  void remove_channel(int index);

  int get_max_channel_index() const;
  bool is_channel_defined(int index) const;

  int get_num_display_regions() const;
  DisplayRegion *get_display_region(int n) const;

  // Mouse and keyboard routines
  int get_num_input_devices() const;
  string get_input_device_name(int device) const;
  bool has_pointer(int device) const;
  bool has_keyboard(int device) const;

public:
  // No need to publish these.
  MouseData get_mouse_data(int device) const;
  bool has_button_event(int device) const;
  ButtonEvent get_button_event(int device);

  virtual int verify_window_sizes(int numsizes, int *dimen);

  PT(DisplayRegion) make_scratch_display_region(int x_size, int y_size) const;

public:
  // These are not intended to be called directly by the user.
  INLINE void win_display_regions_changed();

public:
  // It is an error to call any of the following methods from any
  // thread other than the draw thread.  These methods are normally
  // called by the GraphicsEngine.
  virtual bool begin_frame();
  void clear();
  virtual void end_frame();

  virtual void make_gsg();
  virtual void release_gsg();

  // This method is called in the draw thread prior to issuing any
  // drawing commands for the window.
  virtual void make_current();

  // These methods will be called within the app (main) thread.
  virtual void begin_flip();
  virtual void end_flip();

  // It is an error to call any of the following methods from any
  // thread other than the window thread.  These methods are normally
  // called by the GraphicsEngine.
  virtual void process_events();
  virtual void set_properties_now(WindowProperties &properties);

protected:
  virtual void close_window();
  virtual bool open_window();
  virtual bool do_reshape_request(int x_origin, int y_origin,
                                  int x_size, int y_size);

  void declare_channel(int index, GraphicsChannel *chan);

  // It is an error to call any of the following methods from any
  // thread other than the window thread.
  void system_changed_properties(const WindowProperties &properties);
  void system_changed_size(int x_size, int y_size);
  
protected:
  typedef vector_GraphicsWindowInputDevice InputDevices;
  InputDevices _input_devices;
  Mutex _input_lock;

  PT(GraphicsStateGuardian) _gsg;
  PT(GraphicsPipe) _pipe;

private:
  INLINE void determine_display_regions() const;
  void do_determine_display_regions();

protected:
  WindowProperties _properties;

private:
  Mutex _lock; 
  // protects _channels, _display_regions, and _requested_properties.

  typedef pvector< PT(GraphicsChannel) > Channels;
  Channels _channels;

  typedef pvector<DisplayRegion *> DisplayRegions;
  DisplayRegions _display_regions;
  bool _display_regions_stale;

  WindowProperties _requested_properties;
  WindowProperties _rejected_properties;
  string _window_event;
  
public:
  static TypeHandle get_class_type() {
    return _type_handle;
  }
  static void init_type() {
    TypedReferenceCount::init_type();
    register_type(_type_handle, "GraphicsWindow",
                  TypedReferenceCount::get_class_type());
  }
  virtual TypeHandle get_type() const {
    return get_class_type();
  }
  virtual TypeHandle force_init_type() {init_type(); return get_class_type();}

private:
  static TypeHandle _type_handle;

  friend class GraphicsPipe;
  friend class GraphicsEngine;
};

#include "graphicsWindow.I"

#endif /* GRAPHICSWINDOW_H */
