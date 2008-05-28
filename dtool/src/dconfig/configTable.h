// Filename: configTable.h
// Created by:  drose (15May00)
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

#ifndef CONFIGTABLE_H
#define CONFIGTABLE_H

#include "dtoolbase.h"

#include "symbolEnt.h"

namespace Config {

class EXPCL_DTOOLCONFIG ConfigTable {
public:
  typedef SymbolEnt           SymEnt;
  typedef vector_SymbolEnt    Symbol;
};

#include "configTable.I"

} // close Config namespace

#endif
