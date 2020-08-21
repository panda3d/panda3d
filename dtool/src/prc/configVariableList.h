/**
 * PANDA 3D SOFTWARE
 * Copyright (c) Carnegie Mellon University.  All rights reserved.
 *
 * All use of this software is subject to the terms of the revised BSD
 * license.  You should have received a copy of this license along
 * with this source code in a file named "LICENSE."
 *
 * @file configVariableList.h
 * @author drose
 * @date 2004-10-20
 */

#ifndef CONFIGVARIABLELIST_H
#define CONFIGVARIABLELIST_H

#include "dtoolbase.h"
#include "configVariableBase.h"

/**
 * This class is similar to ConfigVariable, but it reports its value as a list
 * of strings.  In this special case, all of the declarations of the variable
 * are returned as the elements of this list, in order.
 *
 * Note that this is different from a normal ConfigVariableString, which just
 * returns its topmost value, which can optionally be treated as a number of
 * discrete words by dividing it at the spaces.
 *
 * A ConfigVariableList cannot be modified locally.
 */
class EXPCL_DTOOL_PRC ConfigVariableList : public ConfigVariableBase {
PUBLISHED:
  INLINE ConfigVariableList(const std::string &name,
                            const std::string &description = std::string(),
                            int flags = 0);
  INLINE ~ConfigVariableList();

  INLINE size_t get_num_values() const;
  INLINE std::string get_string_value(size_t n) const;

  INLINE size_t get_num_unique_values() const;
  INLINE std::string get_unique_value(size_t n) const;

  INLINE size_t size() const;
  INLINE std::string operator [] (size_t n) const;

  void output(std::ostream &out) const;
  void write(std::ostream &out) const;
};

INLINE std::ostream &operator << (std::ostream &out, const ConfigVariableList &variable);

#include "configVariableList.I"

#endif
