// Filename: configFlags.h
// Created by:  drose (21Oct04)
//
////////////////////////////////////////////////////////////////////
//
// PANDA 3D SOFTWARE
// Copyright (c) 2001 - 2004, Disney Enterprises, Inc.  All rights reserved
//
// All use of this software is subject to the terms of the Panda 3d
// Software license.  You should have received a copy of this license
// along with this source code; you will also find a current copy of
// the license at http://etc.cmu.edu/panda3d/docs/license/ .
//
// To contact the maintainers of this program write to
// panda3d-general@lists.sourceforge.net .
//
////////////////////////////////////////////////////////////////////

#ifndef CONFIGFLAGS_H
#define CONFIGFLAGS_H

#include "dtoolbase.h"

////////////////////////////////////////////////////////////////////
//       Class : ConfigFlags
// Description : This class is the base class of both ConfigVariable
//               and ConfigVariableCore.  It exists only to provide a
//               convenient name scoping for some enumerated values
//               common to both classes.
////////////////////////////////////////////////////////////////////
class EXPCL_DTOOLCONFIG ConfigFlags {
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
  };

  enum VariableFlags {
    // Trust level.  We have the bottom twelve bits reserved for a
    // trust level indicator; then the open and closed bits are a
    // special case.
    F_trust_level_mask  = 0x00000fff,
    F_open              = 0x00001000,
    F_closed            = 0x00002000,

    // F_dynamic means that the variable name is generated dynamically
    // (possibly from a very large pool) and should not be included in
    // the normal list of variable names.
    F_dynamic           = 0x00004000,

    // F_dconfig means that the variable was constructed from the
    // legacy DConfig system, rather than directly by the user.  You
    // shouldn't pass this in directly.
    F_dconfig           = 0x00008000,
  };
};

ostream &operator << (ostream &out, ConfigFlags::ValueType type);

#endif

