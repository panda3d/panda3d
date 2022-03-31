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
  std::cerr << "activate_osx_application\n";
  [ [NSApplication sharedApplication] activateIgnoringOtherApps: YES ];
}

#endif  // IS_OSX
#endif  // CPPPARSER
