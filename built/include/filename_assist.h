/**
 * PANDA 3D SOFTWARE
 * Copyright (c) Carnegie Mellon University.  All rights reserved.
 *
 * All use of this software is subject to the terms of the revised BSD
 * license.  You should have received a copy of this license along
 * with this source code in a file named "LICENSE."
 *
 * @file filename_assist.h
 * @author drose
 * @date 2009-04-13
 */

#ifndef FILENAME_ASSIST_H
#define FILENAME_ASSIST_H

#include "dtoolbase.h"

// A helper module on OSX to call some Objective-C Cocoa functions.

#ifdef IS_OSX

std::string get_osx_home_directory();
std::string get_osx_temp_directory();
std::string get_osx_user_appdata_directory();
std::string get_osx_common_appdata_directory();

#endif  // IS_OSX

#endif
