// Filename: executionEnvironment.C
// Created by:  drose (15May00)
// 
////////////////////////////////////////////////////////////////////

#include "executionEnvironment.h"
#include <assert.h>

// We define the symbol PREREAD_ENVIRONMENT if we cannot rely on
// getenv() to read environment variables at static init time.  In
// this case, we must read all of the environment variables directly
// and cache them locally.

#if defined(PENV_LINUX)
#define PREREAD_ENVIRONMENT
#endif


// We define the symbol USE_ARGV if we have global variables named
// ARGC/ARGV that we can read at static init time to determine our
// command-line arguments.

#if defined(WIN32_VC)
  #define USE_ARGV
  #define ARGV __argv
  #define ARGC __argc

#elif defined(PENV_SGI)
  #define USE_ARGV
  extern char **__Argv;
  extern int __Argc;
  #define ARGV __Argv
  #define ARGC __Argc
#endif

// Linux with GNU libc does have global argv/argc variables, but we
// can't safely access them at stat init time--at least, not in libc5.
// (It does seem to work with glibc2, however.)

ExecutionEnvironment *ExecutionEnvironment::_global_ptr = NULL;


////////////////////////////////////////////////////////////////////
//     Function: ExecutionEnvironment::Constructor
//       Access: Private
//  Description: You shouldn't need to construct one of these; there's
//               only one and it constructs itself.
////////////////////////////////////////////////////////////////////
ExecutionEnvironment::
ExecutionEnvironment() {
  read_environment_variables();
  read_args();
}

////////////////////////////////////////////////////////////////////
//     Function: ExecutionEnvironment::ns_has_environment_variable
//       Access: Private
//  Description: Returns true if the indicated environment variable
//               is defined.  The nonstatic implementation.
////////////////////////////////////////////////////////////////////
bool ExecutionEnvironment::
ns_has_environment_variable(const string &var) const {
#ifdef PREREAD_ENVIRONMENT
  return _variables.count(var) != 0;
#else
  return getenv(var.c_str()) != (char *)NULL;
#endif
}

////////////////////////////////////////////////////////////////////
//     Function: ExecutionEnvironment::ns_get_environment_variable
//       Access: Private
//  Description: Returns the definition of the indicated environment
//               variable, or the empty string if the variable is
//               undefined.  The nonstatic implementation.
////////////////////////////////////////////////////////////////////
string ExecutionEnvironment::
ns_get_environment_variable(const string &var) const {
#ifdef PREREAD_ENVIRONMENT
  EnvironmentVariables::const_iterator evi;
  evi = _variables.find(var);
  if (evi != _variables.end()) {
    return (*evi).second;
  }
  return string();
#else
  const char *def = getenv(var.c_str());
  if (def != (char *)NULL) {
    return def;
  }
  return string();
#endif
}

////////////////////////////////////////////////////////////////////
//     Function: ExecutionEnvironment::ns_get_num_args
//       Access: Private
//  Description: Returns the number of command-line arguments
//               available, not counting arg 0, the binary name.  The
//               nonstatic implementation.
////////////////////////////////////////////////////////////////////
int ExecutionEnvironment::
ns_get_num_args() const {
  return _args.size();
}

////////////////////////////////////////////////////////////////////
//     Function: ExecutionEnvironment::ns_get_arg
//       Access: Private
//  Description: Returns the nth command-line argument.  The index n
//               must be in the range [0 .. get_num_args()).  The
//               first parameter, n == 0, is the first actual
//               parameter, not the binary name.  The nonstatic
//               implementation.
////////////////////////////////////////////////////////////////////
string ExecutionEnvironment::
ns_get_arg(int n) const {
  assert(n >= 0 && n < ns_get_num_args());
  return _args[n];
}
  
////////////////////////////////////////////////////////////////////
//     Function: ExecutionEnvironment::ns_get_binary_name
//       Access: Private
//  Description: Returns the name of the binary executable that
//               started this program, if it can be determined.  The
//               nonstatic implementation.
////////////////////////////////////////////////////////////////////
string ExecutionEnvironment::
ns_get_binary_name() const {
  if (_binary_name.empty()) {
    return "unknown";
  }
  return _binary_name;
}

////////////////////////////////////////////////////////////////////
//     Function: ExecutionEnvironment::get_ptr
//       Access: Private, Static
//  Description: Returns a static pointer that may be used to access
//               the global ExecutionEnvironment object.
////////////////////////////////////////////////////////////////////
ExecutionEnvironment *ExecutionEnvironment::
get_ptr() {
  if (_global_ptr == (ExecutionEnvironment *)NULL) {
    _global_ptr = new ExecutionEnvironment;
  }
  return _global_ptr;
}


////////////////////////////////////////////////////////////////////
//     Function: ExecutionEnvironment::read_environment_variables
//       Access: Private
//  Description: Fills up the internal table of existing environment
//               variables, if we are in PREREAD_ENVIRONMENT mode.
//               Otherwise, does nothing.
////////////////////////////////////////////////////////////////////
void ExecutionEnvironment::
read_environment_variables() {
#if defined(PENV_LINUX)
  // In Linux, we might not be able to use getenv() at static init
  // time.  However, we may be lucky and have a file called
  // /proc/self/environ that may be read to determine all of our
  // environment variables.

  ifstream proc("/proc/self/environ");
  if (proc.fail()) {
    cerr << "Cannot read /proc/self/environ; environment variables unavailable.\n";
    return;
  }

  int ch = proc.get();
  while (!proc.eof() && !proc.fail()) {
    string variable;
    string value;
      
    while (!proc.eof() && !proc.fail() && ch != '=' && ch != '\0') {
      variable += (char)ch;
      ch = proc.get();
    }
      
    if (ch == '=') {
      ch = proc.get();
      while (!proc.eof() && !proc.fail() && ch != '\0') {
	value += (char)ch;
	ch = proc.get();
      }
    }

    if (!variable.empty()) {
      _variables[variable] = value;
    }
    ch = proc.get();
  }
#endif
}

////////////////////////////////////////////////////////////////////
//     Function: ExecutionEnvironment::read_args
//       Access: Private
//  Description: Reads all the command-line arguments and the name of
//               the binary file, if possible.
////////////////////////////////////////////////////////////////////
void ExecutionEnvironment::
read_args() {
#if defined(USE_ARGV)
  int argc = ARGC;
  if (argc > 0) {
    _binary_name = ARGV[0];
  }

  for (int i = 1; i < argc; i++) {
    _args.push_back(ARGV[i]);
  }

#elif defined(PENV_LINUX)
  // In Linux, we might not be able to use the global ARGC/ARGV
  // variables at static init time.  However, we may be lucky and have
  // a file called /proc/self/cmdline that may be read to determine
  // all of our command-line arguments.

  ifstream proc("/proc/self/cmdline");
  if (proc.fail()) {
    cerr << "Cannot read /proc/self/cmdline; command-line arguments unavailable to config.\n";
    return;
  }

  int ch = proc.get();
  int index = 0;
  while (!proc.eof() && !proc.fail()) {
    string arg;

    while (!proc.eof() && !proc.fail() && ch != '\0') {
      arg += (char)ch;
      ch = proc.get();
    }

    if (index == 0) {
      _binary_name = arg;
    } else {
      _args.push_back(arg);
    }
    index++;

    ch = proc.get();
  }
#endif
}
