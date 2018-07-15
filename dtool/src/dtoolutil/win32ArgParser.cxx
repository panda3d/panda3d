/**
 * PANDA 3D SOFTWARE
 * Copyright (c) Carnegie Mellon University.  All rights reserved.
 *
 * All use of this software is subject to the terms of the revised BSD
 * license.  You should have received a copy of this license along
 * with this source code in a file named "LICENSE."
 *
 * @file win32ArgParser.cxx
 * @author drose
 * @date 2011-11-08
 */

#include "win32ArgParser.h"

#ifdef _WIN32

#include "memoryBase.h"
#include "textEncoder.h"
#include "globPattern.h"
#include "filename.h"
#include "executionEnvironment.h"

#include <windows.h>
#include <Tlhelp32.h>

using std::string;

/**
 *
 */
Win32ArgParser::
Win32ArgParser() :
  _argv(nullptr),
  _argc(0)
{
}

/**
 *
 */
Win32ArgParser::
~Win32ArgParser() {
  clear();
}

/**
 * Resets the parser to empty command line and deallocates the internal argv
 * array.
 */
void Win32ArgParser::
clear() {
  assert(_argc == (int)_args.size());

  if (_argv != nullptr) {
    for (int i = 0; i < _argc; ++i) {
      PANDA_FREE_ARRAY(_argv[i]);
    }
    PANDA_FREE_ARRAY(_argv);
    _argv = nullptr;
  }

  _argc = 0;
  _args.clear();
}

/**
 * Sets the string that indicates the full Win32 command line, and starts
 * parsing this into argc, argv.
 */
void Win32ArgParser::
set_command_line(const string &command_line) {
  clear();

  const char *p = command_line.c_str();
  while (*p != '\0') {
    parse_unquoted_arg(p);

    // Skip whitespace.
    while (*p != '\0' && isspace(*p)) {
      ++p;
    }
  }

  assert(_argc == 0 && _argv == nullptr);
  _argc = (int)_args.size();
  _argv = (char **)PANDA_MALLOC_ARRAY(_argc * sizeof(char *));
  for (int i = 0; i < _argc; ++i) {
    const string &arg = _args[i];
    char *astr = (char *)PANDA_MALLOC_ARRAY(arg.size() + 1);
    memcpy(astr, arg.data(), arg.size());
    astr[arg.size()] = '\0';
    _argv[i] = astr;
  }
}

/**
 * Sets the Unicode string that indicates the full Win32 command line, and
 * starts parsing this into argc, argv.
 */
void Win32ArgParser::
set_command_line(const std::wstring &command_line) {
  TextEncoder encoder;
  encoder.set_encoding(Filename::get_filesystem_encoding());
  encoder.set_wtext(command_line);
  set_command_line(encoder.get_text());
}

/**
 * Tells the parser to call GetCommandLine() to query the system command line
 * string, and parse it into argc, argv.
 */
void Win32ArgParser::
set_system_command_line() {
  LPWSTR command_line = GetCommandLineW();
  set_command_line(command_line);
}

/**
 * Returns the argv array as computed by set_command_line() or
 * set_system_command_line(). This array indexes directly into data allocated
 * within the Win32ArgParser object; it will remain valid until
 * set_command_line() or clear() is again called, or until the parser object
 * destructs.
 */
char **Win32ArgParser::
get_argv() {
  return _argv;
}

/**
 * Returns the number of elements in the argv array.
 */
int Win32ArgParser::
get_argc() {
  return _argc;
}

/**
 * Returns true if we should attempt to process (and apply glob matching) to
 * the command line, or false if we should not (for instance, because it has
 * already been done by the shell).
 */
bool Win32ArgParser::
do_glob() {
  // First, we check for the PANDA_GLOB environment variable.  If this is
  // present, it overrides any other checks: "0" means not to do the glob, "1"
  // means to do it.
  string envvar = ExecutionEnvironment::get_environment_variable("PANDA_GLOB");
  if (!envvar.empty()) {
    std::istringstream strm(envvar);
    int value;
    strm >> value;
    if (!strm.fail()) {
      return (value != 0);
    }
  }

  // Nothing explicit, so the default is to perform globbing only if we were
  // launched from the Windows default command shell, cmd.exe.  Presumably if
  // we were launched from something else, like Python, the caller won't
  // expect globbing to be performed; and if we were launched from a Cygwin
  // shell, it will already have been performed.

  // Unfortunately, it is surprisingly difficult to determine the parent
  // process in Windows.  We have to enumerate all of the processes to find
  // it.

  HANDLE toolhelp = CreateToolhelp32Snapshot(TH32CS_SNAPPROCESS, 0);

  PROCESSENTRY32 entry;
  memset(&entry, 0, sizeof(entry));
  entry.dwSize = sizeof(entry);

  DWORD current_id = GetCurrentProcessId();
  DWORD parent_id = -1;

  if (Process32First(toolhelp, &entry)) {
    do {
      if (entry.th32ProcessID == current_id) {
        parent_id = entry.th32ParentProcessID;
        break;
      }
    } while (Process32Next(toolhelp, &entry));
  }

  Filename parent_exe;
  if (parent_id != -1) {
    // Now we've got the parent process ID, go back through the list to get
    // its process name.
    if (Process32First(toolhelp, &entry)) {
      do {
        if (entry.th32ProcessID == parent_id) {
          parent_exe = Filename::from_os_specific(entry.szExeFile);
          break;
        }
      } while (Process32Next(toolhelp, &entry));
    }
  }

  CloseHandle(toolhelp);
  string basename = parent_exe.get_basename();
  if (basename == "cmd.exe") {
    return true;
  }

  return false;
}

/**
 * Parses the quoted argument beginning at p and returns it.  Advances p to
 * the first character following the close quote.
 */
string Win32ArgParser::
parse_quoted_arg(const char *&p) {
  char quote = *p;
  ++p;
  string result;

  while (*p != '\0' && *p != quote) {
    // TODO: handle caret?  What does it mean?

    if (*p == '\\') {
      // A backslash is an escape character only when it precedes a quote
      // mark, or a series of backslashes precede a quote mark.
      int num_slashes = 1;
      ++p;
      while (*p == '\\') {
        ++p;
        ++num_slashes;
      }
      if (*p == quote) {
        // A series of backslashes precede a quote mark.  This means something
        // special.  First, each pair of backslashes means a single backslash.
        for (int i = 0; i < num_slashes; i += 2) {
          result += '\\';
        }
        // And if there's no odd backslashes left over, we've reached the
        // closing quote and we're done.
        if ((num_slashes & 1) == 0) {
          ++p;
          return result;
        }

        // But if there's an odd backslash, it simply escapes the quote mark.
        result += quote;
        ++p;

      } else {
        // A series of backslashes not followed by a quote mark is interpreted
        // literally, not even counting them by twos, per Win32's weird rules.
        for (int i = 0; i < num_slashes; ++i) {
          result += '\\';
        }
      }

    } else {
      // Neither a backslash nor a quote mark, so just interpret it literally.
      result += *p;
      ++p;
    }
  }

  if (*p == quote) {
    ++p;
  }

  return result;
}

/**
 * Parses the unquoted argument beginning at p and saves it in _char_args.
 * Advances p to the first whitespace following the argument.
 */
void Win32ArgParser::
parse_unquoted_arg(const char *&p) {
  string result;
  bool contains_quotes = false;
  while (*p != '\0' && !isspace(*p)) {
    if (*p == '"') {
      // Begin a quoted sequence.
      contains_quotes = true;
      result += parse_quoted_arg(p);
    } else {
      // A regular character.
      result += *p;
      ++p;
    }
  }

  Filename filename = Filename::from_os_specific(result);
  GlobPattern glob(filename);
  if (!contains_quotes && glob.has_glob_characters()) {
    // If the arg contains one or more glob characters (and no quotation
    // marks), we attempt to expand the files.  This means we interpret it as
    // a Windows-specific filename.
    vector_string expand;
    if (glob.match_files(expand) != 0) {
      // The files matched.  Add the expansions.
      vector_string::const_iterator ei;
      for (ei = expand.begin(); ei != expand.end(); ++ei) {
        Filename filename(*ei);
        save_arg(filename.to_os_specific());
      }
    } else {
      // There wasn't a match.  Just add the original, unexpanded string, like
      // bash does.
      save_arg(result);
    }

  } else {
    // No glob characters means we just store it directly.  Also, an embedded
    // quoted string, anywhere within the arg, means we can't expand the glob
    // characters.
    save_arg(result);
  }
}

/**
 * Stores the indicated string as the next argument in _args.
 */
void Win32ArgParser::
save_arg(const string &arg) {
  _args.push_back(arg);
}

#endif  // _WIN32
