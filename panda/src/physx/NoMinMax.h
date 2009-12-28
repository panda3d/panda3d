// Filename: NoMinMax.h
// Created by:  pratt (26Apr2006)
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

// This file exists explicitly for the purpose of undefining min and max
// before NxPhysics.h is included in libphysx_igate.cxx

#undef min
#undef max
