/**
 * PANDA 3D SOFTWARE
 * Copyright (c) Carnegie Mellon University.  All rights reserved.
 *
 * All use of this software is subject to the terms of the revised BSD
 * license.  You should have received a copy of this license along
 * with this source code in a file named "LICENSE."
 *
 * @file dSearchPath.I
 * @author drose
 * @date 2000-07-01
 */

/**
 * Returns the nth filename in the set.  This method is defined to make the
 * Results object appear to be a list in Python.
 */
INLINE Filename DSearchPath::Results::
operator [] (size_t n) const {
  return get_file(n);
}

/**
 * Returns the num of filenames in the set.  This method is defined to make
 * the Results object appear to be a list in Python.
 */
INLINE size_t DSearchPath::Results::
size() const {
  return get_num_files();
}

/**
 * This variant of find_all_files() returns the new Results object, instead of
 * filling on in on the parameter list.  This is a little more convenient to
 * call from Python.
 */
INLINE DSearchPath::Results DSearchPath::
find_all_files(const Filename &filename) const {
  Results results;
  find_all_files(filename, results);
  return results;
}

/**
 * A quick-and-easy way to search a searchpath for a file when you don't feel
 * like building or keeping around a DSearchPath object.  This simply
 * constructs a temporary DSearchPath based on the indicated path string, and
 * searches that.
 */
INLINE Filename DSearchPath::
search_path(const Filename &filename, const std::string &path,
            const std::string &separator) {
  DSearchPath search(path, separator);
  return search.find_file(filename);
}
