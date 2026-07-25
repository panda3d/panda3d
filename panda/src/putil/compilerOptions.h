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
#include "hashVal.h"

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

  // Flags that modify how the shader is compiled and cached.
  enum Flags {
    F_suppress_log       = 0x0001,  // don't write errors to notify stream
    F_debug              = 0x0002,  // compile with debugging support
    F_no_disk_cache      = 0x0010,  // disallow BamCache
    F_no_ram_cache       = 0x0020,  // disallow in-memory caching
    F_no_cache           = 0x0030,  // no_disk + no_ram
  };

  INLINE void set_flags(int flags);
  INLINE int get_flags() const;
  MAKE_PROPERTY(flags, get_flags, set_flags);

public:
  INLINE bool get_suppress_log() const;
  INLINE void set_suppress_log(bool enabled);

  INLINE bool get_debug() const;
  INLINE void set_debug(bool enabled);

  INLINE Optimize get_optimize() const;
  INLINE void set_optimize(Optimize opt);

  INLINE const DSearchPath &get_include_path() const;
  INLINE DSearchPath &get_include_path();
  INLINE void set_include_path(DSearchPath include_path);

  INLINE bool has_define(const std::string &key) const;
  INLINE std::string get_define(const std::string &key) const;

PUBLISHED:
  INLINE void define(const std::string &key, const std::string &value = "1");
  INLINE void undef(const std::string &key);

  MAKE_PROPERTY(suppress_log, get_suppress_log, set_suppress_log);
  MAKE_PROPERTY(debug, get_debug, set_debug);
  MAKE_PROPERTY(optimize, get_optimize, set_optimize);
  MAKE_PROPERTY(include_path, get_include_path, set_include_path);
  MAKE_MAP_PROPERTY(defines, has_define, get_define, define, undef);

  void output(std::ostream &out) const;
  void write_defines(std::ostream &out) const;

public:
  HashVal get_cache_key() const;

private:
  int _flags = 0;
  Optimize _optimize = Optimize::PERFORMANCE;
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
