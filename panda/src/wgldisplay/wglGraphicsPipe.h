// Filename: wglGraphicsPipe.h
// Created by:  mike (09Jan97)
//
////////////////////////////////////////////////////////////////////
//
#ifndef WGLGRAPHICSPIPE_H
#define WGLGRAPHICSPIPE_H
//
////////////////////////////////////////////////////////////////////
// Includes
////////////////////////////////////////////////////////////////////
#include <pandabase.h>

#include <string>
#include <interactiveGraphicsPipe.h>
#include "wglGraphicsWindow.h"
#define WINDOWS_LEAN_AND_MEAN
#include <windows.h>
#undef WINDOWS_LEAN_AND_MEAN

////////////////////////////////////////////////////////////////////
// Defines
////////////////////////////////////////////////////////////////////
class Xclass;

////////////////////////////////////////////////////////////////////
//       Class : wglGraphicsPipe
// Description :
////////////////////////////////////////////////////////////////////
class EXPCL_PANDAGL wglGraphicsPipe : public InteractiveGraphicsPipe {
public:
  wglGraphicsPipe(const PipeSpecifier&);

  wglGraphicsWindow* find_window(HWND win);
  ButtonHandle lookup_key(WPARAM wparam) const;

  virtual TypeHandle get_window_type() const;

public:

  static GraphicsPipe* make_wglGraphicsPipe(const FactoryParams &params);

  static TypeHandle get_class_type(void);
  static void init_type(void);
  virtual TypeHandle get_type(void) const;
  virtual TypeHandle force_init_type() {init_type(); return get_class_type();}

private:

  static TypeHandle _type_handle;

  int				_width;
  int				_height;
  bool				_shift;

protected:

  wglGraphicsPipe(void);
  wglGraphicsPipe(const wglGraphicsPipe&);
  wglGraphicsPipe& operator=(const wglGraphicsPipe&);

  static LONG WINAPI static_window_proc(HWND hwnd, UINT msg, WPARAM wparam,
	LPARAM lparam);
  LONG window_proc(HWND hwnd, UINT msg, WPARAM wparam, LPARAM lparam);
};

#endif
