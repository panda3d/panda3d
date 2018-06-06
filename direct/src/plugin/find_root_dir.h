/**
 * PANDA 3D SOFTWARE
 * Copyright (c) Carnegie Mellon University.  All rights reserved.
 *
 * All use of this software is subject to the terms of the revised BSD
 * license.  You should have received a copy of this license along
 * with this source code in a file named "LICENSE."
 *
 * @file find_root_dir.h
 * @author drose
 * @date 2009-06-29
 */

#ifndef FIND_ROOT_DIR_H
#define FIND_ROOT_DIR_H

#include <string>
#include <iostream>
using namespace std;

std::string find_root_dir();

#ifdef __APPLE__
std::string find_osx_root_dir();
#endif  // __APPLE__

#endif
