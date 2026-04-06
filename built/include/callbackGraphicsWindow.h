/**
 * PANDA 3D SOFTWARE
 * Copyright (c) Carnegie Mellon University.  All rights reserved.
 *
 * All use of this software is subject to the terms of the revised BSD
 * license.  You should have received a copy of this license along
 * with this source code in a file named "LICENSE."
 *
 * @file callbackGraphicsWindow.h
 * @author drose
 * @date 2011-01-06
 */

#ifndef CALLBACKGRAPHICSWINDOW_H
#define CALLBACKGRAPHICSWINDOW_H

#include "pandabase.h"
#include "graphicsWindow.h"

/**
 * This special window object doesn't represent a window in its own right, but
 * instead hooks into some third-party API for creating and rendering to
 * windows via callbacks.  This can be used to allow Panda to render into an
 * already-created OpenGL context, for instance.
 */
class EXPCL_PANDA_DISPLAY CallbackGraphicsWindow : public GraphicsWindow {
protected:
  CallbackGraphicsWindow(GraphicsEngine *engine,
                         GraphicsPipe *pipe,
                         const std::string &name,
                         const FrameBufferProperties &fb_prop,
                         const WindowProperties &win_prop,
                         int flags,
                         GraphicsStateGuardian *gsg);

PUBLISHED:
  virtual ~CallbackGraphicsWindow();

  class EXPCL_PANDA_DISPLAY WindowCallbackData : public CallbackData {
  public:
    INLINE WindowCallbackData(CallbackGraphicsWindow *window);

  PUBLISHED:
    INLINE CallbackGraphicsWindow *get_window() const;
    MAKE_PROPERTY(window, get_window);

  protected:
    PT(CallbackGraphicsWindow) _window;

  public:
    static TypeHandle get_class_type() {
      return _type_handle;
    }
    static void init_type() {
      CallbackData::init_type();
      register_type(_type_handle, "CallbackGraphicsWindow::WindowCallbackData",
                    CallbackData::get_class_type());
    }
    virtual TypeHandle get_type() const {
      return get_class_type();
    }
    virtual TypeHandle force_init_type() {init_type(); return get_class_type();}

  private:
    static TypeHandle _type_handle;
  };

  class EXPCL_PANDA_DISPLAY EventsCallbackData : public WindowCallbackData {
  public:
    INLINE EventsCallbackData(CallbackGraphicsWindow *window);

  PUBLISHED:
    virtual void upcall();

  public:
    static TypeHandle get_class_type() {
      return _type_handle;
    }
    static void init_type() {
      WindowCallbackData::init_type();
      register_type(_type_handle, "CallbackGraphicsWindow::EventsCallbackData",
                    WindowCallbackData::get_class_type());
    }
    virtual TypeHandle get_type() const {
      return get_class_type();
    }
    virtual TypeHandle force_init_type() {init_type(); return get_class_type();}

  private:
    static TypeHandle _type_handle;
  };

  class EXPCL_PANDA_DISPLAY PropertiesCallbackData : public WindowCallbackData {
  public:
    INLINE PropertiesCallbackData(CallbackGraphicsWindow *window, WindowProperties &properties);

  PUBLISHED:
    INLINE WindowProperties &get_properties() const;

    virtual void upcall();

  private:
    WindowProperties &_properties;

  public:
    static TypeHandle get_class_type() {
      return _type_handle;
    }
    static void init_type() {
      WindowCallbackData::init_type();
      register_type(_type_handle, "CallbackGraphicsWindow::PropertiesCallbackData",
                    WindowCallbackData::get_class_type());
    }
    virtual TypeHandle get_type() const {
      return get_class_type();
    }
    virtual TypeHandle force_init_type() {init_type(); return get_class_type();}

  private:
    static TypeHandle _type_handle;
  };

  enum RenderCallbackType {
    RCT_begin_frame,
    RCT_end_frame,
    RCT_begin_flip,
    RCT_end_flip,
  };

  class EXPCL_PANDA_DISPLAY RenderCallbackData : public WindowCallbackData {
  public:
    INLINE RenderCallbackData(CallbackGraphicsWindow *window, RenderCallbackType callback_type, FrameMode frame_mode);

  PUBLISHED:
    INLINE CallbackGraphicsWindow::RenderCallbackType get_callback_type() const;
    INLINE GraphicsOutput::FrameMode get_frame_mode() const;
    MAKE_PROPERTY(callback_type, get_callback_type);
    MAKE_PROPERTY(frame_mode, get_frame_mode);

    INLINE void set_render_flag(bool render_flag);
    INLINE bool get_render_flag() const;
    MAKE_PROPERTY(render_flag, get_render_flag, set_render_flag);

    virtual void upcall();

  private:
    RenderCallbackType _callback_type;
    FrameMode _frame_mode;
    bool _render_flag;

  public:
    static TypeHandle get_class_type() {
      return _type_handle;
    }
    static void init_type() {
      WindowCallbackData::init_type();
      register_type(_type_handle, "CallbackGraphicsWindow::RenderCallbackData",
                    WindowCallbackData::get_class_type());
    }
    virtual TypeHandle get_type() const {
      return get_class_type();
    }
    virtual TypeHandle force_init_type() {init_type(); return get_class_type();}

  private:
    static TypeHandle _type_handle;
  };

  INLINE void set_events_callback(CallbackObject *object);
  INLINE void clear_events_callback();
  INLINE CallbackObject *get_events_callback() const;

  INLINE void set_properties_callback(CallbackObject *object);
  INLINE void clear_properties_callback();
  INLINE CallbackObject *get_properties_callback() const;

  INLINE void set_render_callback(CallbackObject *object);
  INLINE void clear_render_callback();
  INLINE CallbackObject *get_render_callback() const;

  int create_input_device(const std::string &name);

public:
  virtual bool begin_frame(FrameMode mode, Thread *current_thread);
  virtual void end_frame(FrameMode mode, Thread *current_thread);

  virtual void begin_flip();
  virtual void end_flip();

  virtual void process_events();
  virtual void set_properties_now(WindowProperties &properties);

protected:
  virtual bool open_window();
  virtual bool do_reshape_request(int x_origin, int y_origin, bool has_origin,
                                  int x_size, int y_size);

private:
  PT(CallbackObject) _events_callback;
  PT(CallbackObject) _properties_callback;
  PT(CallbackObject) _render_callback;

public:
  static TypeHandle get_class_type() {
    return _type_handle;
  }
  static void init_type() {
    GraphicsWindow::init_type();
    register_type(_type_handle, "CallbackGraphicsWindow",
                  GraphicsWindow::get_class_type());
    WindowCallbackData::init_type();
    EventsCallbackData::init_type();
    PropertiesCallbackData::init_type();
    RenderCallbackData::init_type();
  }
  virtual TypeHandle get_type() const {
    return get_class_type();
  }
  virtual TypeHandle force_init_type() {init_type(); return get_class_type();}

private:
  static TypeHandle _type_handle;

  friend class GraphicsEngine;
};

#include "callbackGraphicsWindow.I"

#endif
