/**
 * PANDA 3D SOFTWARE
 * Copyright (c) Carnegie Mellon University.  All rights reserved.
 *
 * All use of this software is subject to the terms of the revised BSD
 * license.  You should have received a copy of this license along
 * with this source code in a file named "LICENSE."
 *
 * @file mkdir_complete.h
 * @author drose
 * @date 2009-06-29
 */

#ifndef MKDIR_COMPLETE_H
#define MKDIR_COMPLETE_H

#include <string>
#include <iostream>

bool mkdir_complete(const std::string &dirname, std::ostream &logfile);
bool mkfile_complete(const std::string &dirname, std::ostream &logfile);

#ifdef _WIN32
bool mkdir_complete_w(const std::wstring &dirname, std::ostream &logfile);
bool mkfile_complete_w(const std::wstring &dirname, std::ostream &logfile);
#endif  // _WIN32

#endif
