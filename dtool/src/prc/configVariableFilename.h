/**
 * PANDA 3D SOFTWARE
 * Copyright (c) Carnegie Mellon University.  All rights reserved.
 *
 * All use of this software is subject to the terms of the revised BSD
 * license.  You should have received a copy of this license along
 * with this source code in a file named "LICENSE."
 *
 * @file configVariableFilename.h
 * @author drose
 * @date 2004-11-22
 */

#ifndef CONFIGVARIABLEFILENAME_H
#define CONFIGVARIABLEFILENAME_H

#include "dtoolbase.h"
#include "configVariable.h"
#include "filename.h"
/**
 * This is a convenience class to specialize ConfigVariable as a Filename
 * type.  It is almost the same thing as ConfigVariableString, except it
 * handles an implicit Filename::expand_from() operation so that the user may
 * put OS-specific filenames, or filenames based on environment variables, in
 * the prc file.
 */
class EXPCL_DTOOL_PRC ConfigVariableFilename : public ConfigVariable {
PUBLISHED:
  INLINE ConfigVariableFilename(const std::string &name);
  INLINE ConfigVariableFilename(const std::string &name, const Filename &default_value,
                                const std::string &description = std::string(), int flags = 0);

  INLINE void operator = (const Filename &value);
  INLINE operator const Filename &() const;

  // These methods help the ConfigVariableFilename act like a Filename object.
  INLINE const char *c_str() const;
  INLINE bool empty() const;
  INLINE size_t length() const;
  INLINE char operator [] (size_t n) const;

  INLINE std::string get_fullpath() const;
  INLINE std::string get_dirname() const;
  INLINE std::string get_basename() const;
  INLINE std::string get_fullpath_wo_extension() const;
  INLINE std::string get_basename_wo_extension() const;
  INLINE std::string get_extension() const;

  // Comparison operators are handy.
  INLINE bool operator == (const Filename &other) const;
  INLINE bool operator != (const Filename &other) const;
  INLINE bool operator < (const Filename &other) const;

  INLINE void set_value(const Filename &value);
  INLINE Filename get_value() const;
  INLINE Filename get_default_value() const;
  MAKE_PROPERTY(value, get_value, set_value);
  MAKE_PROPERTY(default_value, get_default_value);

  INLINE Filename get_word(size_t n) const;
  INLINE void set_word(size_t n, const Filename &value);

private:
  void reload_cache();
  INLINE const Filename &get_ref_value() const;

private:
  AtomicAdjust::Integer _local_modified;
  Filename _cache;
};

#include "configVariableFilename.I"

#endif
