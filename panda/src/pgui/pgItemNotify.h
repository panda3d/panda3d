// Filename: pgItemNotify.h
// Created by:  drose (18Aug05)
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

#ifndef PGITEMNOTIFY_H
#define PGITEMNOTIFY_H

#include "pandabase.h"

class PGItem;

////////////////////////////////////////////////////////////////////
//       Class : PGItemNotify
// Description : Objects that inherit from this class can receive
//               specialized messages when PGItems change in certain
//               ways.
////////////////////////////////////////////////////////////////////
class EXPCL_PANDA PGItemNotify {
public:
  INLINE PGItemNotify();
  virtual ~PGItemNotify();

protected:
  virtual void item_transform_changed(PGItem *item);
  virtual void item_frame_changed(PGItem *item);
  virtual void item_draw_mask_changed(PGItem *item);

  virtual void item_enter(PGItem *item, const MouseWatcherParameter &param);
  virtual void item_exit(PGItem *item, const MouseWatcherParameter &param);
  virtual void item_within(PGItem *item, const MouseWatcherParameter &param);
  virtual void item_without(PGItem *item, const MouseWatcherParameter &param);
  virtual void item_focus_in(PGItem *item);
  virtual void item_focus_out(PGItem *item);
  virtual void item_press(PGItem *item, const MouseWatcherParameter &param);
  virtual void item_release(PGItem *item, const MouseWatcherParameter &param);
  virtual void item_keystroke(PGItem *item, const MouseWatcherParameter &param);
  virtual void item_candidate(PGItem *item, const MouseWatcherParameter &param);
  virtual void item_move(PGItem *item, const MouseWatcherParameter &param);

protected:
  void add_item(PGItem *item);
  void remove_item(PGItem *item);

private:
  typedef pset<PGItem *> Items;
  Items _items;

  friend class PGItem;
};

#include "pgItemNotify.I"

#endif
