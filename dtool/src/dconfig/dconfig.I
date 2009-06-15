// Filename: dconfig.I
// Created by:  cary (20Mar00)
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


bool DConfig::
GetBool(const string &sym, bool def) {
  ConfigVariableBool var(sym, def, "DConfig", ConfigFlags::F_dconfig);
  return var.get_value();
}

int DConfig::
GetInt(const string &sym, int def) {
  ConfigVariableInt var(sym, def, "DConfig", ConfigFlags::F_dconfig);
  return var.get_value();
}

float DConfig::
GetFloat(const string &sym, float def) {
  ConfigVariableDouble var(sym, def, "DConfig", ConfigFlags::F_dconfig);
  return var.get_value();
}

double DConfig::
GetDouble(const string &sym, double def) {
  ConfigVariableDouble var(sym, def, "DConfig", ConfigFlags::F_dconfig);
  return var.get_value();
}

string DConfig::
GetString(const string &sym, const string &def) {
  ConfigVariableString var(sym, def, "DConfig", ConfigFlags::F_dconfig);
  return var.get_value();
}
