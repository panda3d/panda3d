/**
 * PANDA 3D SOFTWARE
 * Copyright (c) Carnegie Mellon University.  All rights reserved.
 *
 * All use of this software is subject to the terms of the revised BSD
 * license.  You should have received a copy of this license along
 * with this source code in a file named "LICENSE."
 *
 * @file parse_vrml.h
 * @author drose
 * @date 1999-06-24
 */

#ifndef PARSE_VRML_H
#define PARSE_VRML_H

#include "vrmlNode.h"
#include "filename.h"

VrmlScene *parse_vrml(Filename filename);
VrmlScene *parse_vrml(std::istream &in, const std::string &filename);

#endif
