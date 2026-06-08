/**
 * PANDA 3D SOFTWARE
 * Copyright (c) Carnegie Mellon University.  All rights reserved.
 *
 * All use of this software is subject to the terms of the revised BSD
 * license.  You should have received a copy of this license along
 * with this source code in a file named "LICENSE."
 *
 * @file compilerOptions.I
 * @author rdb
 * @date 2025-12-23
 */

/**
 *
 */
INLINE bool CompilerOptions::
get_suppress_log() const {
  return (_flags & F_suppress_log) != 0;
}

/**
 * If enabled, does not write compiler warnings/errors to the standard notify
 * stream.
 */
INLINE void CompilerOptions::
set_suppress_log(bool enabled) {
  if (enabled) {
    _flags |= F_suppress_log;
  } else {
    _flags &= ~F_suppress_log;
  }
}

/**
 * Returns the current optimization setting.
 */
INLINE CompilerOptions::Optimize CompilerOptions::
get_optimize() const {
  return _optimize;
}

/**
 * Sets the focus of optimization: NONE (which does not perform optimization),
 * PERFORMANCE (the default) or SIZE.
 */
INLINE void CompilerOptions::
set_optimize(CompilerOptions::Optimize opt) {
  _optimize = opt;
}

/**
 * Returns true if a custom entry point was requested.
 */
INLINE bool CompilerOptions::
has_entry_point() const {
  return !_entry_point.empty();
}

/**
 * Returns the name of the entry point.
 */
INLINE const std::string &CompilerOptions::
get_entry_point() const {
  return _entry_point;
}

/**
 * Sets the name of the entry point.  If this is set to an empty string, a
 * language-specific default is used.
 */
INLINE void CompilerOptions::
set_entry_point(std::string entry_point) {
  _entry_point = std::move(entry_point);
}

/**
 * Returns the path that includes are searched along.
 */
INLINE const DSearchPath &CompilerOptions::
get_include_path() const {
  return _include_path;
}

/**
 * Returns the path that includes are searched along.
 */
INLINE DSearchPath &CompilerOptions::
get_include_path() {
  return _include_path;
}

/**
 * Sets the search path that includes are searched along.
 */
INLINE void CompilerOptions::
set_include_path(DSearchPath path) {
  _include_path = std::move(path);
}

/**
 *
 */
INLINE bool CompilerOptions::
has_define(const std::string &key) const {
  return _defines.find(key) != _defines.end();
}

/**
 *
 */
INLINE std::string CompilerOptions::
get_define(const std::string &key) const {
  auto it = _defines.find(key);
  nassertr_always(it != _defines.end(), "");
  return it->second;
}

/**
 * Sets a preprocessor definition.
 */
INLINE void CompilerOptions::
define(const std::string &key, const std::string &value) {
  _defines[key] = value;
}

/**
 * Removes a preprocessor definition previously set with define().
 */
INLINE void CompilerOptions::
undef(const std::string &key) {
  _defines.erase(key);
}
