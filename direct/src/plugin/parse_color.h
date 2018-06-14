/**
 * PANDA 3D SOFTWARE
 * Copyright (c) Carnegie Mellon University.  All rights reserved.
 *
 * All use of this software is subject to the terms of the revised BSD
 * license.  You should have received a copy of this license along
 * with this source code in a file named "LICENSE."
 *
 * @file parse_color.h
 * @author drose
 * @date 2011-08-25
 */

#ifndef PARSE_COLOR_H
#define PARSE_COLOR_H

#include <string>

bool parse_color(int &r, int &g, int &b, const std::string &color);

#endif
