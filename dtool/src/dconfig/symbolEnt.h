// Filename: symbolEnt.h
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

#ifndef SYMBOLENT_H
#define SYMBOLENT_H

#include "dtoolbase.h"

#include "config_setup.h"
#include "pvector.h"

namespace Config {

class EXPCL_DTOOLCONFIG SymbolEnt {
   public:
      enum SymbolEntSrc { ConfigFile, Environment, CommandEnv, Commandline,
                          Other, Invalid };
   private:
      ConfigString _src;
      ConfigString _val;
      bool _state;
      enum SymbolEntSrc _srctok;
   public:
      SymbolEnt() : _srctok(Invalid) {}
      SymbolEnt(enum SymbolEntSrc tok, const ConfigString& val,
                const ConfigString& src = "", bool state = false)
        : _src(src), _val(val), _state(state), _srctok(tok) {}
      SymbolEnt(const SymbolEnt& c) : _src(c._src), _val(c._val),
          _state(c._state), _srctok(c._srctok) {}
      INLINE SymbolEnt& operator=(const SymbolEnt&);

      // We need these so we can explicitly instantiate the vector,
      // below.  They're not actually used and do nothing useful.
      INLINE bool operator == (const SymbolEnt &other) const;
      INLINE bool operator != (const SymbolEnt &other) const;
      INLINE bool operator < (const SymbolEnt &other) const;

      INLINE ConfigString Src(void) const;
      INLINE ConfigString Val(void) const;
      INLINE SymbolEntSrc SrcTok(void) const;
      INLINE bool State(void) const;
};

} // close Config namespace

#define EXPCL EXPCL_DTOOLCONFIG
#define EXPTP EXPTP_DTOOLCONFIG
#define TYPE Config::SymbolEnt
#define NAME vector_SymbolEnt
#include "vector_src.h"

namespace Config {

typedef ::vector_SymbolEnt vector_SymbolEnt;

#include "symbolEnt.I"

} // close Config namespace


// Tell GCC that we'll take care of the instantiation explicitly here.
#ifdef __GNUC__
#pragma interface
#endif

#endif
