/**
 * PANDA 3D SOFTWARE
 * Copyright (c) Carnegie Mellon University.  All rights reserved.
 *
 * All use of this software is subject to the terms of the revised BSD
 * license.  You should have received a copy of this license along
 * with this source code in a file named "LICENSE."
 *
 * @file dconfig.I
 * @author cary
 * @date 2000-03-20
 */

bool DConfig::
GetBool(const std::string &sym, bool def) {
  ConfigVariableBool var(sym, def, "DConfig", ConfigFlags::F_dconfig);
  return var.get_value();
}

int DConfig::
GetInt(const std::string &sym, int def) {
  ConfigVariableInt var(sym, def, "DConfig", ConfigFlags::F_dconfig);
  return var.get_value();
}

float DConfig::
GetFloat(const std::string &sym, float def) {
  ConfigVariableDouble var(sym, (double)def, "DConfig", ConfigFlags::F_dconfig);
  return (float)var.get_value();
}

double DConfig::
GetDouble(const std::string &sym, double def) {
  ConfigVariableDouble var(sym, def, "DConfig", ConfigFlags::F_dconfig);
  return var.get_value();
}

std::string DConfig::
GetString(const std::string &sym, const std::string &def) {
  ConfigVariableString var(sym, def, "DConfig", ConfigFlags::F_dconfig);
  return var.get_value();
}
