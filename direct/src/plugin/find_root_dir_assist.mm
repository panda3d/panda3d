// Filename: filename_assist.mm
// Created by:  drose (13Apr09)
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

#include "find_root_dir.h"

#ifdef __APPLE__

#include <Foundation/Foundation.h>
#include <AppKit/AppKit.h>

////////////////////////////////////////////////////////////////////
//     Function: NSString_to_cpp_string
//  Description: Copy the Objective-C string to a C++ string.
////////////////////////////////////////////////////////////////////
static string 
NSString_to_cpp_string(NSString *str) {
  size_t length = [str length];
  string result;
  for (size_t i = 0; i < length; ++i) {
    result += (char)[str characterAtIndex: i];
  }

  return result;
}

////////////////////////////////////////////////////////////////////
//     Function: call_NSSearchPathForDirectories
//  Description: 
////////////////////////////////////////////////////////////////////
static string 
call_NSSearchPathForDirectories(NSSearchPathDirectory dirkey, NSSearchPathDomainMask domain) {
  // Ensure that Carbon has been initialized, and that we have an
  // auto-release pool.
  NSApplicationLoad();
  NSAutoreleasePool *pool = [[NSAutoreleasePool alloc] init]; 

  NSArray *paths = NSSearchPathForDirectoriesInDomains(dirkey, domain, YES);
  string result;
  if ([paths count] != 0) {
    result = NSString_to_cpp_string([paths objectAtIndex:0]);
  }
  [pool release]; 

  return result;
}

////////////////////////////////////////////////////////////////////
//     Function: get_osx_home_directory
//  Description: 
////////////////////////////////////////////////////////////////////
static string
get_osx_home_directory() {
  NSApplicationLoad();
  NSAutoreleasePool *pool = [[NSAutoreleasePool alloc] init]; 

  NSString *dir = NSHomeDirectory();
  string result = NSString_to_cpp_string(dir);
  [pool release]; 

  return result;
}

////////////////////////////////////////////////////////////////////
//     Function: find_osx_root_dir
//  Description: 
////////////////////////////////////////////////////////////////////
string
find_osx_root_dir() {
  string result = call_NSSearchPathForDirectories(NSCachesDirectory, NSUserDomainMask);
  if (!result.empty()) {
    return result + "/Panda3D";
  }
  result = get_osx_home_directory();
  if (!result.empty()) {
    return result + "/Panda3D";
  }

  return string();
}

#endif  // __APPLE__
