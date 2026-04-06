/**
 * PANDA 3D SOFTWARE
 * Copyright (c) Carnegie Mellon University.  All rights reserved.
 *
 * All use of this software is subject to the terms of the revised BSD
 * license.  You should have received a copy of this license along
 * with this source code in a file named "LICENSE."
 *
 * @file nameUniquifier.h
 * @author drose
 * @date 2000-02-16
 */

#ifndef NAMEUNIQUIFIER_H
#define NAMEUNIQUIFIER_H

#include "pandabase.h"

#include <string>
#include "pset.h"

/**
 * A handy class for converting a list of arbitrary names (strings) so that
 * each name is guaranteed to be unique in the list.  Useful for writing egg
 * files with unique vertex pool names, or for file converters to file formats
 * that require unique node names, etc.
 */
class EXPCL_PANDA_PUTIL NameUniquifier {
public:
  NameUniquifier(const std::string &separator = std::string(),
                 const std::string &empty = std::string());
  ~NameUniquifier();

  INLINE std::string add_name(const std::string &name);
  INLINE std::string add_name(const std::string &name, const std::string &prefix);

private:
  std::string add_name_body(const std::string &name, const std::string &prefix);

  typedef pset<std::string, string_hash> Names;
  Names _names;
  std::string _separator;
  std::string _empty;
  int _counter;
};

#include "nameUniquifier.I"

#endif
