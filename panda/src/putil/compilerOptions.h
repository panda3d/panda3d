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

/**
 * Specifies parameters that may be passed to the shader compiler.
 */
class EXPCL_PANDA_PUTIL CompilerOptions {
PUBLISHED:
  CompilerOptions() = default;

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

  INLINE bool has_define(const std::string &key) const;
  INLINE std::string get_define(const std::string &key) const;

PUBLISHED:
  INLINE void define(const std::string &key, const std::string &value = "1");
  INLINE void undef(const std::string &key);

  MAKE_PROPERTY(suppress_log, get_suppress_log, set_suppress_log);
  MAKE_PROPERTY(optimize, get_optimize, set_optimize);
  MAKE_MAP_PROPERTY(defines, has_define, get_define, define, undef);

  void output(std::ostream &out) const;
  void write_defines(std::ostream &out) const;

private:
  enum Flags {
    F_suppress_log       = 0x0001,
  };

  int _flags = 0;
  Optimize _optimize = Optimize::PERFORMANCE;

  typedef pmap<std::string, std::string> Defines;
  Defines _defines;
};

INLINE std::ostream &operator << (std::ostream &out, const CompilerOptions &opts) {
  opts.output(out);
  return out;
}

#include "compilerOptions.I"

#endif
