// Filename: wdxGraphicsWindow.h
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
#ifndef WDXGRAPHICSWINDOW_H
#define WDXGRAPHICSWINDOW_H
//#define WBD_GL_MODE 1    // if setting this, do it in wdxGraphicsStateGuardian.h too
//
////////////////////////////////////////////////////////////////////
// Includes
////////////////////////////////////////////////////////////////////
#include "dxGraphicsStateGuardian.h"

////////////////////////////////////////////////////////////////////
// Defines
////////////////////////////////////////////////////////////////////
class wdxGraphicsPipe;
class wdxGraphicsWindowGroup;

const int WDXWIN_CONFIGURE = 4;
const int WDXWIN_EVENT = 8;

typedef HRESULT (WINAPI * LPDIRECTDRAWCREATEEX)(GUID FAR * lpGuid, LPVOID  *lplpDD, REFIID  iid,IUnknown FAR *pUnkOuter);

typedef struct {
   char    szDriver[MAX_DDDEVICEID_STRING];
   char    szDescription[MAX_DDDEVICEID_STRING];
   GUID    guidDeviceIdentifier;
   HMONITOR hMon;
} DXDeviceInfo;
typedef vector<DXDeviceInfo> DXDeviceInfoVec;

////////////////////////////////////////////////////////////////////
//       Class : wdxGraphicsWindow
// Description :
////////////////////////////////////////////////////////////////////
class EXPCL_PANDADX wdxGraphicsWindow : public GraphicsWindow {
 friend class DXGraphicsStateGuardian;
 friend class DXTextureContext;
 friend class wdxGraphicsWindowGroup;

public:
  wdxGraphicsWindow(GraphicsPipe* pipe);
  wdxGraphicsWindow(GraphicsPipe* pipe,const GraphicsWindow::Properties& props);

  // this constructor will not initialize the wdx stuff, only the panda graphicswindow stuff
  wdxGraphicsWindow(GraphicsPipe* pipe,const GraphicsWindow::Properties& props,wdxGraphicsWindowGroup *pParentGroup);

  virtual ~wdxGraphicsWindow(void);
  virtual bool supports_update() const;
  virtual void update(void);
  virtual void end_frame( void );

  virtual TypeHandle get_gsg_type() const;
  static GraphicsWindow* make_wdxGraphicsWindow(const FactoryParams &params);

  void set_window_handle(HWND hwnd);

  LONG window_proc(HWND hwnd, UINT msg, WPARAM wparam, LPARAM lparam);
  void process_events(void);

  INLINE void handle_mouse_motion(int x, int y);
  INLINE void handle_mouse_entry(bool bEntering, int x, int y);
  INLINE void handle_keypress(ButtonHandle key, int x, int y );
  INLINE void handle_keyrelease(ButtonHandle key);
  void handle_window_move(int x, int y);
  void dx_setup();
  virtual void begin_frame( void );
  void show_frame();
  virtual void resize(unsigned int xsize,unsigned int ysize);
  virtual unsigned int verify_window_sizes(unsigned int numsizes,unsigned int *dimen);
  virtual int get_depth_bitwidth(void);

protected:
  void CreateScreenBuffersAndDevice(LPDIRECTDRAW7 pDD,LPDIRECT3D7 pD3DI);
  ButtonHandle lookup_key(WPARAM wparam) const;
//  virtual void config(void);
  void config_single_window(void);
  void config_window(wdxGraphicsWindowGroup *pParentGroup);
  void finish_window_setup(void);
  bool search_for_device(int devnum,DXDeviceInfo *pDevinfo);
  void setup_colormap(void);

public:
  UINT_PTR _PandaPausedTimer;
  DXGraphicsStateGuardian *_dxgsg;
  void CreateScreenBuffersAndDevice(DXScreenData &Display);
  
private:
  INLINE void track_mouse_leaving(HWND hwnd);
  wdxGraphicsWindowGroup *_pParentWindowGroup;
  HDC               _hdc;
  HPALETTE          _colormap;
  typedef enum { NotAdjusting,MovingOrResizing,Resizing } WindowAdjustType;
  WindowAdjustType  _WindowAdjustingType;
  bool              _bSizeIsMaximized;
  bool              _cursor_in_windowclientarea;
  bool              _tracking_mouse_leaving;
  bool              _ime_open;
  bool              _ime_active;
  bool              _ime_composition_w;
  bool              _exiting_window;
  bool              _window_inactive;
  bool              _active_minimized_fullscreen;
  bool              _return_control_to_app;
  int               _depth_buffer_bpp;

public:
  static TypeHandle get_class_type(void);
  static void init_type(void);
  virtual TypeHandle get_type(void) const;
  virtual TypeHandle force_init_type() {init_type(); return get_class_type();}

  void DestroyMe(bool bAtExitFnCalled);
  virtual void do_close_window();
  void deactivate_window(void);
  void reactivate_window(void);
  void handle_reshape(bool bDoDXReset);

private:
  static TypeHandle _type_handle;
};

// this class really belongs in panda, not here
class EXPCL_PANDADX wdxGraphicsWindowGroup {
// group of windows are all created at the same time
    friend class wdxGraphicsWindow;

PUBLISHED: 
    wdxGraphicsWindowGroup(GraphicsPipe *,const GraphicsWindow::Properties&);
    wdxGraphicsWindowGroup(GraphicsPipe *,const GraphicsWindow::Properties&,const GraphicsWindow::Properties&);
    wdxGraphicsWindowGroup(GraphicsPipe *,const GraphicsWindow::Properties&,const GraphicsWindow::Properties&,
                           const GraphicsWindow::Properties&);
public:
    wdxGraphicsWindowGroup(wdxGraphicsWindow *OneWindow);
    // dont publish variable length one, since FFI wont support it
    wdxGraphicsWindowGroup(GraphicsPipe *pipe,int num_windows,GraphicsWindow::Properties *WinPropArray);
    ~wdxGraphicsWindowGroup();
    void SetCoopLevelsAndDisplayModes(void);
public:
    void CreateWindows(void);
    void make_windows(GraphicsPipe *,int num_windows,GraphicsWindow::Properties *pWinPropArray);
    void initWindowGroup(void);

    pvector<wdxGraphicsWindow *> _windows;
    DXDeviceInfoVec *_pDeviceInfoVec;  // only used during init to store valid devices
    HWND      _hParentWindow;
    HINSTANCE _hDDrawDLL;
    HWND      _hOldForegroundWindow;  
    HCURSOR   _hMouseCursor;
    bool      _bLoadedCustomCursor;
    bool      _bClosingAllWindows;
    int       _numMonitors;
    LPDIRECTDRAWCREATEEX    _pDDCreateEx;
    DXDeviceInfoVec _DeviceInfoVec;

    // win32 fns that dont exist on w95
    typedef BOOL (WINAPI* PFN_GETMONITORINFO)(HMONITOR, LPMONITORINFO);
    PFN_GETMONITORINFO  _pfnGetMonitorInfo;
    typedef BOOL (WINAPI* PFN_TRACKMOUSEEVENT)(LPTRACKMOUSEEVENT);
    PFN_TRACKMOUSEEVENT _pfnTrackMouseEvent;
};

extern void set_global_parameters(void);
extern void restore_global_parameters(void);

#endif
