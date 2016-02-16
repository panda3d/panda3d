/**
 * PANDA 3D SOFTWARE
 * Copyright (c) Carnegie Mellon University.  All rights reserved.
 *
 * All use of this software is subject to the terms of the revised BSD
 * license.  You should have received a copy of this license along
 * with this source code in a file named "LICENSE."
 *
 * @file showBase_assist.mm
 */

#ifndef CPPPARSER
#include "directbase.h"

#ifdef IS_OSX

// We have to include AppKit before showBase.h to avoid a namespace collision.
#include <AppKit/AppKit.h>
#include "showBase.h"

/**
 * Activates the current application for Mac OSX.
 */
void
activate_osx_application() {
  cerr << "activate_osx_application\n";
  [ [NSApplication sharedApplication] activateIgnoringOtherApps: YES ];
}

#endif  // IS_OSX
#endif  // CPPPARSER
