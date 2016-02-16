/**
 * PANDA 3D SOFTWARE
 * Copyright (c) Carnegie Mellon University.  All rights reserved.
 *
 * All use of this software is subject to the terms of the revised BSD
 * license.  You should have received a copy of this license along
 * with this source code in a file named "LICENSE."
 *
 * @file pgItemNotify.h
 * @author drose
 * @date 2005-08-18
 */

#ifndef PGITEMNOTIFY_H
#define PGITEMNOTIFY_H

#include "pandabase.h"
#include "pset.h"

class PGItem;
class MouseWatcherParameter;

/**
 * Objects that inherit from this class can receive specialized messages when
 * PGItems change in certain ways.
 */
class EXPCL_PANDA_PGUI PGItemNotify {
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
