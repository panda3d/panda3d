// Filename: configTable.I
// Created by:  drose (15May00)
// 
////////////////////////////////////////////////////////////////////


INLINE void ConfigTable::ConfigDbgDefault(void) {
/*
#ifdef NDEBUG
*/

  configdbg = false;
  microconfig_cat->set_severity(NS_info);
  dconfig_cat->set_severity(NS_info);

/*
#else * NDEBUG *

  configdbg = true;
  microconfig_cat->set_severity(NS_debug);
  config_cat->set_severity(NS_debug);
#endif * NDEBUG *
*/
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
