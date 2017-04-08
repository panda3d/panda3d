/**
 * PANDA 3D SOFTWARE
 * Copyright (c) Carnegie Mellon University.  All rights reserved.
 *
 * All use of this software is subject to the terms of the revised BSD
 * license.  You should have received a copy of this license along
 * with this source code in a file named "LICENSE."
 *
 * @file winGraphicsPipe.h
 * @author drose
 * @date 2002-12-20
 */

#ifndef WINGRAPHICSPIPE_H
#define WINGRAPHICSPIPE_H

#include "pandabase.h"
#include "graphicsPipe.h"
#include "winGraphicsWindow.h"

/**
 * This is an abstract base class for wglGraphicsPipe and wdxGraphicsPipe;
 * that is, those graphics pipes that are specialized for working with
 * Microsoft Windows.
 *
 * There isn't much code here, since most of the fancy stuff is handled in
 * WinGraphicsWindow.  You could make a case that we don't even need a
 * WinGraphicsPipe class at all, but it is provided mainly for completeness.
 */
class EXPCL_PANDAWIN WinGraphicsPipe : public GraphicsPipe {
public:
  WinGraphicsPipe();
  virtual ~WinGraphicsPipe();

  virtual void lookup_cpu_data();

public:
  static TypeHandle get_class_type() {
    return _type_handle;
  }
  static void init_type() {
    GraphicsPipe::init_type();
    register_type(_type_handle, "WinGraphicsPipe",
                  GraphicsPipe::get_class_type());
  }
  virtual TypeHandle get_type() const {
    return get_class_type();
  }
  virtual TypeHandle force_init_type() {init_type(); return get_class_type();}

private:
  static TypeHandle _type_handle;
};

#include "winGraphicsPipe.I"

#endif
