/**
 * PANDA 3D SOFTWARE
 * Copyright (c) Carnegie Mellon University.  All rights reserved.
 *
 * All use of this software is subject to the terms of the revised BSD
 * license.  You should have received a copy of this license along
 * with this source code in a file named "LICENSE."
 *
 * @file pgMouseWatcherBackground.cxx
 * @author drose
 * @date 2001-08-23
 */

#include "pgMouseWatcherBackground.h"
#include "pgItem.h"

TypeHandle PGMouseWatcherBackground::_type_handle;

/**

 */
PGMouseWatcherBackground::
PGMouseWatcherBackground() :
  MouseWatcherRegion("PGMouseWatcherBackground", 0, 0, 0, 0)
{
  set_active(false);
  set_keyboard(true);
}

/**

 */
PGMouseWatcherBackground::
~PGMouseWatcherBackground() {
}

/**
 * This is a callback hook function, called whenever a mouse or keyboard button
 * is depressed while the mouse is within the background.
 */
void PGMouseWatcherBackground::
press(const MouseWatcherParameter &param) {
  PGItem::background_press(param);
}

/**
 * This is a callback hook function, called whenever a mouse or keyboard button
 * previously depressed with press() is released.
 */
void PGMouseWatcherBackground::
release(const MouseWatcherParameter &param) {
  PGItem::background_release(param);
}

/**
 * This is a callback hook function, called whenever the user presses a key.
 */
void PGMouseWatcherBackground::
keystroke(const MouseWatcherParameter &param) {
  PGItem::background_keystroke(param);
}

/**
 * This is a callback hook function, called whenever the user uses the IME.
 */
void PGMouseWatcherBackground::
candidate(const MouseWatcherParameter &param) {
  PGItem::background_candidate(param);
}
