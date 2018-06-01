/**
 * PANDA 3D SOFTWARE
 * Copyright (c) Carnegie Mellon University.  All rights reserved.
 *
 * All use of this software is subject to the terms of the revised BSD
 * license.  You should have received a copy of this license along
 * with this source code in a file named "LICENSE."
 *
 * @file pgItemNotify.cxx
 * @author drose
 * @date 2005-08-18
 */

#include "pgItemNotify.h"
#include "pgItem.h"

/**
 *
 */
PGItemNotify::
~PGItemNotify() {
  while (!_items.empty()) {
    // Disconnect all of the items that are connected to this object.
    PGItem *item = (*_items.begin());
    nassertv(item->get_notify() == this);
    (*_items.begin())->set_notify(nullptr);
  }
}

/**
 * Called whenever a watched PGItem's local transform has been changed.
 */
void PGItemNotify::
item_transform_changed(PGItem *) {
}

/**
 * Called whenever a watched PGItem's frame has been changed.
 */
void PGItemNotify::
item_frame_changed(PGItem *) {
}

/**
 * Called whenever a watched PGItem's draw_mask has been changed.
 */
void PGItemNotify::
item_draw_mask_changed(PGItem *) {
}

/**
 * Called whenever the "enter" event is triggered on a watched PGItem.  See
 * PGItem::enter_region().
 */
void PGItemNotify::
item_enter(PGItem *, const MouseWatcherParameter &) {
}

/**
 * Called whenever the "exit" event is triggered on a watched PGItem.  See
 * PGItem::exit_region().
 */
void PGItemNotify::
item_exit(PGItem *, const MouseWatcherParameter &) {
}

/**
 * Called whenever the "within" event is triggered on a watched PGItem.  See
 * PGItem::within_region().
 */
void PGItemNotify::
item_within(PGItem *, const MouseWatcherParameter &) {
}

/**
 * Called whenever the "without" event is triggered on a watched PGItem.  See
 * PGItem::without_region().
 */
void PGItemNotify::
item_without(PGItem *, const MouseWatcherParameter &) {
}

/**
 * Called whenever the "focus_in" event is triggered on a watched PGItem.  See
 * PGItem::focus_in().
 */
void PGItemNotify::
item_focus_in(PGItem *) {
}

/**
 * Called whenever the "focus_out" event is triggered on a watched PGItem.
 * See PGItem::focus_out().
 */
void PGItemNotify::
item_focus_out(PGItem *) {
}

/**
 * Called whenever the "press" event is triggered on a watched PGItem.  See
 * PGItem::press().
 */
void PGItemNotify::
item_press(PGItem *, const MouseWatcherParameter &) {
}

/**
 * Called whenever the "release" event is triggered on a watched PGItem.  See
 * PGItem::release().
 */
void PGItemNotify::
item_release(PGItem *, const MouseWatcherParameter &) {
}

/**
 * Called whenever the "keystroke" event is triggered on a watched PGItem.
 * See PGItem::keystroke().
 */
void PGItemNotify::
item_keystroke(PGItem *, const MouseWatcherParameter &) {
}

/**
 * Called whenever the "candidate" event is triggered on a watched PGItem.
 * See PGItem::candidate().
 */
void PGItemNotify::
item_candidate(PGItem *, const MouseWatcherParameter &) {
}

/**
 * Called whenever the "move" event is triggered on a watched PGItem.  See
 * PGItem::move().
 */
void PGItemNotify::
item_move(PGItem *, const MouseWatcherParameter &) {
}

/**
 * Called by PGItem when a new item is set up to notify this object.
 */
void PGItemNotify::
add_item(PGItem *item) {
  bool inserted = _items.insert(item).second;
  nassertv(inserted);
}

/**
 * Called by PGItem when an item is no longer set up to notify this object.
 */
void PGItemNotify::
remove_item(PGItem *item) {
  Items::iterator bi;
  bi = _items.find(item);
  nassertv(bi != _items.end());
  _items.erase(bi);
}
