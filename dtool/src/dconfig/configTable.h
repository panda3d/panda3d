// Filename: configTable.h
// Created by:  drose (15May00)
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
