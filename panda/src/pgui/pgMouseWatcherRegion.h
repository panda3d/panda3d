// Filename: pgMouseWatcherRegion.h
// Created by:  drose (02Jul01)
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

#ifndef PGMOUSEWATCHERREGION_H
#define PGMOUSEWATCHERREGION_H

#include "pandabase.h"

#include "mouseWatcherRegion.h"

class PGItem;

////////////////////////////////////////////////////////////////////
//       Class : PGMouseWatcherRegion
// Description : This is a specialization on MouseWatcherRegion, to
//               add a bit more fields that are relevant to the PG
//               system.  Each PGItem corresponds to exactly one
//               PGMouseWatcherRegion.
////////////////////////////////////////////////////////////////////
class EXPCL_PANDA PGMouseWatcherRegion : public MouseWatcherRegion {
public:
  PGMouseWatcherRegion(PGItem *item);
  virtual ~PGMouseWatcherRegion();

  virtual void enter(const MouseWatcherParameter &param);
  virtual void exit(const MouseWatcherParameter &param);
  virtual void within(const MouseWatcherParameter &param);
  virtual void without(const MouseWatcherParameter &param);
  virtual void press(const MouseWatcherParameter &param);
  virtual void release(const MouseWatcherParameter &param);
  virtual void keystroke(const MouseWatcherParameter &param);

private:
  PGItem *_item;
  static int _next_index;

public:
  static TypeHandle get_class_type() {
    return _type_handle;
  }
  static void init_type() {
    MouseWatcherRegion::init_type();
    register_type(_type_handle, "PGMouseWatcherRegion",
                  MouseWatcherRegion::get_class_type());
  }
  virtual TypeHandle get_type() const {
    return get_class_type();
  }
  virtual TypeHandle force_init_type() {init_type(); return get_class_type();}

private:
  static TypeHandle _type_handle;

  friend class PGItem;
};

#include "pgMouseWatcherRegion.I"

#endif
