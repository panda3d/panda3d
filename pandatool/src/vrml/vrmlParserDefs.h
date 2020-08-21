/**
 * PANDA 3D SOFTWARE
 * Copyright (c) Carnegie Mellon University.  All rights reserved.
 *
 * All use of this software is subject to the terms of the revised BSD
 * license.  You should have received a copy of this license along
 * with this source code in a file named "LICENSE."
 *
 * @file vrmlParserDefs.h
 * @author drose
 * @date 2004-09-30
 */

#ifndef VRMLPARSERDEFS_H
#define VRMLPARSERDEFS_H

#include "pandatoolbase.h"

void vrml_init_parser(std::istream &in, const std::string &filename);
void vrml_cleanup_parser();
int vrmlyyparse();

#endif
