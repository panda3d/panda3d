// Filename: configTable.cxx
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

#include "dconfig.h"
#include "configTable.h"
#include "dSearchPath.h"
#include "executionEnvironment.h"
#include "config_dconfig.h"
#include "pfstream.h"
#include "serialization.h"

#ifdef PENV_PS2

#include <string.h>
#include <sifdev.h>
#include <eekernel.h>

#endif // PENV_PS2

//#define DISABLE_CONFIG

using namespace Config;

ConfigTable* ConfigTable::_instance = (ConfigTable*)0L;

void ConfigTable::CropString(ConfigString& S) {
  size_t i = S.find_first_not_of(" \t\r\f\n");
  if (i != ConfigString::npos) {
    size_t j = S.find_last_not_of(" \t\r\f\n");
    if (j != ConfigString::npos)
      S = S.substr(i, j-i+1);
    else
      S = S.substr(i, ConfigString::npos);
  } else
    S.erase(0, ConfigString::npos);
}

void ConfigTable::DeComment(ConfigString& S) {
  // If the comment delimiter appears in the line followed by
  // whitespace, strip that part of the line out.

  size_t i = S.find(configcmt);
  while (i != ConfigString::npos) {
    if (i + configcmt.length() < S.length() && 
        isspace(S[i + configcmt.length()])) {
      // Here's a comment.
      S.erase(i, ConfigString::npos);
      return;
    }

    i = S.find(configcmt, i + 1);
  }
}

bool ConfigTable::IsComment(const ConfigString& S) {
  // Returns true if the line begins with the comment delimiter,
  // whether or not the delimiter is followed by whitespace.
  return (S.substr(0, configcmt.length()) == configcmt);
}

void ConfigTable::UpCase(ConfigString& S) {
   for (ConfigString::iterator i=S.begin(); i!=S.end(); ++i)
      (*i) = toupper(*i);
}

ConfigString ConfigTable::NextWord(const ConfigString& S) {
  int i(S.find_first_of(" \t\r\f\n"));
  return S.substr(0, i);
}

ConfigString ConfigTable::PopNextWord(ConfigString& S) {
  int i(S.find_first_of(" \t\r\f\n"));
  ConfigString ret(S.substr(0, i));
  S.erase(0, i);
  CropString(S);
  return ret;
}

void ConfigTable::ParseConfigFile(istream& is, const ConfigString& Filename) {
   ConfigString line;

   while (!is.eof() && !is.fail()) {
      std::getline(is, line);
      if (microconfig_cat->is_spam())
         microconfig_cat->spam() << "read from " << Filename << ": '" << line
                                 << "'" << endl;
      DeComment(line);
      CropString(line);
      if (microconfig_cat->is_spam())
         microconfig_cat->spam() << "cropped line to: '" << line << "'"
                                 << endl;
      if (!IsComment(line)) {
         ConfigString protosym(PopNextWord(line));
         if (microconfig_cat->is_spam())
            microconfig_cat->spam() << "protosym is '" << protosym
                                    << "' with value of '" << line << "'"
                                    << endl;
         size_t i(protosym.find("."));
         if (i == ConfigString::npos) {
            if (microconfig_cat->is_spam())
               microconfig_cat->spam() << "this is an unqualified symbol"
                                       << endl;
            unqualified[protosym].push_back(SymEnt(SymEnt::ConfigFile, line, Filename));
         } else {
           ConfigString scope(protosym.substr(0, i));
           ConfigString sym(protosym.substr(i+1, ConfigString::npos));
           if (microconfig_cat->is_spam())
             microconfig_cat->spam() << "this is a qualified symbol."
                                     << " scope '" << scope
                                     << "', symbol '" << sym << "'" << endl;
           (qualified[scope])[sym].push_back(SymEnt(SymEnt::ConfigFile, line, Filename));
         }
      } else if (microconfig_cat->is_spam())
        microconfig_cat->spam() << "line is detected as a comment" << endl;
   }
}

void ConfigTable::ReadConfigFile() {
  // The configpath variable lists the environment variables that
  // themselves should be considered to contain search paths for
  // Configrc files.  This is one level of indirection from the
  // intuitive definition.

  DSearchPath config_search;

  // The configdir variable gets priority--it names a directory that
  // ends up first on the search path.
  if (!configdir.empty()) {
    config_search.append_directory(Filename::from_os_specific(configdir));
  }

  // Then all of the directories named (indirectly) by configpath.
  // This variable actually names a space-delimited list of
  // environment variable names, each of which contains a space- or
  // colon-delimited search path.
  while (!configpath.empty()) {
    int i = configpath.find_first_of(" ");
    ConfigString stmp = configpath.substr(0, i);
    if (ExecutionEnvironment::has_environment_variable(stmp)) {
      string path = ExecutionEnvironment::get_environment_variable(stmp);

      size_t p = 0;
      while (p < path.length()) {
        size_t q = path.find_first_of(" :", p);
        if (q == string::npos) {
          config_search.append_directory(Filename::from_os_specific(path.substr(p)));
          break;
        }
        if (q != p) {
          config_search.append_directory(Filename::from_os_specific(path.substr(p, q - p)));
        }
        p = q + 1;
      }
    }
    configpath.erase(0, i);
    CropString(configpath);
  }

  if (microconfig_cat->is_spam()) {
    microconfig_cat->spam()
      << "search path from configdir and configpath is: " 
      << config_search << endl;
  }

  DSearchPath::Results config_files;

  if (!configsuffix.empty()) {
    if (configsuffix == "*") {
      // A configsuffix of "*" is a special case: this means to find
      // all files that begin with configname.  We don't do full
      // globbing, though.  We also make a special case for files
      // ending in ~, which we always ignore (these are usually just
      // backup files).
      if (microconfig_cat->is_spam())
        microconfig_cat->spam() << "searching for files matching: "
                                << (configname + configsuffix) << endl;
      for (int di = 0; di < config_search.get_num_directories(); di++) {
        const Filename &directory = config_search.get_directory(di);
        vector_string files;
        directory.scan_directory(files);
        // Scan the files in reverse order to match Configrc overwrite
        // rules, so that the alphabetically earliest file has
        // precedence.
        for (vector_string::reverse_iterator fi = files.rbegin();
             fi != files.rend();
             ++fi) {
          if ((*fi).substr(0, configname.length()) == configname &&
              (*fi).substr((*fi).length() - 1) != string("~")) {
            Filename file(directory, (*fi));
            config_files.add_file(file);
          }
        }
      }

    } else {
      if (microconfig_cat->is_spam())
        microconfig_cat->spam() << "aggregate config name is: "
                                << (configname + configsuffix) << endl;
      config_search.find_all_files(configname + configsuffix, config_files);
    }
  } else {
    if (microconfig_cat->is_spam())
      microconfig_cat->spam() << "searching for '" << configname << "'"
                              << endl;
    config_search.find_all_files(configname, config_files);
  }

  if (microconfig_cat->is_spam())
    microconfig_cat->spam() << "found " << config_files.get_num_files()
                            << " files" << endl;

  int num_config_files = config_files.get_num_files();
  for (int i = num_config_files - 1; i >= 0; i--) {
    Filename config_file = config_files.get_file(i);

    if (microconfig_cat->is_spam())
      microconfig_cat->spam() << "examining file '" << config_file << "'"
                               << endl;

    if (!config_file.is_regular_file()) {
      if (microconfig_cat->is_spam()) {
        microconfig_cat->spam()
          << "file is not a regular file, ignoring.\n";
      }

    } else if (configexe && config_file.is_executable()) {
      ConfigString line = config_file.to_os_specific() + " " + configargs;
      if (microconfig_cat->is_spam())
        microconfig_cat->spam() << "file is executable, running '"
                                 << line << "'" << endl;
      IPipeStream ifs(line);
      ParseConfigFile(ifs, config_file);
    } else {
      if (microconfig_cat->is_spam())
        microconfig_cat->spam()
          << "file is not executable, reading normally" << endl;

#ifdef PENV_PS2
      ConfigString line = PS2_FILE_PREFIX + convert_pathname(config_file);

      int fd = sceOpen((char *) line.c_str(), SCE_RDONLY);
      char line_buffer[2048];

      memset(line_buffer, 0, 2048);

      ConfigString file_buffer;

      while (sceRead(fd, line_buffer, 2048) > 0)
      {
        file_buffer += line_buffer;
        memset(line_buffer, 0, 2048);
      }

      sceClose(fd);

      istrstream ifs(file_buffer.c_str());
#else
      ifstream ifs(config_file.to_os_specific().c_str());
#endif
      ParseConfigFile(ifs, config_file);
    }
  }
}

void ConfigTable::ParseCommandEnv(ConfigString& S, const ConfigString& sym)
{
   if (microconfig_cat->is_spam())
      microconfig_cat->spam() << "value of '" << sym << "' is '" << S << "'"
                               << endl;
   while (!S.empty()) {
      ConfigString protosym(PopNextWord(S));
      bool ok = false;
      bool state = false;
      if (protosym[0] == '-')
         ok = true;
      else if (protosym[0] == '+') {
         ok = true;
         state = true;
      }
      if (ok) {
         protosym.erase(0, 1);
         CropString(protosym);
         size_t i(protosym.find("."));
         if (i == ConfigString::npos) {
            unqualified[protosym].push_back(SymEnt(SymEnt::CommandEnv,
                                                   NextWord(S), sym, state));
            if (microconfig_cat->is_spam())
               microconfig_cat->spam() << "unqualified symbol '" << protosym
                                       << "' with value '" << NextWord(S)
                                       << "'" << endl;
         } else {
            ConfigString scope(protosym.substr(0, i));
            ConfigString sym(protosym.substr(i+1, ConfigString::npos));
            (qualified[scope])[sym].push_back(SymEnt(SymEnt::CommandEnv,
                                                     NextWord(S), sym, state));
            if (microconfig_cat->is_spam())
               microconfig_cat->spam() << "qualified symbol '" << sym
                                       << "' in scope '" << scope
                                       << "' and value '" << NextWord(S)
                                       << "'" << endl;
         }
      } else if (microconfig_cat->is_spam())
         microconfig_cat->spam() << "'" << protosym
                                 << "' was not recognized as an option"
                                 << endl;
   }
}

void ConfigTable::ParseArgs() {
   int n = 0;
   int num_args = ExecutionEnvironment::get_num_args();

   while ( n < num_args) {
     bool ok = false;
     bool state = false;
     ConfigString line(ExecutionEnvironment::get_arg(n));
     CropString(line);
     if (line[0] == '-')
       ok = true;
     else if (line[0] == '+') {
       ok = true;
       state = true;
     }
     if (ok) {
       line.erase(0, 1);
       CropString(line);
       size_t i(line.find("."));
       ConfigString aparam;
       if (n + 1 < num_args) {
         aparam = ExecutionEnvironment::get_arg(n + 1);
       }

       if (i == ConfigString::npos) {
         unqualified[line].push_back(SymEnt(SymEnt::Commandline,
                                            aparam, "", state));
         if (microconfig_cat->is_spam())
           microconfig_cat->spam() << "unqualified symbol '" << line
                                   << "' with value '" << aparam
                                   << "'" << endl;
       } else {
         ConfigString scope(line.substr(0, i));
         ConfigString sym(line.substr(i+1, ConfigString::npos));
         (qualified[scope])[sym].push_back(SymEnt(SymEnt::Commandline,
                                                  aparam, "", state));
         if (microconfig_cat->is_spam())
           microconfig_cat->spam() << "qualified symbol '" << sym
                                   << "' with scope '" << scope
                                   << "' and value '" << aparam
                                   << "'" << endl;
       }
     } else if (microconfig_cat->is_spam()) {
       microconfig_cat->spam() << "argument #" << n << " ('" << line
                               << "') is not recognized as an option"
                               << endl;
     }
     ++n;
   }
}

void ConfigTable::ConfigDirDefault() {
  // The configdir default comes from $CONFIGRC_DIR, or from the
  // compiled in DEFAULT_CONFIGRC_DIR if that's unspecified.
  configdir = ExecutionEnvironment::get_environment_variable("CONFIGRC_DIR");
  if (configdir.empty()) {
    configdir = DEFAULT_CONFIGRC_DIR;
  }
}

void ConfigTable::MicroConfig() {
/*
#ifndef NDEBUG
   NotifySeverity mcs = microconfig_cat->get_severity();
   microconfig_cat->set_severity(NS_spam);

   NotifySeverity cs = config_cat->get_severity();
   config_cat->set_severity(NS_spam);
#else * NDEBUG *
*/
  //   NotifySeverity mcs = microconfig_cat->get_severity();
   microconfig_cat->set_severity(NS_info);

   //   NotifySeverity cs = dconfig_cat->get_severity();
   dconfig_cat->set_severity(NS_info);
/*
#endif * NDEBUG *
*/
   string cc = ExecutionEnvironment::get_environment_variable("CONFIG_CONFIG");
   if (microconfig_cat->is_spam()) {
     microconfig_cat->spam() << "CONFIG_CONFIG = '" << cc << "'" << endl;
   }
   bool cdbg = false;
   bool cexe = false;
   bool psep = false;
   bool fsep = false;
   bool cname = false;
   bool csuff = false;
   bool cargs = false;
   bool cpath = false;
   bool cdir = false;
   bool ccmt = false;
   bool asuff = false;
   bool cstub = false;
   bool rdarg = false;
   bool rdenv = false;
   if (!cc.empty()) {
      ConfigString configconfig(cc);
      if (configconfig.length() > 1) {
         ConfigString assign = "=";
         ConfigString sep = configconfig.substr(0, 1);
         if (microconfig_cat->is_spam()) {
            microconfig_cat->spam() << "separator character is: '" << sep
                                    << "'" << endl;
         }
         typedef std::vector<ConfigString> strvec;
         strvec sv;
         size_t q = 1;
         size_t p = configconfig.find(sep, q);
         while (p != ConfigString::npos) {
           sv.push_back(configconfig.substr(q, p - q));
           q = p + 1;
           p = configconfig.find(sep, q);
         }
         if (q + 1 < configconfig.size()) {
           sv.push_back(configconfig.substr(q));
         }
         
         if (microconfig_cat->is_spam())
            microconfig_cat->spam()
               << "extracted vector of microconfig options" << endl;
         for (strvec::iterator i=sv.begin(); i!=sv.end(); ++i) {
            if (microconfig_cat->is_spam())
               microconfig_cat->spam() << "parsing microconfig option '"
                                       << *i << "'" << endl;
            if ((*i).length() == 1) {
              // new assignment character
              assign += *i;
              continue;
            }
            size_t j = (*i).find_first_of(assign);
            if (j != ConfigString::npos) {
               ConfigString tok = (*i).substr(0, j);
               ConfigString rest = (*i).substr(j+1, ConfigString::npos);
               if (microconfig_cat->is_spam())
                  microconfig_cat->spam() << "split microconfig option into '"
                                          << tok << "' and '" << rest << "'"
                                          << endl;
               if (tok == "pathsep") {
                  pathsep = rest;
                  psep = true;
                  if (microconfig_cat->is_spam())
                     microconfig_cat->spam()
                        << "got a microconfig pathsep directive, "
                        << "setting the path separator to '" << pathsep << "'"
                        << endl;
               } else if (tok == "filesep") {
                  filesep = rest;
                  fsep = true;
                  if (microconfig_cat->is_spam())
                     microconfig_cat->spam()
                        << "got a microconfig filesep directive, "
                        << "setting the file separator to '" << filesep << "'"
                        << endl;
               } else if (tok == "configname") {
                  configname = rest;
                  cname = true;
                  if (microconfig_cat->is_spam())
                     microconfig_cat->spam()
                        << "got a microconfig configname directive, "
                        << "setting the configfile name to '" << configname
                        << "'" << endl;
               } else if (tok == "configsuffix") {
                  configsuffix = rest;
                  csuff = true;
                  if (microconfig_cat->is_spam())
                     microconfig_cat->spam()
                        << "got a microconfig configsuffix directive, "
                        << "setting the config file suffix to '"
                        << configsuffix << "'"
                        << endl;
               } else if (tok == "configargs") {
                  configargs = rest;
                  cargs = true;
                  if (microconfig_cat->is_spam())
                     microconfig_cat->spam()
                        << "got a microconfig configargs directive, "
                        << "setting the config file args to '"
                        << configargs << "'"
                        << endl;
               } else if (tok == "configpath") {
                  if (cpath) {
                    configpath += " " + rest;
                    if (microconfig_cat->is_spam())
                      microconfig_cat->spam()
                        << "got a microconfig configpath directive, "
                        << "adding '" << rest << "' to the configpath (now '"
                        << configpath << "')"
                        << endl;
                  } else {
                    configpath = rest;
                    if (microconfig_cat->is_spam())
                      microconfig_cat->spam()
                        << "got a microconfig configpath directive, "
                        << "setting the configpath to '" << configpath << "'"
                        << endl;
                  }
                  cpath = true;
               } else if (tok == "configdir") {
                 configdir = rest;
                 if (microconfig_cat->is_spam())
                   microconfig_cat->spam()
                     << "got a microconfig configdir directive, "
                     << "setting the configdir to '" << configdir << "'"
                     << endl;
                  cdir = true;
               } else if (tok == "configcmt") {
                  configcmt = rest;
                  ccmt = true;
                  if (microconfig_cat->is_spam())
                     microconfig_cat->spam()
                        << "got a microconfig configcmt directive, "
                        << "setting the config comment to '" << configcmt
                        << "'" << endl;
               } else if (tok == "argsuffix") {
                  argsuffix = rest;
                  asuff = true;
                  if (microconfig_cat->is_spam())
                     microconfig_cat->spam()
                        << "got a microconfig argsuffix directive, "
                        << "setting the argument environment suffix to '"
                        << argsuffix << "'" << endl;
               } else if (tok == "commandstub") {
                  commandstub = rest;
                  cstub = true;
                  if (microconfig_cat->is_spam())
                     microconfig_cat->spam()
                        << "got a microconfig commandstub directive, "
                        << "setting the command environment stub "
                        << "to '" << commandstub << "'" << endl;
               } else if (tok == "configdbg") {
                  configdbg = TrueOrFalse(rest);
                  cdbg = true;
                  if (configdbg) {
                     microconfig_cat->set_severity(NS_spam);
                     dconfig_cat->set_severity(NS_spam);
                  } else {
                     microconfig_cat->set_severity(NS_info);
                     dconfig_cat->set_severity(NS_info);
                  }
                  if (microconfig_cat->is_spam())
                     microconfig_cat->spam()
                        << "got a microconfig configdbg directive, "
                        << "setting the config spam state to " << configdbg
                        << endl;
               } else if (tok == "configexe") {
                  configexe = TrueOrFalse(rest);
                  cexe = true;
                  if (configexe) {
                     microconfig_cat->set_severity(NS_spam);
                     dconfig_cat->set_severity(NS_spam);
                  } else {
                     microconfig_cat->set_severity(NS_info);
                     dconfig_cat->set_severity(NS_info);
                  }
                  if (microconfig_cat->is_spam())
                     microconfig_cat->spam()
                        << "got a microconfig configexe directive, "
                        << "setting the config spam state to " << configexe
                        << endl;
               } else if (tok == "readargs") {
                  readargs = TrueOrFalse(rest);
                  rdarg = true;
                  if (microconfig_cat->is_spam())
                     microconfig_cat->spam()
                        << "got a microconfig readargs directive, "
                        << (readargs?"will":"will not")
                        << " read from the commandline." << endl;
               } else if (tok == "readenv") {
                  readenvs = TrueOrFalse(rest);
                  rdenv = true;
                  if (microconfig_cat->is_spam())
                     microconfig_cat->spam()
                        << "got a microconfig readenv directive, "
                        << (readargs?"will":"will not")
                        << " read the environment." << endl;
               }
            } else if (microconfig_cat->is_spam())
               microconfig_cat->spam()
                  << "no '=' in microconfig option, ignoring it" << endl;
         }
      } else if (microconfig_cat->is_spam())
         microconfig_cat->spam()
            << "CONFIG_CONFIG contains only a single character" << endl;
   } else if (microconfig_cat->is_spam())
      microconfig_cat->spam() << "CONFIG_CONFIG is empty" << endl;
   if (!cdbg)
      ConfigDbgDefault();
   if (!cexe)
      ConfigExeDefault();
   if (!psep) {
      PathSepDefault();
      if (microconfig_cat->is_spam())
         microconfig_cat->spam() << "no microconfig for pathsep, "
                                  << "setting to default '" << pathsep << "'"
                                  << endl;
   }
   if (!fsep) {
      FileSepDefault();
      if (microconfig_cat->is_spam())
        microconfig_cat->spam() << "no microconfig for filesep, "
                                << "setting to default '" << filesep << "'"
                                << endl;
   }
   if (!cname) {
      ConfigNameDefault();
      if (microconfig_cat->is_spam())
        microconfig_cat->spam() << "no microconfig for configname, "
                                << "setting to default '" << configname
                                << "'" << endl;
   }
   if (!csuff) {
      ConfigSuffixDefault();
      if (microconfig_cat->is_spam())
        microconfig_cat->spam() << "no microconfig for configsuffix, "
                                << "setting to default '" << configsuffix
                                << "'" << endl;
   }
   if (!cargs) {
      ConfigArgsDefault();
      if (microconfig_cat->is_spam())
        microconfig_cat->spam() << "no microconfig for configargs, "
                                << "setting to default '" << configargs
                                << "'" << endl;
   }
   if (!cpath) {
      ConfigPathDefault();
      if (microconfig_cat->is_spam())
        microconfig_cat->spam() << "no microconfig for configpath, "
                                << "setting to default '" << configpath
                                << "'" << endl;
   }
   if (!cdir) {
      ConfigDirDefault();
      if (microconfig_cat->is_spam())
        microconfig_cat->spam() << "no microconfig for configdir, "
                                << "setting to default '" << configdir
                                << "'" << endl;
   }
   if (!ccmt) {
      ConfigCmtDefault();
      if (microconfig_cat->is_spam())
        microconfig_cat->spam() << "no microconfig for configcmt, "
                                << "setting to default '" << configcmt
                                << "'" << endl;
   }
   if (!asuff) {
      ArgSuffixDefault();
      if (microconfig_cat->is_spam())
        microconfig_cat->spam() << "no microconfig for argsuffix, "
                                << "setting to default '" << argsuffix
                                << "'" << endl;
   }
   if (!cstub) {
      CommandStubDefault();
      if (microconfig_cat->is_spam())
        microconfig_cat->spam() << "no microconfig for commandstub, "
                                << "setting to default '" << commandstub
                                << "'" << endl;
   }
   if (!rdarg) {
      ReadArgsDefault();
      if (microconfig_cat->is_spam())
        microconfig_cat->spam() << "no microconfig for readargs, "
                                << "setting to default: "
                                << (readargs?"true":"false") << endl;
   }
   if (!rdenv) {
      ReadEnvsDefault();
      if (microconfig_cat->is_spam())
        microconfig_cat->spam() << "no microconfig for readenv, "
                                << "setting to default: "
                                << (readargs?"true":"false") << endl;
   }
}

void ConfigTable::GetData() {
  MicroConfig();
#ifndef DISABLE_CONFIG
  ReadConfigFile();
  if (readenvs) {
    ConfigString comarg = commandstub + argsuffix;
    if (microconfig_cat->is_spam())
      microconfig_cat->spam() << "comarg is '" << comarg << "'"
                              << endl;
    if (ExecutionEnvironment::has_environment_variable(comarg)) {
      ConfigString env = ExecutionEnvironment::get_environment_variable(comarg);
      ParseCommandEnv(env, comarg);
    }
    ConfigString line = ExecutionEnvironment::get_binary_name() + argsuffix;
    UpCase(line);
    if (microconfig_cat->is_spam())
      microconfig_cat->spam() << "binarg is '" << line << "'"
                              << endl;
    if (ExecutionEnvironment::has_environment_variable(line)) {
      ConfigString env = ExecutionEnvironment::get_environment_variable(line);
      ParseCommandEnv(env, line);
    }
  }
  if (readargs)
    ParseArgs();
#endif  // DISABLE_CONFIG
}

ConfigTable* ConfigTable::Instance() {
  if (_instance == (ConfigTable*)0L) {
    _instance = new ConfigTable;
    _instance->GetData();
    _instance->_initializing = false;
    Notify::ptr()->config_initialized();
  }
  return _instance;
}

bool ConfigTable::AmInitializing() {
  return _initializing;
}

bool ConfigTable::TrueOrFalse(const ConfigString& in, bool def) {
  bool ret = def;
  ConfigString S = in;
  UpCase(S);
  if (S[0] == '#') {
    if (S[1] == 'F') {
      ret = false;
    } else if (S[1] == 'T') {
      ret = true;
    }
  } else if (S == "0") {
    ret = false;
  } else if (S == "1") {
    ret = true;
  } else if (S == "FALSE") {
    ret = false;
  } else if (S == "TRUE") {
    ret = true;
  }
  return ret;
}

bool ConfigTable::Defined(const ConfigString& sym,
                                 const ConfigString qual) {
#ifdef DISABLE_CONFIG
  return false;
#else
  if (qual.empty()) {
    return (unqualified.count(sym) != 0 ||
            ExecutionEnvironment::has_environment_variable(sym));

  } else {
    TableMap::const_iterator ti;
    ti = qualified.find(qual);
    if (ti != qualified.end()) {
      const SymbolTable &table = (*ti).second;
      if (table.count(sym) != 0) {
        return true;
      }
    }

    return ExecutionEnvironment::has_environment_variable(qual + "." + sym);
  }
#endif // DISABLE_CONFIG
}

ConfigTable::SymEnt ConfigTable::Get(const ConfigString& sym,
                                     const ConfigString qual) {
#ifndef DISABLE_CONFIG
  const ConfigTable::Symbol &symbol = GetSym(sym, qual);
  if (!symbol.empty()) {
    return symbol.back();
  }

  // No explicit config definition; fall back to the environment.
  string envvar = sym;
  if (!qual.empty()) {
    envvar = qual + "." + sym;
  }

  if (ExecutionEnvironment::has_environment_variable(sym)) {
    string def = ExecutionEnvironment::get_environment_variable(sym);
    return ConfigTable::SymEnt(ConfigTable::SymEnt::Environment, def);
  }
#endif // DISABLE_CONFIG

  // No definition for the variable.  Too bad for you.
  return ConfigTable::SymEnt();
}

const ConfigTable::Symbol& ConfigTable::GetSym(const ConfigString& sym,
                                               const ConfigString qual) {
  static ConfigTable::Symbol empty_symbol;

#ifndef DISABLE_CONFIG
  total_num_get++;
  if (qual.empty()) {
    SymbolTable::const_iterator si;
    si = unqualified.find(sym);
    if (si != unqualified.end()) {
      return (*si).second;
    }

  } else {
    TableMap::const_iterator ti;
    ti = qualified.find(qual);
    if (ti != qualified.end()) {
      const SymbolTable &table = (*ti).second;
      SymbolTable::const_iterator si;
      si = table.find(sym);
      if (si != table.end()) {
        return (*si).second;
      }
    }
  }
#endif // DISABLE_CONFIG

  return empty_symbol;
}
