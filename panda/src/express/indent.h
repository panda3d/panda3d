// Filename: indent.h
// Created by:  drose (16Jan99)
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

#ifndef INDENT_H
#define INDENT_H

#include "pandabase.h"

////////////////////////////////////////////////////////////////////
//     Function: indent
//  Description: A handy function for doing text formatting.  This
//               function simply outputs the indicated number of
//               spaces to the given output stream, returning the
//               stream itself.  Useful for indenting a series of
//               lines of text by a given amount.
////////////////////////////////////////////////////////////////////
EXPCL_PANDAEXPRESS ostream &
indent(ostream &out, int indent_level);

////////////////////////////////////////////////////////////////////
//     Function: write_long_list
//  Description: Writes a list of things to the indicated output
//               stream, with a space separating each item.  One or
//               more lines will be written, and the lines will
//               automatically be broken such that no line exceeds
//               max_col columns if possible.
////////////////////////////////////////////////////////////////////
template<class InputIterator>
void
write_long_list(ostream &out, int indent_level,
                InputIterator ifirst, InputIterator ilast,
                string first_prefix = "",
                string later_prefix = "",
                int max_col = 72);

#include "indent.I"

#endif


