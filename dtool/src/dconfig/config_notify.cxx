// Filename: config_notify.cxx
// Created by:  drose (29Feb00)
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

#include "config_notify.h"
#include "dconfig.h"

ConfigureDef(config_notify);

ConfigureFn(config_notify) {
}

// We have to declare this as a function instead of a static const,
// because its value might be needed at static init time.
bool
get_assert_abort() {
  static bool *assert_abort = (bool *)NULL;
  if (assert_abort == (bool *)NULL) {
    assert_abort = new bool;
    *assert_abort = config_notify.GetBool("assert-abort", false);
  }
  return *assert_abort;
}

bool
get_notify_timestamp() {
  static bool *notify_timestamp = (bool *)NULL;
  if (notify_timestamp == (bool *)NULL) {
    notify_timestamp = new bool;
    *notify_timestamp = config_notify.GetBool("notify-timestamp", false);
  }
  return *notify_timestamp;
}
