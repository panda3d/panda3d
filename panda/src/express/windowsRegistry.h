// Filename: windowsRegistry.h
// Created by:  drose (06Aug01)
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

#ifndef WINDOWSREGISTRY_H
#define WINDOWSREGISTRY_H

#include "pandabase.h"

// This class is only defined on Windows builds.
#ifdef WIN32_VC

////////////////////////////////////////////////////////////////////
//       Class : WindowsRegistry
// Description : This class provides a hook to Python to read and
//               write strings and integers to the windows registry.
//               It automatically converts strings from utf-8 encoding
//               and stores them in Unicode (and conversely reconverts
//               them on retrieval).
////////////////////////////////////////////////////////////////////
class EXPCL_PANDAEXPRESS WindowsRegistry {
PUBLISHED:
  static bool set_string_value(const string &key, const string &name,
                               const string &value);
  static bool set_int_value(const string &key, const string &name, int value);

  enum Type {
    T_none,
    T_int,
    T_string,
  };
  static Type get_key_type(const string &key, const string &name);
  static string get_string_value(const string &key, const string &name,
                                 const string &default_value);
  static int get_int_value(const string &key, const string &name,
                           int default_value);

private:
  static bool do_set(const string &key, const string &name,
                     int data_type, const void *data, int data_length);
  static bool do_get(const string &key, const string &name,
                     int &data_type, string &data);
  static string format_message(int error_code);
};

#endif  // WIN32_VC

#endif
