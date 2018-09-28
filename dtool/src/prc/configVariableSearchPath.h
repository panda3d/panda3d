/**
 * PANDA 3D SOFTWARE
 * Copyright (c) Carnegie Mellon University.  All rights reserved.
 *
 * All use of this software is subject to the terms of the revised BSD
 * license.  You should have received a copy of this license along
 * with this source code in a file named "LICENSE."
 *
 * @file configVariableSearchPath.h
 * @author drose
 * @date 2004-10-21
 */

#ifndef CONFIGVARIABLESEARCHPATH_H
#define CONFIGVARIABLESEARCHPATH_H

#include "dtoolbase.h"
#include "configVariableBase.h"
#include "dSearchPath.h"

/**
 * This is similar to a ConfigVariableList, but it returns its list as a
 * DSearchPath, as a list of directories.
 *
 * You may locally append directories to the end of the search path with the
 * methods here, or prepend them to the beginning.  Use these methods to make
 * adjustments to the path; do not attempt to directly modify the const
 * DSearchPath object returned by get_value().
 *
 * Unlike other ConfigVariable types, local changes (made by calling
 * append_directory() and prepend_directory()) are specific to this particular
 * instance of the ConfigVariableSearchPath.  A separate instance of the same
 * variable, created by using the same name to the constructor, will not
 * reflect the local changes.
 */
class EXPCL_DTOOL_PRC ConfigVariableSearchPath : public ConfigVariableBase {
PUBLISHED:
  INLINE ConfigVariableSearchPath(const std::string &name,
                                  const std::string &description = std::string(),
                                  int flags = 0);
  INLINE ConfigVariableSearchPath(const std::string &name,
                                  const DSearchPath &default_value,
                                  const std::string &description,
                                  int flags = 0);
  INLINE ConfigVariableSearchPath(const std::string &name,
                                  const std::string &default_value,
                                  const std::string &description,
                                  int flags = 0);
  INLINE ~ConfigVariableSearchPath();

  INLINE operator DSearchPath () const;
  INLINE DSearchPath get_value() const;
  INLINE const DSearchPath &get_default_value() const;
  MAKE_PROPERTY(value, get_value);
  MAKE_PROPERTY(default_value, get_default_value);

  INLINE bool clear_local_value();

  INLINE void clear();
  INLINE void append_directory(const Filename &directory);
  INLINE void prepend_directory(const Filename &directory);
  INLINE void append_path(const std::string &path,
                          const std::string &separator = std::string());
  INLINE void append_path(const DSearchPath &path);
  INLINE void prepend_path(const DSearchPath &path);

  INLINE bool is_empty() const;
  INLINE size_t get_num_directories() const;
  INLINE Filename get_directory(size_t n) const;
  MAKE_SEQ(get_directories, get_num_directories, get_directory);
  MAKE_SEQ_PROPERTY(directories, get_num_directories, get_directory);

  INLINE Filename find_file(const Filename &filename) const;
  INLINE size_t find_all_files(const Filename &filename,
                               DSearchPath::Results &results) const;
  INLINE DSearchPath::Results find_all_files(const Filename &filename) const;

  INLINE void output(std::ostream &out) const;
  INLINE void write(std::ostream &out) const;

private:
  void reload_search_path();

  mutable MutexImpl _lock;
  DSearchPath _default_value;
  DSearchPath _prefix, _postfix;

  AtomicAdjust::Integer _local_modified;
  DSearchPath _cache;
};

INLINE std::ostream &operator << (std::ostream &out, const ConfigVariableSearchPath &variable);

#include "configVariableSearchPath.I"

#endif
