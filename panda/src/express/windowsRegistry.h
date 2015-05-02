// Filename: windowsRegistry.h
// Created by:  drose (06Aug01)
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
class EXPCL_PANDAEXPRESS WindowsRegistry
{
PUBLISHED:
  enum RegLevel {
      rl_machine = 0,
      rl_user = 1
  };

  static bool set_string_value(const string &key, const string &name,
                               const string &value, RegLevel rl = rl_machine);
  static bool set_int_value(const string &key, const string &name, int value, RegLevel rl = rl_machine);

  enum Type {
    T_none,
    T_int,
    T_string,
  };
  static Type get_key_type(const string &key, const string &name, RegLevel rl = rl_machine);
  static string get_string_value(const string &key, const string &name,
                                 const string &default_value, RegLevel rl = rl_machine);
  static int get_int_value(const string &key, const string &name,
                           int default_value, RegLevel rl = rl_machine);

private:
  static bool do_set(const string &key, const string &name,
                     int data_type, const void *data, int data_length, const RegLevel rl);
  static bool do_get(const string &key, const string &name,
                     int &data_type, string &data, const RegLevel rl);
  static string format_message(int error_code);
};

#endif  // WIN32_VC

#endif
