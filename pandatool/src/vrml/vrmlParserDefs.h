// Filename: vrmlParserDefs.h
// Created by:  drose (30Sep04)
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

#ifndef VRMLPARSERDEFS_H
#define VRMLPARSERDEFS_H

#include "pandatoolbase.h"

void vrml_init_parser(istream &in, const string &filename);
void vrml_cleanup_parser();
int vrmlyyparse();

#endif
