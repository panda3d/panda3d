// Filename: mathHelpers.h
// Created by:  mike (25Apr97)
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

#ifndef MATHHELPERS_H
#define MATHHELPERS_H

////////////////////////////////////////////////////////////////////
// Includes
///////////////////////////////////////////////////////////////////
#include "pandabase.h"

#include "luse.h"

BEGIN_PUBLISH

INLINE_MATHUTIL float distance(const LPoint3f &pos0, const LPoint3f &pos1);

END_PUBLISH

#include "mathHelpers.I"

#endif
