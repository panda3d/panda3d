// Filename: wdxGraphicsWindow7.h
// Created by:  drose (20Dec02)
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

#ifndef WDXGRAPHICSWINDOW7_H
#define WDXGRAPHICSWINDOW7_H

#include "pandabase.h"
#include "winGraphicsWindow.h"
#include "pvector.h"

class DXGraphicsStateGuardian7;

typedef struct {
   char    szDriver[MAX_DDDEVICEID_STRING];
   char    szDescription[MAX_DDDEVICEID_STRING];
   GUID    guidDeviceIdentifier;
   HMONITOR hMon;
} DXDeviceInfo;
typedef pvector<DXDeviceInfo> DXDeviceInfoVec;

typedef HRESULT (WINAPI * LPDIRECTDRAWCREATEEX)(GUID FAR * lpGuid, LPVOID  *lplpDD, REFIID  iid,IUnknown FAR *pUnkOuter);

////////////////////////////////////////////////////////////////////
//       Class : wdxGraphicsWindow7
// Description : A single graphics window for rendering DirectX under
//               Microsoft Windows.
////////////////////////////////////////////////////////////////////
class EXPCL_PANDADX wdxGraphicsWindow7 : public WinGraphicsWindow {
public:
  wdxGraphicsWindow7(GraphicsPipe *pipe, GraphicsStateGuardian *gsg,
                     const string &name);
  virtual ~wdxGraphicsWindow7();
  virtual void end_flip();
  //virtual bool begin_frame();
  virtual void make_current();
  virtual bool open_window();

protected:
  virtual void fullscreen_restored(WindowProperties &properties);
  virtual void handle_reshape();
  virtual bool do_fullscreen_resize(int x_size, int y_size);
  virtual void support_overlay_window(bool flag);

private:
  bool set_to_temp_rendertarget();
  void create_screen_buffers_and_device(DXScreenData &Display,
                                        bool force_16bpp_zbuffer);
  bool choose_device(int devnum, DXDeviceInfo *pDevinfo);
  void set_coop_levels_and_display_modes();

  DXGraphicsStateGuardian7 *_dxgsg;
  DXScreenData _wcontext;

public:
  static TypeHandle get_class_type() {
    return _type_handle;
  }
  static void init_type() {
    WinGraphicsWindow::init_type();
    register_type(_type_handle, "wdxGraphicsWindow7",
                  WinGraphicsWindow::get_class_type());
  }
  virtual TypeHandle get_type() const {
    return get_class_type();
  }
  virtual TypeHandle force_init_type() {init_type(); return get_class_type();}

private:
  static TypeHandle _type_handle;
};

#include "wdxGraphicsWindow7.I"

#endif
