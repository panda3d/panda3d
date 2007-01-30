// Filename: encrypt_string.h
// Created by:  drose (30Jan07)
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
