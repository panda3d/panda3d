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


INLINE void ConfigTable::ConfigDbgDefault() {
  // this should not be called if  CONFIG_CONFIG=:configdbg=1 
  configdbg = false;
  microconfig_cat->set_severity(NS_info);
  dconfig_cat->set_severity(NS_info);
}

INLINE void ConfigTable::ReadArgsDefault() {
  readargs = true;
}

INLINE void ConfigTable::ReadEnvsDefault() {
  readenvs = true;
}

INLINE void ConfigTable::PathSepDefault() {
  pathsep = ": ";
}

INLINE void ConfigTable::FileSepDefault() {
  filesep = "/";
}

INLINE void ConfigTable::ConfigNameDefault() {
  configname = "Configrc";
}

INLINE void ConfigTable::ConfigSuffixDefault() {
  configsuffix = "";
}

INLINE void ConfigTable::ConfigArgsDefault() {
  configargs = "";
}

INLINE void ConfigTable::ConfigPathDefault() {
  configpath = "CFG_PATH";
}

INLINE void ConfigTable::ConfigCmtDefault() {
  configcmt = "#";
}

INLINE void ConfigTable::ArgSuffixDefault() {
  argsuffix = "_ARGS";
}

INLINE void ConfigTable::CommandStubDefault() {
  commandstub = "CONFIG";
}

INLINE ConfigString ConfigTable::GetConfigPath() const {
  return configpath;
}
