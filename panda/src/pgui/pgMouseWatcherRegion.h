// Filename: pgMouseWatcherRegion.h
// Created by:  drose (02Jul01)
//
////////////////////////////////////////////////////////////////////
//
// PANDA 3D SOFTWARE
// Copyright (c) Carnegie Mellon University.  All rights reserved.
//
// All use of this software is subject to the terms of the revised BSD
// license.  You should have received a copy of this license along
// with this source code in a file named "LICENSE."
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
class EXPCL_PANDA_PGUI PGMouseWatcherRegion : public MouseWatcherRegion {
public:
  PGMouseWatcherRegion(PGItem *item);
  virtual ~PGMouseWatcherRegion();

  virtual void enter_region(const MouseWatcherParameter &param);
  virtual void exit_region(const MouseWatcherParameter &param);
  virtual void within_region(const MouseWatcherParameter &param);
  virtual void without_region(const MouseWatcherParameter &param);
  virtual void press(const MouseWatcherParameter &param);
  virtual void release(const MouseWatcherParameter &param);
  virtual void keystroke(const MouseWatcherParameter &param);
  virtual void candidate(const MouseWatcherParameter &param);
  virtual void move(const MouseWatcherParameter &param);

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
