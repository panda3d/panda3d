// Filename: config_char.h
// Created by:  drose (28Feb00)
//
////////////////////////////////////////////////////////////////////
//
// PANDA 3D SOFTWARE
// Copyright (c) 2001, Disney Enterprises, Inc.  All rights reserved
//
// All use of this software is subject to the terms of the Panda 3d
// Software license.  You should have received a copy of this license
// along with this source code; you will also find a current copy of
// the license at http://www.panda3d.org/license.txt .
//
// To contact the maintainers of this program write to
// panda3d@yahoogroups.com .
//
////////////////////////////////////////////////////////////////////

#ifndef CONFIG_CHAR_H
#define CONFIG_CHAR_H

#include "pandabase.h"
#include "notifyCategoryProxy.h"

// CPPParser can't handle token-pasting to a keyword.
#ifndef CPPPARSER
NotifyCategoryDecl(char, EXPCL_PANDA, EXPTP_PANDA);
#endif

// Configure variables for char package.
extern const bool EXPCL_PANDA even_animation;

extern EXPCL_PANDA void init_libchar();

#endif
