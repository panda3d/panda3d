// Filename: executionEnvironment.h
// Created by:  drose (15May00)
//
////////////////////////////////////////////////////////////////////
//
// PANDA 3D SOFTWARE
// Copyright (c) 2001, Disney Enterprises, Inc.  All rights reserved
//
// All use of this software is subject to the terms of the Panda 3d
// Software license.  You should have received a copy of this license
// along with this source code; you will also find a current copy of
// the license at http://www.panda3d.org/license.txt .
//
// To contact the maintainers of this program write to
// panda3d@yahoogroups.com .
//
////////////////////////////////////////////////////////////////////

#ifndef EXECUTIONENVIRONMENT_H
#define EXECUTIONENVIRONMENT_H

#include "ppremake.h"
#include "filename.h"

////////////////////////////////////////////////////////////////////
//       Class : ExecutionEnvironment
// Description : This class is borrowed from dtool/src/dtoolutil, and
//               stripped down to just the bare minimum that Filename
//               needs; and also modified to build outside of Panda.
////////////////////////////////////////////////////////////////////
class ExecutionEnvironment {
public:
  static string get_environment_variable(const string &var);
  static string expand_string(const string &str);
  static Filename get_cwd();
};

#endif
