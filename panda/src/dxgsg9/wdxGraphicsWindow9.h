// Filename: wdxGraphicsWindow8.h
// Created by:   masad (02Jan04)
//
////////////////////////////////////////////////////////////////////
//
// PANDA 3D SOFTWARE
// Copyright (c) 2001 - 2004, Disney Enterprises, Inc.  All rights reserved
//
// All use of this software is subject to the terms of the Panda 3d
// Software license.  You should have received a copy of this license
// along with this source code; you will also find a current copy of
// the license at http://etc.cmu.edu/panda3d/docs/license/ .
//
// To contact the maintainers of this program write to
// panda3d-general@lists.sourceforge.net .
//
////////////////////////////////////////////////////////////////////

#ifndef wdxGraphicsWindow9_H
#define wdxGraphicsWindow9_H

#include "pandabase.h"
#include "winGraphicsWindow.h"
#include "dxGraphicsStateGuardian9.h"
#include "dxInput9.h"
#include "wdxGraphicsPipe9.h"

class wdxGraphicsPipe9;

static const int WDXWIN_CONFIGURE = 4;
static const int WDXWIN_EVENT = 8;

//#define FIND_CARD_MEMAVAILS

////////////////////////////////////////////////////////////////////
//       Class : wdxGraphicsWindow9
// Description : A single graphics window for rendering DirectX under
//               Microsoft Windows.
////////////////////////////////////////////////////////////////////
class EXPCL_PANDADX wdxGraphicsWindow9 : public WinGraphicsWindow {
public:
  wdxGraphicsWindow9(GraphicsPipe *pipe, GraphicsStateGuardian *gsg,
                     const string &name);
  virtual ~wdxGraphicsWindow9();
  virtual bool open_window(void);
  virtual void reset_window(bool swapchain);

  virtual int verify_window_sizes(int numsizes, int *dimen);

  virtual bool begin_frame();
  virtual void end_flip();
  virtual LONG window_proc(HWND hwnd, UINT msg, WPARAM wparam, LPARAM lparam);
  virtual bool handle_mouse_motion(int x, int y);

protected:
  virtual void fullscreen_restored(WindowProperties &properties);
  virtual void handle_reshape();
  virtual bool do_fullscreen_resize(int x_size, int y_size);
  virtual void support_overlay_window(bool flag);

private:
  //  bool set_to_temp_rendertarget();
  void create_screen_buffers_and_device(DXScreenData &Display,
                                        bool force_16bpp_zbuffer);

  bool choose_device(void);
  bool search_for_device(wdxGraphicsPipe9 *dxpipe, DXDeviceInfo *device_info);

  //  void set_coop_levels_and_display_modes();
/*
  void search_for_valid_displaymode(UINT RequestedX_Size, UINT RequestedY_Size,
                                    bool bWantZBuffer, bool bWantStencil,
                                    UINT *pSupportedScreenDepthsMask,
                                    bool *pCouldntFindAnyValidZBuf,
                                    D3DFORMAT *pSuggestedPixFmt,
                                    bool bVerboseMode = false);
*/
  bool reset_device_resize_window(UINT new_xsize, UINT new_ysize);
  void init_resized_window();
  static int D3DFMT_to_DepthBits(D3DFORMAT fmt);
  static bool is_badvidmem_card(D3DADAPTER_IDENTIFIER9 *pDevID);

  DXGraphicsStateGuardian9 *_dxgsg;
  DXScreenData _wcontext;

  int _depth_buffer_bpp;
  bool _awaiting_restore;

public:
  static TypeHandle get_class_type() {
    return _type_handle;
  }
  static void init_type() {
    WinGraphicsWindow::init_type();
    register_type(_type_handle, "wdxGraphicsWindow9",
                  WinGraphicsWindow::get_class_type());
  }
  virtual TypeHandle get_type() const {
    return get_class_type();
  }
  virtual TypeHandle force_init_type() {init_type(); return get_class_type();}
  virtual void make_current(void);

private:
  static TypeHandle _type_handle;
  friend class wdxGraphicsPipe9;
};

//extern bool is_badvidmem_card(D3DADAPTER_IDENTIFIER9 *pDevID);

#include "wdxGraphicsWindow9.I"

#endif
