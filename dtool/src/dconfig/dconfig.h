// Filename: dconfig.h
// Created by:  cary (14Jul98)
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

#ifndef DCONFIG_H
#define DCONFIG_H

#include "dtoolbase.h"

#include "config_setup.h"
#include "config_dconfig.h"
#include "configTable.h"
#include "executionEnvironment.h"
#include "configVariableString.h"
#include "configVariableBool.h"
#include "configVariableInt.h"
#include "configVariableDouble.h"
#include "configVariableList.h"

#include <vector>
#include <map>
#include <time.h>

namespace Config {

// Config is, itself, configurable.  The things that can be set are:
//    path separator      ( ': ' ) [pathsep]
//    filename separator  ( / ) [filesep]
//    config filename     ( Configrc ) [configname]
//    config file suffix  ( ) [configsuffix]
//    config file path    ( $CONFIG_PATH ) [configpath]
//        (can be multiple of these)
//    config file comment ( '#' ) [configcmt]
//    'args' suffix       ( _ARGS ) [argsuffix]
//    command arg env     ( CONFIG + _____ ) [commandstub]
//    debugging output    ( false ) [configdbg]
//    read commandline    ( true ) [readargs]
//    read environment    ( true ) [readenv]
// all of these can be set from the one unconfigurable thing in config, the
// environment variable called CONFIG_CONFIG.  The format is:
//    separator_char{{field_id}={field_value}separator_char}*

// Call these functions to get stats on how Config is spending its
// time.
EXPCL_DTOOLCONFIG INLINE double get_total_time_config_init();
EXPCL_DTOOLCONFIG INLINE double get_total_time_external_init();
EXPCL_DTOOLCONFIG INLINE double get_total_num_get();

// These globals are intended for internal bookkeeping by Config to
// compute the above functions' return values.
EXPCL_DTOOLCONFIG extern clock_t total_time_config_init;
EXPCL_DTOOLCONFIG extern clock_t total_time_external_init;
EXPCL_DTOOLCONFIG extern int total_num_get;

template <class GetConfig>
class Config {
public:
  Config();
  ~Config();
  static bool         AmInitializing();
  static ConfigString Name();
  static bool         Flag();
  static void         Init();
  static bool         Defined(const ConfigString& sym);
  static ConfigString Get(const ConfigString sym);
  static ConfigTable::Symbol& GetAll(const ConfigString,
                                     ConfigTable::Symbol&);
PUBLISHED:
  static bool         GetBool(const ConfigString sym, bool def = false);
  static int          GetInt(const ConfigString sym, int def = 0);
  static float        GetFloat(const ConfigString sym, float def = 0.);
  static double       GetDouble(const ConfigString sym, double def = 0.);
  static ConfigString GetString(const ConfigString sym,
                                const ConfigString def = "");
protected:
  static void ConfigFunc();
  static void Flag(bool);
};

// Implementation follows

template<class GetConfig>
void Config<GetConfig>::ConfigFunc() {
  GetConfig::config_func();
}

template<class GetConfig>
bool Config<GetConfig>::AmInitializing() {
  return false;
}

template<class GetConfig>
ConfigString Config<GetConfig>::Name() {
   return GetConfig::get_name();
}

template<class GetConfig>
bool Config<GetConfig>::Flag() {
   return GetConfig::_flag;
}

template<class GetConfig>
void Config<GetConfig>::Flag(bool f) {
   GetConfig::_flag = f;
}

template<class GetConfig>
void Config<GetConfig>::Init() {
   if (Flag())
      return;

   Flag(true);

   ConfigFunc();
}

template<class GetConfig>
Config<GetConfig>::Config() {
   Init();
}

template<class GetConfig>
Config<GetConfig>::~Config() {
}

template<class GetConfig>
bool Config<GetConfig>::Defined(const ConfigString& sym)
{
   Init();
   return true;
}

template<class GetConfig>
ConfigString Config<GetConfig>::Get(ConfigString sym)
{
   Init();
   ConfigVariableString var(sym);
   return var.get_value();
}

template<class GetConfig>
ConfigTable::Symbol& Config<GetConfig>::GetAll(const ConfigString sym,
                                                ConfigTable::Symbol& s)
{
   Init();
   ConfigVariableList var(sym, "DConfig");

   int num_values = var.get_num_values();
   for (int i = 0; i < num_values; i++) {
     string value = var.get_string_value(i);
     s.push_back(SymbolEnt(SymbolEnt::ConfigFile, value));
   }

   return s;
}

template<class GetConfig>
bool Config<GetConfig>::GetBool(const ConfigString sym, bool def)
{
   Init();
   ConfigVariableBool var(sym, def, "DConfig");
   return var.get_value();
}

template<class GetConfig>
int Config<GetConfig>::GetInt(const ConfigString sym, int def)
{
   Init();
   ConfigVariableInt var(sym, def, "DConfig");
   return var.get_value();
}

template<class GetConfig>
float Config<GetConfig>::GetFloat(const ConfigString sym, float def)
{
   Init();
   ConfigVariableDouble var(sym, def, "DConfig");
   return var.get_value();
}

template<class GetConfig>
double Config<GetConfig>::GetDouble(const ConfigString sym, double def)
{
   Init();
   ConfigVariableDouble var(sym, def, "DConfig");
   return var.get_value();
}

template<class GetConfig>
ConfigString Config<GetConfig>::GetString(const ConfigString sym,
                                           const ConfigString def)
{
   Init();
   ConfigVariableString var(sym, def, "DConfig");
   return var.get_value();
}

#include "dconfig.I"

} // close Config namespace


// Finally, here is a set of handy macros to define and reference a
// Configure object in each package.

// This macro defines an external reference to a suitable Configure
// object; it should appear in the config_*.h file.  The config object
// will be named name.

#if defined(WIN32_VC) && !defined(CPPPARSER)

#define ConfigureDecl(name, expcl, exptp) \
  class expcl ConfigureGetConfig_ ## name { \
  public: \
    static const char *get_name(); \
    static void config_func(); \
    static bool _flag; \
  }; \
  exptp template class expcl Config::Config<ConfigureGetConfig_ ## name>; \
  extern expcl Config::Config<ConfigureGetConfig_ ## name> name;

#else // WIN32_VC

#define ConfigureDecl(name, expcl, exptp) \
  class expcl ConfigureGetConfig_ ## name { \
  public: \
    static const char *get_name(); \
    static void config_func(); \
    static bool _flag; \
  }; \
  extern expcl Config::Config<ConfigureGetConfig_ ## name> name;

#endif  // WIN32_VC

// This macro defines the actual declaration of the Configure object
// defined above; it should appear in the config_*.C file.

#if defined(PENV_OSX)
#define ConfigureDef(name) \
  Config::Config<ConfigureGetConfig_ ## name> name; \
  bool ConfigureGetConfig_ ## name::_flag = false; \
  int FILE_SYM_NAME = 0; \
  const char *ConfigureGetConfig_ ## name:: \
  get_name() { \
    return #name; \
  }
#else  /* PENV_OSX */
#define ConfigureDef(name) \
  Config::Config<ConfigureGetConfig_ ## name> name; \
  bool ConfigureGetConfig_ ## name::_flag = false; \
  const char *ConfigureGetConfig_ ## name:: \
  get_name() { \
    return #name; \
  }
#endif /* PENV_OSX */

// This macro can be used in lieu of the above two when the Configure
// object does not need to be visible outside of the current C file.

#if defined(PENV_OSX)
#define Configure(name) \
  class ConfigureGetConfig_ ## name { \
  public: \
    static const char *get_name(); \
    static void config_func(); \
    static bool _flag; \
  }; \
  static Config::Config<ConfigureGetConfig_ ## name> name; \
  bool ConfigureGetConfig_ ## name::_flag = false; \
  int FILE_SYM_NAME = 0; \
  const char *ConfigureGetConfig_ ## name:: \
  get_name() { \
    return #name; \
  }
#else  /* PENV_OSX */
#define Configure(name) \
  class ConfigureGetConfig_ ## name { \
  public: \
    static const char *get_name(); \
    static void config_func(); \
    static bool _flag; \
  }; \
  static Config::Config<ConfigureGetConfig_ ## name> name; \
  bool ConfigureGetConfig_ ## name::_flag = false; \
  const char *ConfigureGetConfig_ ## name:: \
  get_name() { \
    return #name; \
  }
#endif /* PENV_OSX */

// This one defines a block of code that will be executed at static
// init time.  It must always be defined (in the C file), even if no
// code is to be executed.

#define ConfigureFn(name) \
  void ConfigureGetConfig_ ## name::config_func()

#if defined(PENV_OSX)
// This is a hack to provide a unique locatable symbol in each lib.
#define ConfigureLibSym \
  char LIB_SYMBOL_NAME[] = ALL_FILE_SYMS_NAME ;
#else
#define ConfigureLibSym
#endif /* PENV_OSX */

#endif /* __CONFIG_H__ */
