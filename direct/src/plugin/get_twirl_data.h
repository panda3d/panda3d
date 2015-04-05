// Filename: get_twirl_data.h
// Created by:  drose (24Aug11)
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

#ifndef GET_TWIRL_DATA_H
#define GET_TWIRL_DATA_H

#include <stddef.h>

static const int twirl_width = 48;
static const int twirl_height = 48;
static const int twirl_num_steps = 12;

bool get_twirl_data(unsigned char data[], size_t data_length, int step,
                    int fg_r, int fg_g, int fg_b,
                    int bg_r, int bg_g, int bg_b);

#endif

