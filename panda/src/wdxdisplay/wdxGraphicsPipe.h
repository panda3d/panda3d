// Filename: wdxGraphicsPipe.h
// Created by:  mike (09Jan97)
//
////////////////////////////////////////////////////////////////////
//
#ifndef WDXGRAPHICSPIPE_H
#define WDXGRAPHICSPIPE_H
//
////////////////////////////////////////////////////////////////////
// Includes
////////////////////////////////////////////////////////////////////
#include <pandabase.h>

#include <string>
#include <interactiveGraphicsPipe.h>
#include "wdxGraphicsWindow.h"
#define WINDOWS_LEAN_AND_MEAN
#include <windows.h>
#undef WINDOWS_LEAN_AND_MEAN

////////////////////////////////////////////////////////////////////
// Defines
////////////////////////////////////////////////////////////////////
class Xclass;

extern char * ConvD3DErrorToString(const HRESULT &error);

////////////////////////////////////////////////////////////////////
//       Class : wdxGraphicsPipe
// Description :
////////////////////////////////////////////////////////////////////
class EXPCL_PANDADX wdxGraphicsPipe : public InteractiveGraphicsPipe {
public:
  wdxGraphicsPipe(const PipeSpecifier&);

  wdxGraphicsWindow* find_window(HWND win);
//  ButtonHandle lookup_key(WPARAM wparam) const;

  virtual TypeHandle get_window_type() const;

public:

  static GraphicsPipe* make_wdxGraphicsPipe(const FactoryParams &params);

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

  wdxGraphicsPipe(void);
  wdxGraphicsPipe(const wdxGraphicsPipe&);
  wdxGraphicsPipe& operator=(const wdxGraphicsPipe&);

};

#endif
