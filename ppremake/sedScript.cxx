// Filename: sedScript.cxx
// Created by:  drose (24Oct00)
// 
////////////////////////////////////////////////////////////////////

#include "sedScript.h"
#include "sedCommand.h"
#include "sedContext.h"


////////////////////////////////////////////////////////////////////
//     Function: SedScript::Constructor
//       Access: Public
//  Description: 
////////////////////////////////////////////////////////////////////
SedScript::
SedScript() {
  _quit = false;
}

////////////////////////////////////////////////////////////////////
//     Function: SedScript::Destructor
//       Access: Public
//  Description: 
////////////////////////////////////////////////////////////////////
SedScript::
~SedScript() {
  Commands::iterator ci;
  for (ci = _commands.begin(); ci != _commands.end(); ++ci) {
    delete (*ci);
  }
}

////////////////////////////////////////////////////////////////////
//     Function: SedScript::add_line
//       Access: Public
//  Description: Adds the indicated script line to the script.
//               Returns true if it is a valid line, false if there is
//               an error.
////////////////////////////////////////////////////////////////////
bool SedScript::
add_line(const string &line) {
  size_t p = 0;
  SedCommand *command = new SedCommand;
  if (!command->parse_command(line, p)) {
    // That's an invalid command.
    delete command;
    return false;
  }
  _commands.push_back(command);

  while (p < line.length()) {
    // There's more to the line.
    if (line[p] != ';') {
      // But it's an error.
      cerr << "Invalid character at: " << line.substr(p) << "\n";
      return false;
    }
    p++;

    command = new SedCommand;
    if (!command->parse_command(line, p)) {
      // That's an invalid command.
      delete command;
      return false;
    }
    _commands.push_back(command);
  }

  // Everything parsed ok.
  return true;
}

////////////////////////////////////////////////////////////////////
//     Function: SedScript::run
//       Access: Public
//  Description: Runs the script, modifying the context as
//               appropriate.  Returns true if the process should
//               continue with the next line, or false if we have quit
//               and we should terminate.
////////////////////////////////////////////////////////////////////
bool SedScript::
run(SedContext &context) {
  context._deleted = false;
  _next_command = _commands.begin();
  while (!_quit && _next_command != _commands.end()) {
    SedCommand *command = (*_next_command);
    ++_next_command;
    command->run(*this, context);
  }

  if (!context._deleted) {
    context._out << context._pattern_space << "\n";
  }

  return !_quit;
}
