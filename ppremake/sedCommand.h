// Filename: sedCommand.h
// Created by:  drose (24Oct00)
// 
////////////////////////////////////////////////////////////////////

#ifndef SEDCOMMAND_H
#define SEDCOMMAND_H

#include "ppremake.h"

#include <sys/types.h>

#ifdef HAVE_REGEX_H
#include <regex.h>
#else
#include "gnu_regex.h"
#endif


class SedScript;
class SedContext;
class SedAddress;

///////////////////////////////////////////////////////////////////
//       Class : SedCommand
// Description : This represents a single command (of several
//               possible, separated by semicolons) to a SedProgram.
////////////////////////////////////////////////////////////////////
class SedCommand {
public:
  SedCommand();
  ~SedCommand();

  bool parse_command(const string &line, size_t &p);

  void run(SedScript &script, SedContext &context);

private:
  bool parse_s_params(const string &line, size_t &p);
  void do_command(SedScript &script, SedContext &context);
  void do_s_command(SedContext &context);

  SedAddress *_addr1;
  SedAddress *_addr2;
  char _command;

  string _text;

  regex_t _re;
  string _string1;
  string _string2;

  enum Flags {
    F_have_re  = 0x001,
    F_g        = 0x002,
  };

  int _flags;

  bool _active;
};

#endif
