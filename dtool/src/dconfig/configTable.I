// Filename: configTable.I
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


INLINE void ConfigTable::ConfigDbgDefault(void) {
  // this should not be called if  CONFIG_CONFIG=:configdbg=1 
  configdbg = false;
  microconfig_cat->set_severity(NS_info);
  dconfig_cat->set_severity(NS_info);
}

INLINE void ConfigTable::ReadArgsDefault(void) {
  readargs = true;
}

INLINE void ConfigTable::ReadEnvsDefault(void) {
  readenvs = true;
}

INLINE void ConfigTable::PathSepDefault(void) {
  pathsep = ": ";
}

INLINE void ConfigTable::FileSepDefault(void) {
  filesep = "/";
}

INLINE void ConfigTable::ConfigNameDefault(void) {
  configname = "Configrc";
}

INLINE void ConfigTable::ConfigSuffixDefault(void) {
  configsuffix = "";
}

INLINE void ConfigTable::ConfigPathDefault(void) {
#ifdef PENV_PS2

  // When this is empty, ReadConfigFile (ConfigTable.C) is forced
  // to look in CONFIG_PATH compiler_flag (instead of env var)

  configpath = "";
#else
  configpath = "CONFIG_PATH";
#endif
}

INLINE void ConfigTable::ConfigCmtDefault(void) {
  configcmt = "#";
}

INLINE void ConfigTable::ArgSuffixDefault(void) {
  argsuffix = "_ARGS";
}

INLINE void ConfigTable::CommandStubDefault(void) {
  commandstub = "CONFIG";
}

INLINE ConfigString ConfigTable::GetConfigPath(void) const {
  return configpath;
}
