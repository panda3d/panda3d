/**
 * PANDA 3D SOFTWARE
 * Copyright (c) Carnegie Mellon University.  All rights reserved.
 *
 * All use of this software is subject to the terms of the revised BSD
 * license.  You should have received a copy of this license along
 * with this source code in a file named "LICENSE."
 *
 * @file pathReplace.h
 * @author drose
 * @date 2003-02-07
 */

#ifndef PATHREPLACE_H
#define PATHREPLACE_H

#include "pandatoolbase.h"
#include "pathStore.h"
#include "referenceCount.h"
#include "globPattern.h"
#include "filename.h"
#include "dSearchPath.h"
#include "pvector.h"
#include "pmap.h"

/**
 * This encapsulates the user's command-line request to replace existing,
 * incorrect pathnames to models and textures from a file with correct
 * pathnames.  It corresponds to a sequence of -pr command-line options, as
 * well as the -pp option.
 *
 * This can also go the next step, which is to convert a known file into a
 * suitable form for storing in a model file.  In this capacity, it
 * corresponds to the -ps and -pd options.
 */
class PathReplace : public ReferenceCount {
public:
  PathReplace();
  ~PathReplace();

  INLINE void clear_error();
  INLINE bool had_error() const;

  INLINE void clear();
  INLINE void add_pattern(const std::string &orig_prefix, const std::string &replacement_prefix);

  INLINE int get_num_patterns() const;
  INLINE const std::string &get_orig_prefix(int n) const;
  INLINE const std::string &get_replacement_prefix(int n) const;

  INLINE bool is_empty() const;

  Filename match_path(const Filename &orig_filename,
                      const DSearchPath &additional_path = DSearchPath());
  Filename store_path(const Filename &orig_filename);

  INLINE Filename convert_path(const Filename &orig_filename,
                               const DSearchPath &additional_path = DSearchPath());

  void full_convert_path(const Filename &orig_filename,
                         const DSearchPath &additional_path,
                         Filename &resolved_path,
                         Filename &output_path);

  void write(std::ostream &out, int indent_level = 0) const;

public:
  // This is used (along with _entries) to support match_path().
  DSearchPath _path;

  // These are used to support store_path().
  PathStore _path_store;
  Filename _path_directory;
  bool _copy_files;
  Filename _copy_into_directory;

  // If this is this true, then the error flag is set (see had_error() and
  // clear_error()) if any Filename passed to match_path() or convert_path(),
  // and unmatched by one of the prefixes, happens to be an absolute pathname.
  bool _noabs;

  // If this is true, then the error flag is set if any Filename passed to
  // match_path() or convert_path() cannot be found.
  bool _exists;

private:
  bool copy_this_file(Filename &filename);

  class Component {
  public:
    INLINE Component(const std::string &component);
    INLINE Component(const Component &copy);
    INLINE void operator = (const Component &copy);

    GlobPattern _orig_prefix;
    bool _double_star;
  };
  typedef pvector<Component> Components;

  class Entry {
  public:
    Entry(const std::string &orig_prefix, const std::string &replacement_prefix);
    INLINE Entry(const Entry &copy);
    INLINE void operator = (const Entry &copy);

    bool try_match(const Filename &filename, Filename &new_filename) const;
    size_t r_try_match(const vector_string &components, size_t oi, size_t ci) const;

    std::string _orig_prefix;
    Components _orig_components;
    bool _is_local;
    std::string _replacement_prefix;
  };

  typedef pvector<Entry> Entries;
  Entries _entries;

  bool _error_flag;

  typedef pmap<Filename, Filename> Copied;
  Copied _orig_to_target;
  Copied _target_to_orig;
};

#include "pathReplace.I"

#endif
