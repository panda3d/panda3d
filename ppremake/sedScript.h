// Filename: sedScript.h
// Created by:  drose (24Oct00)
// 
////////////////////////////////////////////////////////////////////

#ifndef SEDSCRIPT_H
#define SEDSCRIPT_H

#include "ppremake.h"

#include <vector>

class SedCommand;
class SedContext;

///////////////////////////////////////////////////////////////////
//       Class : SedScript
// Description : This is a complete sed script: a linear list of
//               commands that are to be applied for each line read
//               from input.
////////////////////////////////////////////////////////////////////
class SedScript {
public:
  SedScript();
  ~SedScript();

  bool add_line(const string &line);

  bool run(SedContext &context);

public:
  bool _quit;

  typedef vector<SedCommand *> Commands;
  Commands _commands;
  Commands::const_iterator _next_command;
};

#endif
