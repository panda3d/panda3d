// Filename: pgMouseWatcherBackground.h
// Created by:  drose (23Aug01)
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

#ifndef PGMOUSEWATCHERBACKGROUND_H
#define PGMOUSEWATCHERBACKGROUND_H

#include "pandabase.h"

#include "mouseWatcherRegion.h"

////////////////////////////////////////////////////////////////////
//       Class : PGMouseWatcherBackground
// Description : This is a special kind of MouseWatcherRegion that
//               doesn't have a rectangle and is never active, but
//               just quietly listens for keypresses and sends them to
//               all the PGItems with background focus.
////////////////////////////////////////////////////////////////////
class EXPCL_PANDA PGMouseWatcherBackground : public MouseWatcherRegion {
PUBLISHED:
  PGMouseWatcherBackground();
  virtual ~PGMouseWatcherBackground();

  virtual void press(const MouseWatcherParameter &param);
  virtual void release(const MouseWatcherParameter &param);
  virtual void keystroke(const MouseWatcherParameter &param);

public:
  static TypeHandle get_class_type() {
    return _type_handle;
  }
  static void init_type() {
    MouseWatcherRegion::init_type();
    register_type(_type_handle, "PGMouseWatcherBackground",
                  MouseWatcherRegion::get_class_type());
  }
  virtual TypeHandle get_type() const {
    return get_class_type();
  }
  virtual TypeHandle force_init_type() {init_type(); return get_class_type();}

private:
  static TypeHandle _type_handle;
};

#endif
