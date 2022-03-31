/**
 * PANDA 3D SOFTWARE
 * Copyright (c) Carnegie Mellon University.  All rights reserved.
 *
 * All use of this software is subject to the terms of the revised BSD
 * license.  You should have received a copy of this license along
 * with this source code in a file named "LICENSE."
 *
 * @file config_pgui.cxx
 * @author drose
 * @date 2001-07-02
 */

#include "config_pgui.h"
#include "pgButton.h"
#include "pgCullTraverser.h"
#include "pgEntry.h"
#include "pgMouseWatcherParameter.h"
#include "pgMouseWatcherGroup.h"
#include "pgItem.h"
#include "pgMouseWatcherBackground.h"
#include "pgMouseWatcherRegion.h"
#include "pgScrollFrame.h"
#include "pgSliderBar.h"
#include "pgTop.h"
#include "pgVirtualFrame.h"
#include "pgWaitBar.h"

#include "dconfig.h"

#if !defined(CPPPARSER) && !defined(LINK_ALL_STATIC) && !defined(BUILDING_PANDA_PGUI)
  #error Buildsystem error: BUILDING_PANDA_PGUI not defined
#endif

Configure(config_pgui);
NotifyCategoryDef(pgui, "");

ConfigureFn(config_pgui) {
  init_libpgui();
}

ConfigVariableDouble scroll_initial_delay
("scroll-initial-delay", 0.3,
 PRC_DESC("This is the amount of time, in seconds, to delay after the user "
          "first clicks and holds on a scrollbar button before the scrolling "
          "continues automatically."));

ConfigVariableDouble scroll_continued_delay
("scroll-continued-delay", 0.1,
 PRC_DESC("This is the amount of time, in seconds, to delay between lines "
          "scrolled while the user is continuing to hold down the scrollbar "
          "button."));

/**
 * Initializes the library.  This must be called at least once before any of
 * the functions or classes in this library can be used.  Normally it will be
 * called by the static initializers and need not be called explicitly, but
 * special cases exist.
 */
void
init_libpgui() {
  static bool initialized = false;
  if (initialized) {
    return;
  }
  initialized = true;

  PGButton::init_type();
  PGCullTraverser::init_type();
  PGEntry::init_type();
  PGMouseWatcherParameter::init_type();
  PGMouseWatcherGroup::init_type();
  PGItem::init_type();
  PGMouseWatcherBackground::init_type();
  PGMouseWatcherRegion::init_type();
  PGScrollFrame::init_type();
  PGSliderBar::init_type();
  PGTop::init_type();
  PGVirtualFrame::init_type();
  PGWaitBar::init_type();
}
