/**
 * PANDA 3D SOFTWARE
 * Copyright (c) Carnegie Mellon University.  All rights reserved.
 *
 * All use of this software is subject to the terms of the revised BSD
 * license.  You should have received a copy of this license along
 * with this source code in a file named "LICENSE."
 *
 * @file windowsRegistry.h
 * @author drose
 * @date 2001-08-06
 */

#ifndef WINDOWSREGISTRY_H
#define WINDOWSREGISTRY_H

#include "pandabase.h"

// This class is only defined on Windows builds.
#ifdef WIN32_VC

/**
 * This class provides a hook to Python to read and write strings and integers
 * to the windows registry.  It automatically converts strings from utf-8
 * encoding and stores them in Unicode (and conversely reconverts them on
 * retrieval).
 */
class EXPCL_PANDA_EXPRESS WindowsRegistry
{
PUBLISHED:
  enum RegLevel {
      rl_machine = 0,
      rl_user = 1
  };

  static bool set_string_value(const std::string &key, const std::string &name,
                               const std::string &value, RegLevel rl = rl_machine);
  static bool set_int_value(const std::string &key, const std::string &name, int value, RegLevel rl = rl_machine);

  enum Type {
    T_none,
    T_int,
    T_string,
  };
  static Type get_key_type(const std::string &key, const std::string &name, RegLevel rl = rl_machine);
  static std::string get_string_value(const std::string &key, const std::string &name,
                                 const std::string &default_value, RegLevel rl = rl_machine);
  static int get_int_value(const std::string &key, const std::string &name,
                           int default_value, RegLevel rl = rl_machine);

private:
  static bool do_set(const std::string &key, const std::string &name,
                     int data_type, const void *data, int data_length, const RegLevel rl);
  static bool do_get(const std::string &key, const std::string &name,
                     int &data_type, std::string &data, const RegLevel rl);
  static std::string format_message(int error_code);
};

#endif  // WIN32_VC

#endif
