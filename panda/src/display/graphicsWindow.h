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
//
////////////////////////////////////////////////////////////////////
// Includes
////////////////////////////////////////////////////////////////////
#include <pandabase.h>

#include "graphicsWindowInputDevice.h"
#include "graphicsChannel.h"
#include "displayRegion.h"
#include "graphicsStateGuardian.h"

#include <typedef.h>
#include <configurable.h>
#include <referenceCount.h>
#include <mouseData.h>
#include <modifierButtons.h>
#include <buttonEvent.h>
#include <iterator_types.h>
#include <factory.h>
#include <pStatCollector.h>

#include <string>
#include "pvector.h"
#include "pdeque.h"

////////////////////////////////////////////////////////////////////
// Defines
////////////////////////////////////////////////////////////////////
enum WindowModeType
{
    W_RGBA =                            0,
    W_RGB =                             0,
    W_INDEX =                           1,
    W_SINGLE =                          0,
    W_DOUBLE =                          2,
    W_ACCUM =                           4,
    W_ALPHA =                           8,
    W_DEPTH =                           16,
    W_STENCIL =                         32,
    W_MULTISAMPLE =                     128,
    W_STEREO =                          256,
    W_LUMINANCE =                       512
};

class GraphicsPipe;
class GraphicsWindow;

////////////////////////////////////////////////////////////////////
//       Class : GraphicsWindow
// Description :
////////////////////////////////////////////////////////////////////
class EXPCL_PANDA GraphicsWindow : public Configurable, public ReferenceCount {
public:
  class EXPCL_PANDA Properties {
  public:
    Properties();
  public:
    int _xorg;
    int _yorg;
    int _xsize;
    int _ysize;
    string _title;
    bool _border;
    bool _fullscreen;
    uint _mask;
    int _want_depth_bits;
    int _want_color_bits;
  };

  class EXPCL_PANDA Callback {
  public:
    virtual void draw(bool);
    virtual void idle();
  };
  typedef void (*vfn)();
  typedef void (*vfnii)(int, int);

public:

  GraphicsWindow(GraphicsPipe*);
#ifdef WIN32_VC
  GraphicsWindow(GraphicsPipe*, const Properties&);
#else
  GraphicsWindow(GraphicsPipe*, const GraphicsWindow::Properties&);
#endif
  virtual ~GraphicsWindow();

  INLINE const GraphicsWindow::Properties& get_properties() const;

PUBLISHED:
  INLINE int get_width() const;
  INLINE int get_height() const;
  INLINE int get_xorg() const;
  INLINE int get_yorg() const;

  INLINE GraphicsStateGuardian *get_gsg() const;

  INLINE GraphicsPipe *get_pipe() const;

  INLINE void set_frame_number(const int);
  INLINE int get_frame_number() const;

  virtual void close_window(int exit_status) {return;};  //release windowing system resources

public:
  virtual void resized(const int, const int);

  INLINE virtual void set_draw_callback(Callback *c);
  INLINE virtual void set_idle_callback(Callback *c);

  INLINE void call_draw_callback(bool force_redraw);
  INLINE void call_idle_callback();

  PT(DisplayRegion) make_scratch_display_region(int xsize,
                                                int ysize) const;

  virtual TypeHandle get_gsg_type() const=0;

public:
  // context setting
  virtual void make_current();
  virtual void unmake_current();

PUBLISHED:
  // Mouse and keyboard routines
  INLINE int get_num_input_devices() const;
  INLINE string get_input_device_name(int device) const;
  INLINE bool has_pointer(int device) const;
  INLINE bool has_keyboard(int device) const;

  virtual void process_events(void) { return; };   // process window events

public:
  INLINE const MouseData &get_mouse_data(int device) const;
  INLINE bool has_button_event(int device) const;
  INLINE ButtonEvent get_button_event(int device);

PUBLISHED:
  // GUI glue methods
  virtual void flag_redisplay();
  virtual void register_draw_function(GraphicsWindow::vfn);
  virtual void register_idle_function(GraphicsWindow::vfn);
  virtual void register_resize_function(GraphicsWindow::vfnii);

  virtual void main_loop();
  virtual bool supports_update() const;
  virtual void update();

public:
  virtual void begin_frame();
  virtual void end_frame();

  // Statistics
  static PStatCollector _app_pcollector;
  static PStatCollector _show_code_pcollector;
  static PStatCollector _swap_pcollector;  // dxgsg needs access so this is public
  static PStatCollector _clear_pcollector;
  static PStatCollector _show_fps_pcollector;
  static PStatCollector _make_current_pcollector;

protected:
  void make_gsg();

  typedef vector_GraphicsWindowInputDevice InputDevices;
  InputDevices _input_devices;

  PT(GraphicsStateGuardian) _gsg;
  Properties _props;

  GraphicsPipe *_pipe;
  vfn _draw_function;
  vfn _idle_function;
  vfnii _resize_function;
  int _frame_number;

protected:

  Callback *_draw_callback;
  Callback *_idle_callback;
  Callback *_resize_callback;

public:
  virtual GraphicsChannel *get_channel(int index);
  void remove_channel(int index);

  int get_max_channel_index() const;
  bool is_channel_defined(int index) const;

protected:
  void declare_channel(int index, GraphicsChannel *chan);

private:
  typedef pvector< PT(GraphicsChannel) > Channels;
  Channels _channels;

public:

  // factory stuff
  typedef Factory<GraphicsWindow> WindowFactory;
  typedef FactoryParam WindowParam;

  // make a factory parameter type for the window properties
  class EXPCL_PANDA WindowProps : public FactoryParam {
  public:
    INLINE WindowProps(void) : WindowParam() {}
    INLINE WindowProps(const Properties& p) : WindowParam(), _p(p) {}
    virtual ~WindowProps(void);
    INLINE Properties get_properties(void) { return _p; }
  public:
    static TypeHandle get_class_type(void);
    static void init_type(void);
    virtual TypeHandle get_type(void) const;
    virtual TypeHandle force_init_type(void);
  private:
    Properties _p;
    static TypeHandle _type_handle;
  };
  // make a factory parameter type for the GraphicsPipe*
  class EXPCL_PANDA WindowPipe : public FactoryParam {
  public:
    INLINE WindowPipe(GraphicsPipe* p) : WindowParam(), _p(p) {}
    virtual ~WindowPipe(void);
    INLINE GraphicsPipe* get_pipe(void) { return _p; }
  public:
    static TypeHandle get_class_type(void);
    static void init_type(void);
    virtual TypeHandle get_type(void) const;
    virtual TypeHandle force_init_type(void);
  private:
    GraphicsPipe* _p;
    static TypeHandle _type_handle;

    INLINE WindowPipe(void) : WindowParam() {}
  };

  static WindowFactory &get_factory();

private:

  static void read_priorities(void);

  GraphicsWindow(const GraphicsWindow&);
  GraphicsWindow &operator=(const GraphicsWindow&);

  static WindowFactory *_factory;

public:
  static TypeHandle get_class_type() {
    return _type_handle;
  }
  static void init_type() {
    Configurable::init_type();
    ReferenceCount::init_type();
    register_type(_type_handle, "GraphicsWindow",
                  Configurable::get_class_type(),
                  ReferenceCount::get_class_type());
  }
  virtual TypeHandle get_type() const {
    return get_class_type();
  }
  virtual TypeHandle force_init_type() {init_type(); return get_class_type();}

private:
  static TypeHandle _type_handle;

  friend class GraphicsPipe;
};

#include "graphicsWindow.I"

#endif /* GRAPHICSWINDOW_H */
