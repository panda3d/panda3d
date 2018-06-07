/**
 * PANDA 3D SOFTWARE
 * Copyright (c) Carnegie Mellon University.  All rights reserved.
 *
 * All use of this software is subject to the terms of the revised BSD
 * license.  You should have received a copy of this license along
 * with this source code in a file named "LICENSE."
 *
 * @file graphicsWindowProcCallbackData.h
 * @author Walt Destler
 * @date 2010-06
 */

#ifndef GRAPHICSWINDOWPROCCALLBACKDATA_H
#define GRAPHICSWINDOWPROCCALLBACKDATA_H

#include "pandabase.h"
#include "callbackData.h"
#include "touchInfo.h"

class GraphicsWindow;

/**
 * This specialization on CallbackData is passed when the callback is
 * initiated from from an implementation of the GraphicsWindowProc class, such
 * as PythonGraphicsWindowProc.
 */
class EXPCL_PANDA_DISPLAY GraphicsWindowProcCallbackData : public CallbackData {
public:
  INLINE GraphicsWindowProcCallbackData(GraphicsWindow* graphicsWindow);

  INLINE GraphicsWindow* get_graphics_window() const;

#ifdef WIN32
  INLINE void set_hwnd(uintptr_t hwnd);
  INLINE void set_msg(int msg);
  INLINE void set_wparam(int wparam);
  INLINE void set_lparam(int lparam);
#endif

PUBLISHED:
  virtual void output(std::ostream &out) const;

#ifdef WIN32
  INLINE uintptr_t get_hwnd() const;
  INLINE int get_msg() const;
  INLINE int get_wparam() const;
  INLINE int get_lparam() const;
#endif

  bool is_touch_event();
  int get_num_touches();
  TouchInfo get_touch_info(int index);

private:
  GraphicsWindow* _graphicsWindow;
#ifdef WIN32
  uintptr_t _hwnd;
  int _msg;
  int _wparam;
  int _lparam;
#endif

public:
  static TypeHandle get_class_type() {
    return _type_handle;
  }
  static void init_type() {
    CallbackData::init_type();
    register_type(_type_handle, "GraphicsWindowProcCallbackData",
                  CallbackData::get_class_type());
  }
  virtual TypeHandle get_type() const {
    return get_class_type();
  }
  virtual TypeHandle force_init_type() {init_type(); return get_class_type();}

private:
  static TypeHandle _type_handle;
};

#include "graphicsWindowProcCallbackData.I"

#endif // GRAPHICSWINDOWPROCCALLBACKDATA_H
