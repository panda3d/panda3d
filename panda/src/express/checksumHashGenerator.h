// Filename: checksumHashGenerator.h
// Created by:  drose (14May01)
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

#ifndef CHECKSUMHASHGENERATOR_H
#define CHECKSUMHASHGENERATOR_H

#include "pandabase.h"

#include "hashGeneratorBase.h"

////////////////////////////////////////////////////////////////////
//       Class : ChecksumHashGenerator
// Description : This is a specific kind of HashGenerator that simply
//               adds up all of the ints.  Nothing fancy, and pretty
//               quick.
////////////////////////////////////////////////////////////////////
class EXPCL_PANDAEXPRESS ChecksumHashGenerator : public HashGeneratorBase {
public:
  INLINE void add_int(int num);
  INLINE void add_bool(bool flag);
  INLINE void add_fp(float num, float threshold);
  INLINE void add_fp(double num, double threshold);
  INLINE void add_pointer(void *ptr);
  void add_string(const string &str);
};

#include "checksumHashGenerator.I"

#endif
