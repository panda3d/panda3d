// Filename: symbolEnt.I
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


INLINE SymbolEnt& SymbolEnt::operator=(const SymbolEnt& c) {
  _src = c._src;
  _val = c._val;
  _srctok = c._srctok;
  _state = c._state;
  return *this;
}

INLINE bool SymbolEnt::
operator == (const SymbolEnt &) const {
  return true;
}

INLINE bool SymbolEnt::
operator != (const SymbolEnt &) const {
  return false;
}

INLINE bool SymbolEnt::
operator < (const SymbolEnt &) const {
  return false;
}

INLINE ConfigString SymbolEnt::Src(void) const {
  return _src;
}

INLINE ConfigString SymbolEnt::Val(void) const {
  return _val;
}

INLINE SymbolEnt::SymbolEntSrc SymbolEnt::SrcTok(void) const {
  return _srctok;
}

INLINE bool SymbolEnt::State(void) const {
  return _state;
}
