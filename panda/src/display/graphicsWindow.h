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
#include "graphicsChannel.h"
#include "displayRegion.h"
#include "graphicsStateGuardian.h"
#include "clearableRegion.h"

#include "typedef.h"
#include "configurable.h"
#include "referenceCount.h"
#include "mouseData.h"
#include "modifierButtons.h"
#include "buttonEvent.h"
#include "iterator_types.h"
#include "factory.h"
#include "pStatCollector.h"

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
class CullHandler;

////////////////////////////////////////////////////////////////////
//       Class : GraphicsWindow
// Description :
////////////////////////////////////////////////////////////////////
class EXPCL_PANDA GraphicsWindow : public Configurable, public ReferenceCount, public ClearableRegion {
PUBLISHED:
  class EXPCL_PANDA Properties : public ClearableRegion {
  PUBLISHED:
    Properties();
    INLINE Properties(const Properties &copy);
    INLINE void operator = (const Properties &copy);
    INLINE ~Properties();

    INLINE void set_origin(int xorg, int yorg);
    INLINE void set_size(int xsize, int ysize);
    INLINE void set_title(const string &title);
    INLINE void set_border(bool border);
    INLINE void set_fullscreen(bool fullscreen);
    INLINE void set_mask(uint mask);
    INLINE void set_bit_depth(int want_depth_bits, int want_color_bits);

  public:
    int _xorg;
    int _yorg;
    int _xsize;
    int _ysize;
    string _title;
    bool _border;
    bool _fullscreen;
    bool _bCursorIsVisible;
    uint _mask;
    int _want_depth_bits;
    int _want_color_bits;
  };

public:
  class EXPCL_PANDA Callback {
  public:
    virtual void draw(bool);
    virtual void idle();
  };
  typedef void (*vfn)();
  typedef void (*vfnii)(int, int);

public:

  GraphicsWindow(GraphicsPipe *pipe, const Properties &props = Properties());
  virtual ~GraphicsWindow();

  INLINE const Properties& get_properties() const;

  virtual void get_framebuffer_format(PixelBuffer::Type &fb_type, PixelBuffer::Format &fb_format);

PUBLISHED:
  INLINE int get_width() const;
  INLINE int get_height() const;
  INLINE int get_xorg() const;
  INLINE int get_yorg() const;

  // # of z bits/pixel.  purpose is to adjust camera near plane if have fewer z bits
  virtual int get_depth_bitwidth(void);  

  INLINE GraphicsStateGuardian *get_gsg() const;
  INLINE GraphicsPipe *get_pipe() const;

  INLINE void close_window();
  INLINE bool is_closed() const;

  INLINE void set_frame_number(const int);
  INLINE int get_frame_number() const;

  INLINE void set_sync(const bool);
  INLINE bool get_sync() const;

  // since this requires gsg modification, dont worry about implementing it yet
  // since init control is good enough
  // virtual void set_cursor_visible(bool bIsVisible);  // should be overridden by gsg to implement

  // resize the window to the given size
  virtual bool resize(unsigned int xsize,unsigned int ysize);  

  virtual void swap();

public:
  virtual void resized(const unsigned int, const unsigned int);

  // see if window sizes are supported (i.e. in fullscrn mode)
  // 
  // note: it might be better to implement some sort of query
  //       interface that returns an array of supported sizes,
  //       but this way is somewhat simpler and will do the job 
  //       on most cards, assuming they handle the std sizes the app
  //       knows about.
  virtual unsigned int verify_window_sizes(unsigned int numsizes,unsigned int *dimen);

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

public:
  INLINE const MouseData &get_mouse_data(int device) const;
  INLINE bool has_button_event(int device) const;
  INLINE ButtonEvent get_button_event(int device);

PUBLISHED:
  // GUI glue methods
  virtual void flag_redisplay();
  virtual void register_draw_function(GraphicsWindow::vfn);
  virtual void register_idle_function(GraphicsWindow::vfn);

public:
  virtual void begin_frame();
  void clear();
  virtual void end_frame();

  virtual void process_events();

  INLINE bool get_window_active() const;
  virtual void deactivate_window();
  virtual void reactivate_window();

  INLINE void win_display_regions_changed();

  // Statistics
  static PStatCollector _app_pcollector;
  static PStatCollector _show_code_pcollector;
  static PStatCollector _swap_pcollector;  // dxgsg needs access so this is public
  static PStatCollector _clear_pcollector;
  static PStatCollector _show_fps_pcollector;
  static PStatCollector _make_current_pcollector;

protected:
  void make_gsg();
  void release_gsg();
  virtual void do_close_window();

  typedef vector_GraphicsWindowInputDevice InputDevices;
  InputDevices _input_devices;

  PT(GraphicsStateGuardian) _gsg;
  Properties _props;

  GraphicsPipe *_pipe;
  vfn _draw_function;
  vfn _idle_function;
  vfnii _resize_function;
  int _frame_number;

  bool _is_synced;
  bool _window_active;

protected:

  Callback *_draw_callback;
  Callback *_idle_callback;

PUBLISHED:
  virtual GraphicsChannel *get_channel(int index);
  void remove_channel(int index);

  int get_max_channel_index() const;
  bool is_channel_defined(int index) const;

  INLINE int get_num_display_regions() const;
  INLINE DisplayRegion *get_display_region(int n) const;

protected:
  void declare_channel(int index, GraphicsChannel *chan);

private:
  INLINE void determine_display_regions() const;
  void do_determine_display_regions();

  typedef pvector< PT(GraphicsChannel) > Channels;
  Channels _channels;

  typedef pvector<DisplayRegion *> DisplayRegions;
  DisplayRegions _display_regions;
  bool _display_regions_stale;

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
