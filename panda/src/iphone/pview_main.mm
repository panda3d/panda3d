// Filename: pview_main.mm
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

extern "C" int main(int argc, char *argv[]);

int
main(int argc, char *argv[]) { 
  NSAutoreleasePool *pool = [[NSAutoreleasePool alloc] init]; 

  /* Call with the name of our application delegate class */ 
  int retVal = UIApplicationMain(argc, argv, nil, @"PviewAppDelegate"); 
  [pool release]; 
  return retVal; 
} 
