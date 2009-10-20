// Filename: find_root_dir.h
// Created by:  drose (29Jun09)
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

#ifndef FIND_ROOT_DIR_H
#define FIND_ROOT_DIR_H

#include <string>
#include <iostream>
using namespace std;

string find_root_dir();

#ifdef __APPLE__
string find_osx_root_dir();
#endif  // __APPLE__

#endif
