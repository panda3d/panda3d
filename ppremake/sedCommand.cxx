// Filename: sedCommand.cxx
// Created by:  drose (24Oct00)
// 
////////////////////////////////////////////////////////////////////

#include "sedCommand.h"
#include "sedAddress.h"
#include "sedContext.h"
#include "sedScript.h"

////////////////////////////////////////////////////////////////////
//     Function: SedCommand::Constructor
//       Access: Public
//  Description: 
////////////////////////////////////////////////////////////////////
SedCommand::
SedCommand() {
  _addr1 = (SedAddress *)NULL;
  _addr2 = (SedAddress *)NULL;
  _command = '\0';
  _flags = 0;
  _active = false;
}

////////////////////////////////////////////////////////////////////
//     Function: SedCommand::Destructor
//       Access: Public
//  Description: 
////////////////////////////////////////////////////////////////////
SedCommand::
~SedCommand() {
  if (_addr1 != (SedAddress *)NULL) {
    delete _addr1;
  }
  if (_addr2 != (SedAddress *)NULL) {
    delete _addr2;
  }
  if ((_flags & F_have_re) != 0) {
    regfree(&_re);
  }
}

////////////////////////////////////////////////////////////////////
//     Function: SedCommand::parse_command
//       Access: Public
//  Description: Scans the indicated string at the given character
//               position for a legal command.  If a legal command is
//               found, stores it and increments p to the first
//               non-whitespace character after the command, returning
//               true.  Otherwise, returns false.
////////////////////////////////////////////////////////////////////
bool SedCommand::
parse_command(const string &line, size_t &p) {
  // First, skip initial whitespace.
  while (p < line.length() && isspace(line[p])) {
    p++;
  }

  // Now, check for an address.
  if (p < line.length() && 
      (isdigit(line[p]) || line[p] == '/' || line[p] == '\\')) {
    _addr1 = new SedAddress;
    if (!_addr1->parse_address(line, p)) {
      return false;
    }

    if (p < line.length() && line[p] == ',') {
      // Another address.

      // Skip the comma and more whitespace.
      p++;
      while (p < line.length() && isspace(line[p])) {
        p++;
      }

      _addr2 = new SedAddress;
      if (!_addr2->parse_address(line, p)) {
        return false;
      }
    }
  }

  if (p >= line.length()) {
    // It's a null command, which is acceptable; ignore it.
    return true;
  }

  _command = line[p];

  // Skip more whitespace after the command letter.
  p++;
  while (p < line.length() && isspace(line[p])) {
    p++;
  }

  // At the moment, we only accept a small subset of sed commands.  We
  // can add more later as we see the need.
  switch (_command) {
  case 'd':
    // No arguments.
    return true;

  case 's':
    // /regexp/repl/flags
    return parse_s_params(line, p);

  default:
    cerr << "Unknown command: " << _command << "\n";
    return false;
  }
}

////////////////////////////////////////////////////////////////////
//     Function: SedCommand::run
//       Access: Public
//  Description: Runs the script command, modifying the context and/or
//               the script position as appropriate.
////////////////////////////////////////////////////////////////////
void SedCommand::
run(SedScript &script, SedContext &context) {
  // First, see if this command matches the pattern space.
  bool matches = false;

  if (_addr1 != (SedAddress *)NULL && _addr2 != (SedAddress *)NULL) {
    // If the user supplied two addresses, all lines inclusive between
    // the lines matched by the two addresses are considered matching.
    if (_active) {
      // We have previously matched _addr1.  Therefore this line is
      // in, but are the rest of the lines following this one?
      matches = true;
      if (_addr2->matches(context)) {
        // If this line matches addr2, that's the end of our range for
        // next time.
        _active = false;
      }
    } else {
      // We have not yet matched _addr1.  This line and subsequent
      // lines are in only if we match now.
      if (_addr1->matches(context)) {
        matches = true;
        if (!_addr2->precedes(context)) {
          _active = true;
        }
      }
    }

  } else if (_addr1 != (SedAddress *)NULL) {
    // If the user supplied only one address, only those lines that
    // exactly match the address are considered matching.
    matches = _addr1->matches(context);

  } else {
    // If the user supplied no addresses, all lines are considered
    // matching.
    matches = true;
  }

  if (matches) {
    do_command(script, context);
  }
}

////////////////////////////////////////////////////////////////////
//     Function: SedCommand::parse_s_params
//       Access: Private
//  Description: Parses the /regexp/replacement/flags parameters that
//               follow the 's' command.
////////////////////////////////////////////////////////////////////
bool SedCommand::
parse_s_params(const string &line, size_t &p) {
  size_t p0 = p;
  char delimiter = line[p];
  p++;
  if (p < line.length() && delimiter == '\\') {
    // A backslash might escape the opening character.
    delimiter = line[p];
    p++;
  }
  
  size_t begin = p;
  while (p < line.length() && line[p] != delimiter) {
    if (line[p] == '\\') {
      p++;
      // A backslash could escape the closing character.
    }
    p++;
  }
  
  if (p >= line.length()) {
    cerr << "Could not find terminating character '" << delimiter
         << "' in regular expression: " << line.substr(p0) << "\n";
    return false;
  }

  string re = line.substr(begin, p - begin);
  p++;

  int error = regcomp(&_re, re.c_str(), 0);
  if (error != 0) {
    static const int errbuf_size = 512;
    char errbuf[errbuf_size];
    regerror(error, &_re, errbuf, errbuf_size);
    
    cerr << "Invalid regular expression: " << re << "\n"
         << errbuf << "\n";
    return false;
  }
  _flags |= F_have_re;

  // Get the replacement string.
  begin = p;
  while (p < line.length() && line[p] != delimiter) {
    if (line[p] == '\\') {
      p++;
      // A backslash could escape the closing character.
    }
    p++;
  }

  if (p >= line.length()) {
    cerr << "Could not find terminating character '" << delimiter
         << "' in replacement string: " << line.substr(p0) << "\n";
    return false;
  }

  _string2 = line.substr(begin, p - begin);

  // Skip the final delimiter.
  p++;
  if (p < line.length() && line[p] == 'g') {
    // Global flag.
    p++;
    _flags |= F_g;
  }

  // Skip any more whitespace after the parameters.
  while (p < line.length() && isspace(line[p])) {
    p++;
  }

  return true;
}

////////////////////////////////////////////////////////////////////
//     Function: SedCommand::do_command
//       Access: Private
//  Description: Actually invokes the command, once it has been
//               determined that the command applied to the current
//               pattern space.
////////////////////////////////////////////////////////////////////
void SedCommand::
do_command(SedScript &script, SedContext &context) {
  switch (_command) {
  case '\0':
    // Null command.
    return;

  case 'd':
    // Delete line.
    context._deleted = true;
    script._next_command = script._commands.end();
    return;

  case 's':
    // Substitute.
    do_s_command(context);
    return;
  }

  cerr << "Undefined command: " << _command << "\n";
}

////////////////////////////////////////////////////////////////////
//     Function: SedCommand::do_s_command
//       Access: Private
//  Description: Invokes the s command, which performs a
//               pattern/replacement substitution.
////////////////////////////////////////////////////////////////////
void SedCommand::
do_s_command(SedContext &context) {
  size_t nmatch = _re.re_nsub + 1;
  regmatch_t *pmatch = new regmatch_t[nmatch];

  string result;
  const char *str = context._pattern_space.c_str();
  int error = regexec(&_re, str, nmatch, pmatch, 0);  
  while (error == 0) {
    // Here's a match.  Determine the replacement.
    string repl;

    size_t p = 0;
    while (p < _string2.length()) {
      if (_string2[p] == '\\') {
        p++;
        if (p < _string2.length()) {
          if (isdigit(_string2[p])) {
            // Here's a subexpression reference.
            const char *numstr = _string2.c_str() + p;
            char *numend;
            int ref = strtol(numstr, &numend, 10);
            p += (numend - numstr);
            if (ref <= 0 || ref >= (int)nmatch) {
              cerr << "Invalid subexpression number: " << ref << "\n";
            } else {
              repl += string(str + pmatch[ref].rm_so,
                             pmatch[ref].rm_eo - pmatch[ref].rm_so);
            }
          } else {
            // Here's an escaped character.
            repl += _string2[p];
            p++;
          }
        }
      } else {
        // Here's a normal character.
        repl += _string2[p];
        p++;
      }
    }

    // Store the result so far.
    result += string(str, pmatch[0].rm_so);
    result += repl;
    str += pmatch[0].rm_eo;

    if ((_flags & F_g) == 0) {
      // If we don't have the global flag set, stop after the first iteration.
      result += str;
      context._pattern_space = result;
      delete[] pmatch;
      return;
    }

    error = regexec(&_re, str, nmatch, pmatch, 0);  
  }

  // All done.
  result += str;
  context._pattern_space = result;
  delete[] pmatch;
}
