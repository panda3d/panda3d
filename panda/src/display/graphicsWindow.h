// Filename: graphicsWindow.h
// Created by:  mike (09Jan97)
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

#ifndef GRAPHICSWINDOW_H
#define GRAPHICSWINDOW_H

#include "pandabase.h"

#include "graphicsOutput.h"
#include "graphicsWindowInputDevice.h"
#include "windowProperties.h"
#include "mouseData.h"
#include "modifierButtons.h"
#include "buttonEvent.h"
#include "pnotify.h"
#include "lightMutex.h"
#include "lightReMutex.h"
#include "pvector.h"

////////////////////////////////////////////////////////////////////
//       Class : GraphicsWindow
// Description : A window, fullscreen or on a desktop, into which a
//               graphics device sends its output for interactive
//               display.
////////////////////////////////////////////////////////////////////
class EXPCL_PANDA_DISPLAY GraphicsWindow : public GraphicsOutput {
protected:
  GraphicsWindow(GraphicsEngine *engine,
                 GraphicsPipe *pipe, 
                 const string &name,
                 const FrameBufferProperties &fb_prop,
                 const WindowProperties &win_prop,
                 int flags,
                 GraphicsStateGuardian *gsg,
                 GraphicsOutput *host);

PUBLISHED:
  virtual ~GraphicsWindow();

  const WindowProperties get_properties() const;
  const WindowProperties get_requested_properties() const;
  void clear_rejected_properties();
  WindowProperties get_rejected_properties() const;
  void request_properties(const WindowProperties &requested_properties);
  INLINE bool is_closed() const;
  virtual bool is_active() const;
  INLINE bool is_fullscreen() const;

  void set_window_event(const string &window_event);
  string get_window_event() const;

  void set_close_request_event(const string &close_request_event);
  string get_close_request_event() const;

  // Mouse and keyboard routines
  int get_num_input_devices() const;
  string get_input_device_name(int device) const;
  MAKE_SEQ(get_input_device_names, get_num_input_devices, get_input_device_name);
  bool has_pointer(int device) const;
  bool has_keyboard(int device) const;
  

  void enable_pointer_events(int device);
  void disable_pointer_events(int device);
  void enable_pointer_mode(int device, double speed);
  void disable_pointer_mode(int device);

  MouseData get_pointer(int device) const;
  virtual bool move_pointer(int device, int x, int y);
  virtual void close_ime();

public:
  // No need to publish these.
  bool has_button_event(int device) const;
  ButtonEvent get_button_event(int device);
  bool has_pointer_event(int device) const;
  PT(PointerEventList) get_pointer_events(int device);

  virtual int verify_window_sizes(int numsizes, int *dimen);

public:
  virtual void request_open();
  virtual void request_close();

  // It is an error to call any of the following methods from any
  // thread other than the window thread.  These methods are normally
  // called by the GraphicsEngine.
  virtual void set_close_now();
  virtual void process_events();
  virtual void set_properties_now(WindowProperties &properties);

protected:
  virtual void close_window();
  virtual bool open_window();
  virtual void reset_window(bool swapchain);

  virtual bool do_reshape_request(int x_origin, int y_origin, bool has_origin,
                                  int x_size, int y_size);

  virtual void mouse_mode_absolute();
  virtual void mouse_mode_relative();

  // It is an error to call any of the following methods from any
  // thread other than the window thread.
  void system_changed_properties(const WindowProperties &properties);
  void system_changed_size(int x_size, int y_size);

private:
  static unsigned int parse_color_mask(const string &word);

protected:
  INLINE void add_input_device(const GraphicsWindowInputDevice &device);
  typedef vector_GraphicsWindowInputDevice InputDevices;
  InputDevices _input_devices;
  LightMutex _input_lock;

protected:
  WindowProperties _properties;

private:
  LightReMutex _properties_lock; 
  // protects _requested_properties, _rejected_properties, and
  // _window_event.

  WindowProperties _requested_properties;
  WindowProperties _rejected_properties;
  string _window_event;
  string _close_request_event;
  
public:
  static TypeHandle get_class_type() {
    return _type_handle;
  }
  static void init_type() {
    GraphicsOutput::init_type();
    register_type(_type_handle, "GraphicsWindow",
                  GraphicsOutput::get_class_type());
  }
  virtual TypeHandle get_type() const {
    return get_class_type();
  }
  virtual TypeHandle force_init_type() {init_type(); return get_class_type();}

private:
  static TypeHandle _type_handle;

  friend class GraphicsEngine;
};

#include "graphicsWindow.I"

#endif /* GRAPHICSWINDOW_H */
