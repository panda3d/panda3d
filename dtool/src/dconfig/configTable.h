// Filename: configTable.h
// Created by:  drose (15May00)
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

#ifndef CONFIGTABLE_H
#define CONFIGTABLE_H

#include <dtoolbase.h>

#include "config_setup.h"
#include "config_dconfig.h"
#include "symbolEnt.h"

#include <vector>
#include <map>

namespace Config {

class EXPCL_DTOOLCONFIG ConfigTable {
   private:
      static ConfigTable* _instance;
   public:
      typedef SymbolEnt           SymEnt;
      typedef vector_SymbolEnt    Symbol;
   private:
      typedef std::map<ConfigString, Symbol>      SymbolTable;
      typedef std::map<ConfigString, SymbolTable> TableMap;
      SymbolTable unqualified;
      TableMap qualified;
      bool _initializing;
      bool configdbg;
      bool readargs;
      bool readenvs;
      ConfigString pathsep;
      ConfigString filesep;
      ConfigString configname;
      ConfigString configsuffix;
      ConfigString configpath;
      ConfigString configcmt;
      ConfigString argsuffix;
      ConfigString commandstub;

      static void CropString(ConfigString& S);
      bool IsComment(const ConfigString&);
      static void UpCase(ConfigString&);
      ConfigString NextWord(const ConfigString& S);
      ConfigString PopNextWord(ConfigString& S);
      void ParseConfigFile(istream&, const ConfigString&);
      void ReadConfigFile(void);
      void ParseCommandEnv(ConfigString&, const ConfigString&);
      void ParseArgs(void);
      INLINE void ConfigDbgDefault(void);
      INLINE void ReadArgsDefault(void);
      INLINE void ReadEnvsDefault(void);
      INLINE void PathSepDefault(void);
      INLINE void FileSepDefault(void);
      INLINE void ConfigNameDefault(void);
      INLINE void ConfigSuffixDefault(void);
      INLINE void ConfigPathDefault(void);
      INLINE void ConfigCmtDefault(void);
      INLINE void ArgSuffixDefault(void);
      INLINE void CommandStubDefault(void);
      void MicroConfig(void);
      void GetData(void);
   protected:
      ConfigTable(void) : _initializing(true) {}
   public:
      static ConfigTable* Instance(void);
      bool AmInitializing(void);
      static bool TrueOrFalse(const ConfigString& in, bool def = false);
      bool Defined(const ConfigString& sym, const ConfigString qual="");
      SymEnt Get(const ConfigString& sym, const ConfigString qual = "");
      const Symbol& GetSym(const ConfigString& sym,
                           const ConfigString qual = "");
      INLINE ConfigString GetConfigPath(void) const;
};

#include "configTable.I"

} // close Config namespace

#endif
