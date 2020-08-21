/**
 * PANDA 3D SOFTWARE
 * Copyright (c) Carnegie Mellon University.  All rights reserved.
 *
 * All use of this software is subject to the terms of the revised BSD
 * license.  You should have received a copy of this license along
 * with this source code in a file named "LICENSE."
 *
 * @file configFlags.h
 * @author drose
 * @date 2004-10-21
 */

#ifndef CONFIGFLAGS_H
#define CONFIGFLAGS_H

#include "dtoolbase.h"
#include "numeric_types.h"
#include "atomicAdjust.h"

/**
 * This class is the base class of both ConfigVariable and ConfigVariableCore.
 * It exists only to provide a convenient name scoping for some enumerated
 * values common to both classes.
 */
class EXPCL_DTOOL_PRC ConfigFlags {
PUBLISHED:
  enum ValueType {
    VT_undefined,
    VT_list,
    VT_string,
    VT_filename,
    VT_bool,
    VT_int,
    VT_double,
    VT_enum,
    VT_search_path,
    VT_int64,
    VT_color,
  };

  enum VariableFlags {
    // Trust level.  We have the bottom twelve bits reserved for a trust level
    // indicator; then the open and closed bits are a special case.
    F_trust_level_mask  = 0x00000fff,
    F_open              = 0x00001000,
    F_closed            = 0x00002000,

    // F_dynamic means that the variable name is generated dynamically
    // (possibly from a very large pool) and should not be included in the
    // normal list of variable names.
    F_dynamic           = 0x00004000,

    // F_dconfig means that the variable was constructed from the legacy
    // DConfig system, rather than directly by the user.  You shouldn't pass
    // this in directly.
    F_dconfig           = 0x00008000,
  };

protected:
  ALWAYS_INLINE static bool is_cache_valid(AtomicAdjust::Integer local_modified);
  ALWAYS_INLINE static void mark_cache_valid(AtomicAdjust::Integer &local_modified);
  INLINE static AtomicAdjust::Integer initial_invalid_cache();
  INLINE static void invalidate_cache();

private:
  static TVOLATILE AtomicAdjust::Integer _global_modified;
};

std::ostream &operator << (std::ostream &out, ConfigFlags::ValueType type);

#include "configFlags.I"

#endif
