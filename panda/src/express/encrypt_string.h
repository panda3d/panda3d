// Filename: encrypt_string.h
// Created by:  drose (30Jan07)
//
////////////////////////////////////////////////////////////////////
//
// PANDA 3D SOFTWARE
// Copyright (c) Carnegie Mellon University.  All rights reserved.
//
// All use of this software is subject to the terms of the revised BSD
// license.  You should have received a copy of this license along
// with this source code in a file named "LICENSE."
//
////////////////////////////////////////////////////////////////////

#ifndef ENCRYPT_STRING_H
#define ENCRYPT_STRING_H

#include "pandabase.h"

#ifdef HAVE_OPENSSL

BEGIN_PUBLISH
EXPCL_PANDAEXPRESS string encrypt_string(const string &source, const string &password);
EXPCL_PANDAEXPRESS string decrypt_string(const string &source, const string &password);
END_PUBLISH

#endif  // HAVE_OPENSSL

#endif
