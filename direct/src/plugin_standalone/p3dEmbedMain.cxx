// Filename: p3dEmbedMain.cxx
// Created by:  drose (04Jan10)
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

#include "p3dEmbed.h"

#ifdef _WIN32
volatile unsigned long p3d_offset = 0xFF3D3D00;
#else
#include <stdint.h>
volatile uint32_t p3d_offset = 0xFF3D3D00;
#endif

int
main(int argc, char *argv[]) {
  P3DEmbed program(true);
  return program.run_embedded(p3d_offset, argc, argv);
}

