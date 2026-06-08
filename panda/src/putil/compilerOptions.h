/**
 * PANDA 3D SOFTWARE
 * Copyright (c) Carnegie Mellon University.  All rights reserved.
 *
 * All use of this software is subject to the terms of the revised BSD
 * license.  You should have received a copy of this license along
 * with this source code in a file named "LICENSE."
 *
 * @file compilerOptions.h
 * @author rdb
 * @date 2025-12-23
 */

#ifndef COMPILEROPTIONS_H
#define COMPILEROPTIONS_H

#include "pandabase.h"
#include "pmap.h"
#include "pnotify.h"
#include "dSearchPath.h"

/**
 * Specifies parameters that may be passed to the shader compiler.
 */
class EXPCL_PANDA_PUTIL CompilerOptions {
PUBLISHED:
  CompilerOptions();

  enum class Optimize {
    NONE = 0,
    PERFORMANCE,
    SIZE,
  };

public:
  INLINE bool get_suppress_log() const;
  INLINE void set_suppress_log(bool enabled);

  INLINE Optimize get_optimize() const;
  INLINE void set_optimize(Optimize opt);

  INLINE bool has_entry_point() const;
  INLINE const std::string &get_entry_point() const;
  INLINE void set_entry_point(std::string entry_point);

  INLINE const DSearchPath &get_include_path() const;
  INLINE DSearchPath &get_include_path();
  INLINE void set_include_path(DSearchPath include_path);

  INLINE bool has_define(const std::string &key) const;
  INLINE std::string get_define(const std::string &key) const;

PUBLISHED:
  INLINE void define(const std::string &key, const std::string &value = "1");
  INLINE void undef(const std::string &key);

  MAKE_PROPERTY(suppress_log, get_suppress_log, set_suppress_log);
  MAKE_PROPERTY(optimize, get_optimize, set_optimize);
  MAKE_PROPERTY(entry_point, get_entry_point, set_entry_point);
  MAKE_PROPERTY(include_path, get_include_path, set_include_path);
  MAKE_MAP_PROPERTY(defines, has_define, get_define, define, undef);

  void output(std::ostream &out) const;
  void write_defines(std::ostream &out) const;

private:
  enum Flags {
    F_suppress_log       = 0x0001,
  };

  int _flags = 0;
  Optimize _optimize = Optimize::PERFORMANCE;
  std::string _entry_point;
  DSearchPath _include_path;

  typedef pmap<std::string, std::string> Defines;
  Defines _defines;
};

INLINE std::ostream &operator << (std::ostream &out, const CompilerOptions &opts) {
  opts.output(out);
  return out;
}

#include "compilerOptions.I"

#endif
