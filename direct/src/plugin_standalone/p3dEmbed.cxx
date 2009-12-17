// Filename: p3dEmbed.cxx
// Created by:  rdb (07Dec09)
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

#include "p3d_plugin_composite1.cxx"
#include "panda3dBase.cxx"

#ifdef _WIN32
const unsigned long p3d_offset = 0xFF3D3D00;
#else
#include <stdint.h>
const uint32_t p3d_offset = 0xFF3D3D00;
#endif

int
main(int argc, char *argv[]) {
  Panda3DBase program(true);
  return program.run_embedded(p3d_offset, argc, argv);
}

