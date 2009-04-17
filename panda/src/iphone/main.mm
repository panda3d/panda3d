// Filename: main.mm
// Created by:  drose (10Apr09)
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

#import <UIKit/UIKit.h>

#include <mach-o/dyld.h>
#include <dlfcn.h>
#include <iostream>

extern "C" int main(int argc, char *argv[]);

int main(int argc, char *argv[]) { 
  // This is weird and hacky.  We have our main application not link
  // statically with any Panda code.  Instead, it dynamically loads in
  // the required Panda code during main().

  // We need to do this to avoid static-init ordering issues.  Cocoa
  // doesn't fully initialize all its low-level memory-allocation
  // stuff until main begins or NSApplicationLoad() is called, but
  // unfortunately NSApplicationLoad() doesn't exist on the IPhone.
  // So on the IPhone, we have to be sure we don't make any calls into
  // Panda (which might make a low-level Cocoa call) until after we
  // have started main().

  setenv("DYLD_LIBRARY_PATH", "/Applications/pview.app", 1);

  void *answer = dlopen("/Applications/pview.app/libiphone_pview.dylib", RTLD_NOW | RTLD_LOCAL);
  if (answer == (void *)NULL) {
    std::cerr << "Couldn't load dylib\n";
    exit(1);
  }

  NSAutoreleasePool * pool = [[NSAutoreleasePool alloc] init]; 
  /* Call with the name of our application delegate class */ 
  int retVal = UIApplicationMain(argc, argv, nil, @"ControllerDemoAppDelegate"); 
  [pool release]; 
  return retVal; 
} 
