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

#include "dtoolbase.h"

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
  ConfigString configargs;
  ConfigString configpath;
  ConfigString configcmt;
  ConfigString argsuffix;
  ConfigString commandstub;

  static void CropString(ConfigString& S);
  void DeComment(ConfigString& S);
  bool IsComment(const ConfigString&);
  static void UpCase(ConfigString&);
  ConfigString NextWord(const ConfigString& S);
  ConfigString PopNextWord(ConfigString& S);
  void ParseConfigFile(istream&, const ConfigString&);
  void ReadConfigFile();
  void ParseCommandEnv(ConfigString&, const ConfigString&);
  void ParseArgs();
  INLINE void ConfigDbgDefault();
  INLINE void ReadArgsDefault();
  INLINE void ReadEnvsDefault();
  INLINE void PathSepDefault();
  INLINE void FileSepDefault();
  INLINE void ConfigNameDefault();
  INLINE void ConfigSuffixDefault();
  INLINE void ConfigArgsDefault();
  INLINE void ConfigPathDefault();
  INLINE void ConfigCmtDefault();
  INLINE void ArgSuffixDefault();
  INLINE void CommandStubDefault();
  void MicroConfig();
  void GetData();

protected:
  ConfigTable() : _initializing(true) {}

public:
  static ConfigTable* Instance();
  bool AmInitializing();
  static bool TrueOrFalse(const ConfigString& in, bool def = false);
  bool Defined(const ConfigString& sym, const ConfigString qual="");
  SymEnt Get(const ConfigString& sym, const ConfigString qual = "");
  const Symbol& GetSym(const ConfigString& sym,
                       const ConfigString qual = "");
  INLINE ConfigString GetConfigPath() const;
  INLINE bool IsConfigDbg() { return configdbg; };
};

#include "configTable.I"

} // close Config namespace

#endif
