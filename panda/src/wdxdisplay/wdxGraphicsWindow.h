// Filename: wdxGraphicsWindow.h
// Created by:  mike (09Jan97)
//
////////////////////////////////////////////////////////////////////
//
#ifndef WDXGRAPHICSWINDOW_H
#define WDXGRAPHICSWINDOW_H
//#define WBD_GL_MODE 1    // if setting this, do it in wdxGraphicsStateGuardian.h too
//
////////////////////////////////////////////////////////////////////
// Includes
////////////////////////////////////////////////////////////////////
#include <pandabase.h>

#include <graphicsWindow.h>
#define WINDOWS_LEAN_AND_MEAN
#include <windows.h>
#undef WINDOWS_LEAN_AND_MEAN
#include <d3d.h>


////////////////////////////////////////////////////////////////////
// Defines
////////////////////////////////////////////////////////////////////
class wdxGraphicsPipe;

const int WDXWIN_CONFIGURE =	4;
const int WDXWIN_EVENT =	8;

////////////////////////////////////////////////////////////////////
//       Class : wdxGraphicsWindow
// Description :
////////////////////////////////////////////////////////////////////
class EXPCL_PANDADX wdxGraphicsWindow : public GraphicsWindow {

friend class DXGraphicsStateGuardian;

public:
  wdxGraphicsWindow(GraphicsPipe* pipe);
  wdxGraphicsWindow(GraphicsPipe* pipe,
		     const GraphicsWindow::Properties& props);
  virtual ~wdxGraphicsWindow(void);

  virtual bool supports_update() const;
  virtual void update(void);
  virtual void end_frame( void );

  virtual TypeHandle get_gsg_type() const;
  static GraphicsWindow* make_wdxGraphicsWindow(const FactoryParams &params);

  static LONG WINAPI static_window_proc(HWND hwnd, UINT msg, WPARAM wparam,
	LPARAM lparam);
  LONG window_proc(HWND hwnd, UINT msg, WPARAM wparam, LPARAM lparam);

  INLINE bool mouse_entry_enabled(void) { return _mouse_entry_enabled; }
  INLINE bool mouse_motion_enabled(void) { return _mouse_motion_enabled; }
  INLINE bool mouse_passive_motion_enabled(void) { 
    return _mouse_passive_motion_enabled; 
  }
  void handle_window_move( int x, int y );
  void handle_mouse_motion( int x, int y );
  void handle_mouse_entry( int state, HCURSOR hMouseCursor );
  void handle_keypress( ButtonHandle key, int x, int y );
  void handle_keyrelease( ButtonHandle key, int x, int y );
  void dx_setup();
  virtual void begin_frame( void );
  void show_frame();
  DXGraphicsStateGuardian *get_dxgsg(void);

protected:
#if WBD_GL_MODE
  PIXELFORMATDESCRIPTOR* try_for_visual(/*wdxGraphicsPipe *pipe,*/
	int mask, int want_depth_bits = 1, int want_color_bits = 1);
  void choose_visual(void);
  static void get_config(PIXELFORMATDESCRIPTOR* visual, int attrib, int *value);
#else 
  ButtonHandle lookup_key(WPARAM wparam) const;

#endif
  virtual void config( void );
  void setup_colormap(void);

  void enable_mouse_input(bool val);
  void enable_mouse_motion(bool val);
  void enable_mouse_passive_motion(bool val);
  void enable_mouse_entry(bool val);

  void process_events(void);
  //void idle_wait(void);

  
public:
  HWND				_mwindow;
  HWND              _hParentWindow;

private:
  HDC				_hdc;
  HPALETTE			_colormap;


  bool				_mouse_input_enabled;
  bool				_mouse_motion_enabled;
  bool				_mouse_passive_motion_enabled;
  bool				_mouse_entry_enabled;
  int				_entry_state;
  bool				_ignore_key_repeat;
  bool				_dx_ready;

public:
  static TypeHandle get_class_type(void);
  static void init_type(void);
  virtual TypeHandle get_type(void) const;
  virtual TypeHandle force_init_type() {init_type(); return get_class_type();}
//  virtual void make_current(void);
//  virtual void unmake_current(void);


private:
  static TypeHandle _type_handle;
};

#endif
