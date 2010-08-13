// Filename: p3dEmbed.h
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

#ifndef P3DEMBED_H
#define P3DEMBED_H

#include "dtoolbase.h"
#ifdef _WIN32
#include <winsock2.h>
#endif

#include "panda3dBase.h"
#include "p3d_plugin.h"
#include "httpChannel.h"
#include "ramfile.h"
#include "fileSpec.h"
#include "pset.h"
#include "vector_string.h"

////////////////////////////////////////////////////////////////////
//       Class : P3DEmbed
// Description : This program is constructed to self-embed a p3d file
//               and execute it directly.
////////////////////////////////////////////////////////////////////
class P3DEmbed : public Panda3DBase {
public:
  P3DEmbed(bool console_environment);

  int run_embedded(streampos read_offset, int argc, char *argv[]);

  streampos _read_offset_check;
};

#endif
