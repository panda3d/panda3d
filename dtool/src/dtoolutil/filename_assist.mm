/**
 * PANDA 3D SOFTWARE
 * Copyright (c) Carnegie Mellon University.  All rights reserved.
 *
 * All use of this software is subject to the terms of the revised BSD
 * license.  You should have received a copy of this license along
 * with this source code in a file named "LICENSE."
 *
 * @file filename_assist.mm
 * @author drose
 * @date 2009-04-13
 */

#include "filename_assist.h"

#ifdef IS_OSX

#include <strings.h>
#include <Foundation/Foundation.h>

#ifndef BUILD_IPHONE
#include <AppKit/AppKit.h>
#endif

using std::string;

/**
 * Copy the Objective-C string to a C++ string.
 */
static string
NSString_to_cpp_string(NSString *str) {
  size_t length = [str length];
  string result;
  for (size_t i = 0; i < length; ++i) {
    result += (char)[str characterAtIndex: i];
  }

  return result;
}

/**
 *
 */
static string
call_NSSearchPathForDirectories(NSSearchPathDirectory dirkey, NSSearchPathDomainMask domain) {
  // Ensure that Carbon has been initialized, and that we have an auto-release
  // pool.  Unfortunately, this very important function doesn't exist on the
  // IPhone.
#ifndef BUILD_IPHONE
  NSApplicationLoad();
#endif

  NSAutoreleasePool *pool = [[NSAutoreleasePool alloc] init];

  NSArray *paths = NSSearchPathForDirectoriesInDomains(dirkey, domain, YES);
  string result;
  if ([paths count] != 0) {
    result = NSString_to_cpp_string([paths objectAtIndex:0]);
  }
  [pool release];

  return result;
}

/**
 *
 */
string
get_osx_home_directory() {
#ifndef BUILD_IPHONE
  NSApplicationLoad();
#endif

  NSAutoreleasePool *pool = [[NSAutoreleasePool alloc] init];

  NSString *dir = NSHomeDirectory();
  string result = NSString_to_cpp_string(dir);
  [pool release];

  return result;
}

/**
 *
 */
string
get_osx_temp_directory() {
#ifndef BUILD_IPHONE
  NSApplicationLoad();
#endif

  NSAutoreleasePool *pool = [[NSAutoreleasePool alloc] init];

  NSString *dir = NSTemporaryDirectory();
  if (dir == nil) {
    NSArray *paths = NSSearchPathForDirectoriesInDomains(NSCachesDirectory, NSUserDomainMask, YES);
    dir = [paths objectAtIndex:0];
  }

  string result = NSString_to_cpp_string(dir);
  [pool release];

  return result;
}

/**
 *
 */
string
get_osx_user_appdata_directory() {
  string lib_result = call_NSSearchPathForDirectories(NSLibraryDirectory, NSUserDomainMask);
  if (lib_result.empty()) {
    return get_osx_home_directory();
  }

  NSAutoreleasePool *pool = [[NSAutoreleasePool alloc] init];
  NSString *lib_directory = [NSString stringWithCString:lib_result.c_str() encoding:[NSString defaultCStringEncoding]];
  NSString *app_folder = @"Panda3d";
  NSString *app_directory = [lib_directory stringByAppendingPathComponent:app_folder];
  [pool release];

  return NSString_to_cpp_string(app_directory);
}

/**
 *
 */
string
get_osx_common_appdata_directory() {
  string result = call_NSSearchPathForDirectories(NSDocumentDirectory, NSLocalDomainMask);
  if (!result.empty()) {
    return result;
  }
  return get_osx_user_appdata_directory();
}

#endif  // IS_OSX
