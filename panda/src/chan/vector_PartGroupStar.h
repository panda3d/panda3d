// Filename: vector_PartGroupStar.h
// Created by:  drose (06Jul00)
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

#ifndef VECTOR_PARTGROUPSTAR_H
#define VECTOR_PARTGROUPSTAR_H

#include "pandabase.h"
#include "partGroup.h"
#include "pvector.h"

////////////////////////////////////////////////////////////////////
//       Class : vector_PartGroupStar
// Description : A vector of PartGroup pointers.  This class is
//               defined once here, and exported to PANDA.DLL; other
//               packages that want to use a vector of this type
//               (whether they need to export it or not) should
//               include this header file, rather than defining the
//               vector again.
////////////////////////////////////////////////////////////////////

#define EXPCL EXPCL_PANDA_CHAN
#define EXPTP EXPTP_PANDA_CHAN
#define TYPE PartGroup *
#define NAME vector_PartGroupStar

#include "vector_src.h"

// Tell GCC that we'll take care of the instantiation explicitly here.
#ifdef __GNUC__
#pragma interface
#endif

#endif
