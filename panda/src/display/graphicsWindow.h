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

#include "graphicsOutput.h"
#include "graphicsWindowInputDevice.h"
#include "windowProperties.h"
#include "mouseData.h"
#include "modifierButtons.h"
#include "buttonEvent.h"
#include "notify.h"
#include "pmutex.h"
#include "filename.h"
#include "pvector.h"

////////////////////////////////////////////////////////////////////
//       Class : GraphicsWindow
// Description : A window, fullscreen or on a desktop, into which a
//               graphics device sends its output for interactive
//               display.
////////////////////////////////////////////////////////////////////
class EXPCL_PANDA GraphicsWindow : public GraphicsOutput {
protected:
  GraphicsWindow(GraphicsPipe *pipe, GraphicsStateGuardian *gsg);

PUBLISHED:
  virtual ~GraphicsWindow();

  WindowProperties get_properties() const;
  WindowProperties get_requested_properties() const;
  void clear_rejected_properties();
  WindowProperties get_rejected_properties() const;
  void request_properties(const WindowProperties &requested_properties);
  INLINE bool is_closed() const;
  virtual bool is_active() const;
  INLINE bool is_fullscreen() const;

  void set_window_event(const string &window_event);
  string get_window_event() const;

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

public:
  virtual void request_open();
  virtual void request_close();
  virtual void set_close_now();

  // It is an error to call any of the following methods from any
  // thread other than the window thread.  These methods are normally
  // called by the GraphicsEngine.
  virtual void process_events();
  virtual void set_properties_now(WindowProperties &properties);

protected:
  virtual bool do_reshape_request(int x_origin, int y_origin,
                                  int x_size, int y_size);

  // It is an error to call any of the following methods from any
  // thread other than the window thread.
  void system_changed_properties(const WindowProperties &properties);
  void system_changed_size(int x_size, int y_size);
  
protected:
  typedef vector_GraphicsWindowInputDevice InputDevices;
  InputDevices _input_devices;
  Mutex _input_lock;

protected:
  WindowProperties _properties;

private:
  Mutex _properties_lock; 
  // protects _requested_properties, _rejected_properties, and
  // _window_event.

  WindowProperties _requested_properties;
  WindowProperties _rejected_properties;
  string _window_event;
  
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
};

#include "graphicsWindow.I"

#endif /* GRAPHICSWINDOW_H */
