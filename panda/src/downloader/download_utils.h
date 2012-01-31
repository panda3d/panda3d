// Filename: download_utils.h
// Created by:  mike (18Jan99)
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

#ifndef DOWNLOAD_UTILS_H
#define DOWNLOAD_UTILS_H

#include "pandabase.h"
#include "filename.h"

#ifdef HAVE_ZLIB

BEGIN_PUBLISH

EXPCL_PANDAEXPRESS unsigned long check_crc(Filename name);
EXPCL_PANDAEXPRESS unsigned long check_adler(Filename name);

END_PUBLISH

#endif  // HAVE_ZLIB

#endif

