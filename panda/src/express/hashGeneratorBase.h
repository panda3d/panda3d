// Filename: hashGeneratorBase.h
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

#ifndef HASHGENERATORBASE_H
#define HASHGENERATORBASE_H

#include "pandabase.h"

////////////////////////////////////////////////////////////////////
//       Class : HashGeneratorBase
// Description : This is the base class for a number of classes that
//               generate arbitrary hash numbers for complex objects,
//               based fundamentally on a sequence of integers.
//
//               There are no virtual functions here, for performance
//               reasons; it is generally desirable to generate hash
//               numbers as quickly as possible.  The derived classes
//               must redefine all the basic functionality.
//
//               Thus, a compile-time decision must be made for the
//               kind of HashGenerator that is appropriate for a
//               particular class.
////////////////////////////////////////////////////////////////////
class EXPCL_PANDAEXPRESS HashGeneratorBase {
public:
  INLINE HashGeneratorBase();
  INLINE ~HashGeneratorBase();

  INLINE size_t get_hash() const;

protected:
  size_t _hash;
};

#include "hashGeneratorBase.I"

#endif
