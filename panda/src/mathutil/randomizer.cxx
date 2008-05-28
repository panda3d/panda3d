// Filename: randomizer.cxx
// Created by:  drose (18Jan07)
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

#include "randomizer.h"

Mersenne Randomizer::_next_seed(0);
bool Randomizer::_got_first_seed = false;

