// Filename: describe_data_verbose.h
// Created by:  drose (27Mar00)
//
////////////////////////////////////////////////////////////////////
//
// PANDA 3D SOFTWARE
// Copyright (c) 2001, Disney Enterprises, Inc.  All rights reserved
//
// All use of this software is subject to the terms of the Panda 3d
// Software license.  You should have received a copy of this license
// along with this source code; you will also find a current copy of
// the license at http://www.panda3d.org/license.txt .
//
// To contact the maintainers of this program write to
// panda3d@yahoogroups.com .
//
////////////////////////////////////////////////////////////////////

#ifndef DESCRIBE_DATA_VERBOSE_H
#define DESCRIBE_DATA_VERBOSE_H

#include <pandabase.h>

class NodeAttributes;

////////////////////////////////////////////////////////////////////
//     Function: describe_data_verbose
//  Description: Writes to the indicated output stream a
//               nicely-formatted, multi-line description of all the
//               data values included in the indicated state.
////////////////////////////////////////////////////////////////////
void describe_data_verbose(ostream &out, const NodeAttributes &state,
                           int indent_level = 0);

#endif
