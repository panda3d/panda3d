// Filename: crypto_utils.h
// Created by:  drose (07Nov00)
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

#ifndef CRYPTO_UTILS_H
#define CRYPTO_UTILS_H

#include <pandabase.h>
#include <filename.h>
#include "typedef.h"

class HashVal;

BEGIN_PUBLISH

EXPCL_PANDAEXPRESS void md5_a_file(const Filename &fname, HashVal &ret);
EXPCL_PANDAEXPRESS void md5_a_buffer(const uchar *buf, ulong len, HashVal &ret);

END_PUBLISH

#endif

