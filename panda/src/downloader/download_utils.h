// Filename: download_utils.h
// Created by:  mike (18Jan99)
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

#ifndef DOWNLOAD_UTILS_H
#define DOWNLOAD_UTILS_H

#include <pandabase.h>
#include <filename.h>
#include <typedef.h>

BEGIN_PUBLISH

EXPCL_PANDAEXPRESS ulong check_crc(Filename name);
EXPCL_PANDAEXPRESS ulong check_adler(Filename name);

END_PUBLISH

#endif

