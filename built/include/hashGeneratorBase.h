/**
 * PANDA 3D SOFTWARE
 * Copyright (c) Carnegie Mellon University.  All rights reserved.
 *
 * All use of this software is subject to the terms of the revised BSD
 * license.  You should have received a copy of this license along
 * with this source code in a file named "LICENSE."
 *
 * @file hashGeneratorBase.h
 * @author drose
 * @date 2001-05-14
 */

#ifndef HASHGENERATORBASE_H
#define HASHGENERATORBASE_H

#include "pandabase.h"

/**
 * This is the base class for a number of classes that generate arbitrary hash
 * numbers for complex objects, based fundamentally on a sequence of integers.
 *
 * There are no virtual functions here, for performance reasons; it is
 * generally desirable to generate hash numbers as quickly as possible.  The
 * derived classes must redefine all the basic functionality.
 *
 * Thus, a compile-time decision must be made for the kind of HashGenerator
 * that is appropriate for a particular class.
 */
class EXPCL_PANDA_EXPRESS HashGeneratorBase {
public:
  INLINE HashGeneratorBase();

  INLINE size_t get_hash() const;

protected:
  size_t _hash;
};

#include "hashGeneratorBase.I"

#endif
