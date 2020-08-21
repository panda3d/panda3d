/**
 * PANDA 3D SOFTWARE
 * Copyright (c) Carnegie Mellon University.  All rights reserved.
 *
 * All use of this software is subject to the terms of the revised BSD
 * license.  You should have received a copy of this license along
 * with this source code in a file named "LICENSE."
 *
 * @file programBase.cxx
 * @author drose
 * @date 2000-02-13
 */

#include "programBase.h"
#include "wordWrapStream.h"

#include "pnmFileTypeRegistry.h"
#include "indent.h"
#include "dSearchPath.h"
#include "coordinateSystem.h"
#include "dconfig.h"
#include "string_utils.h"
#include "vector_string.h"
#include "configVariableInt.h"
#include "configVariableBool.h"
#include "panda_getopt_long.h"
#include "preprocess_argv.h"
#include "pandaSystem.h"

#include <stdlib.h>
#include <algorithm>
#include <ctype.h>

// This manifest is defined if we are running on a system (e.g.  most any
// Unix) that allows us to determine the width of the terminal screen via an
// ioctl() call.  It's just handy to know for formatting output nicely for the
// user.
#ifdef IOCTL_TERMINAL_WIDTH
  #include <termios.h>
  #ifndef TIOCGWINSZ
    #include <sys/ioctl.h>
  #elif __APPLE__
    #include <sys/ioctl.h>
  #endif  // TIOCGWINSZ
#endif  // IOCTL_TERMINAL_WIDTH

using std::cerr;
using std::cout;
using std::max;
using std::min;
using std::string;

bool ProgramBase::SortOptionsByIndex::
operator () (const Option *a, const Option *b) const {
  if (a->_index_group != b->_index_group) {
    return a->_index_group < b->_index_group;
  }
  return a->_sequence < b->_sequence;
}

// This should be called at program termination just to make sure Notify gets
// properly flushed before we exit, if someone calls exit().  It's probably
// not necessary, but why not be phobic about it?
static void flush_nout() {
  nout << std::flush;
}

static ConfigVariableInt default_terminal_width
("default-terminal-width", 72,
 PRC_DESC("Specify the column at which to wrap output lines "
          "from pandatool-based programs, if it cannot be determined "
          "automatically."));

static ConfigVariableBool use_terminal_width
("use-terminal-width", true,
 PRC_DESC("True to try to determine the terminal width automatically from "
          "the operating system, if supported; false to use the width "
          "specified by default-terminal-width even if the operating system "
          "appears to report a valid width."));

/**
 *
 */
ProgramBase::
ProgramBase(const string &name) : _name(name) {
  // Set up Notify to write output to our own formatted stream.
  Notify::ptr()->set_ostream_ptr(new WordWrapStream(this), true);

  // And we'll want to be sure to flush that in all normal exit cases.
  atexit(&flush_nout);

  _path_replace = new PathReplace;

  // If a program never adds the path store options, the default path store is
  // PS_absolute.  This is the most robust solution for programs that read
  // files but do not need to write them.
  _path_replace->_path_store = PS_absolute;
  _got_path_store = false;
  _got_path_directory = false;

  _next_sequence = 0;
  _sorted_options = false;
  _last_newline = false;
  _got_terminal_width = false;
  _got_option_indent = false;

  add_option("h", "", 100,
             "Display this help page.",
             &ProgramBase::handle_help_option, nullptr, (void *)this);

  // It's nice to start with a blank line.
  nout << "\r";
}

/**
 *
 */
ProgramBase::
~ProgramBase() {
  // Reset Notify in case any messages get sent after our destruction--our
  // stream is no longer valid.
  Notify::ptr()->set_ostream_ptr(nullptr, false);
}

/**
 * Writes the program description to stderr.
 */
void ProgramBase::
show_description() {
  nout << _description << "\n";
}

/**
 * Writes the usage line(s) to stderr.
 */
void ProgramBase::
show_usage() {
  nout << "\rUsage:\n";
  Runlines::const_iterator ri;
  string prog = "  " + _program_name.get_basename_wo_extension();

  for (ri = _runlines.begin(); ri != _runlines.end(); ++ri) {
    show_text(prog, prog.length() + 1, *ri);
  }
  nout << "\r";
}

/**
 * Describes each of the available options to stderr.
 */
void ProgramBase::
show_options() {
  sort_options();
  if (!_got_option_indent) {
    get_terminal_width();
    _option_indent = min(15, (int)(_terminal_width * 0.25));
    _got_option_indent = true;
  }

  nout << "Options:\n";
  OptionsByIndex::const_iterator oi;
  for (oi = _options_by_index.begin(); oi != _options_by_index.end(); ++oi) {
    const Option &opt = *(*oi);
    string prefix = "  -" + opt._option + " " + opt._parm_name;
    show_text(prefix, _option_indent, opt._description + "\r");
  }
}

/**
 * Formats the indicated text and its prefix for output to stderr with the
 * known _terminal_width.
 */
void ProgramBase::
show_text(const string &prefix, int indent_width, string text) {
  get_terminal_width();

  // This is correct!  It goes go to cerr, not to nout.  Sending it to nout
  // would be cyclic, since nout is redefined to map back through this
  // function.
  format_text(cerr, _last_newline,
              prefix, indent_width, text, _terminal_width);
}

/**
 * Generates a man page in nroff syntax based on the description and options.
 * This is useful when creating a man page for this utility.
 */
void ProgramBase::
write_man_page(std::ostream &out) {
  string prog = _program_name.get_basename_wo_extension();
  out << ".\\\" Automatically generated by " << prog << " -write-man\n";

  // Format the man page title as the uppercase version of the program name,
  // as per the UNIX manual conventions.
  out << ".TH ";
  string::const_iterator si;
  for (si = _name.begin(); si != _name.end(); ++si) {
    out << (char)toupper(*si);
  }

  // Generate a date string for inclusion into the footer.
  char date_str[256];
  date_str[0] = 0;
  time_t current_time = time(nullptr);

  if (current_time != (time_t) -1) {
    tm *today = localtime(&current_time);
    if (today == nullptr || 0 == strftime(date_str, 256, "%d %B %Y", today)) {
      date_str[0] = 0;
    }
  }

  out << " 1 \"" << date_str << "\" \""
      << PandaSystem::get_version_string() << "\" Panda3D\n";

  out << ".SH NAME\n";
  if (_brief.empty()) {
    out << _name << "\n";
  } else {
    out << _name << " \\- " << _brief << "\n";
  }

  out << ".SH SYNOPSIS\n";
  Runlines::const_iterator ri = _runlines.begin();
  if (ri != _runlines.end()) {
    out << "\\fB" << prog << "\\fR " << *ri << "\n";
    ++ri;
  }

  for (; ri != _runlines.end(); ++ri) {
    out << ".br\n";
    out << "\\fB" << prog << "\\fR " << *ri << "\n";
  }

  out << ".SH DESCRIPTION\n";
  string::const_iterator di;
  char prev = 0;
  for (di = _description.begin(); di != _description.end(); ++di) {
    if ((*di) == '-') {
      // We have to escape hyphens.
      out << "\\-";
    } else if (prev == '\n' && (*di) == '\n') {
      // Indicate the start of a new paragraph.
      out << ".PP\n";
    } else {
      out << (char)(*di);
    }
    prev = (*di);
  }
  out << "\n";

  out << ".SH OPTIONS\n";
  sort_options();
  OptionsByIndex::const_iterator oi;
  for (oi = _options_by_index.begin(); oi != _options_by_index.end(); ++oi) {
    const Option &opt = *(*oi);
    out << ".TP\n";

    if (opt._parm_name.empty()) {
      out << ".B \\-" << opt._option << "\n";
    } else {
      out << ".BI \"\\-" << opt._option << " \" \"" << opt._parm_name << "\"\n";
    }
    out << opt._description << "\n";
  }
}

/**
 * Dispatches on each of the options on the command line, and passes the
 * remaining parameters to handle_args().  If an error on the command line is
 * detected, will automatically call show_usage() and exit(1).
 */
void ProgramBase::
parse_command_line(int argc, char **argv) {
  preprocess_argv(argc, argv);

  // Setting this variable to zero reinitializes the options parser This is
  // only necessary for processing multiple command lines in the same program
  // (mainly the MaxToEgg converter plugin)
  extern int optind;
  optind = 0;

  _program_name = Filename::from_os_specific(argv[0]);
  int i;
  for (i = 1; i < argc; i++) {
    _program_args.push_back(argv[i]);
  }

  if (_name.empty()) {
    _name = _program_name.get_basename_wo_extension();
  }

  // Catch a special hidden option: -write-man, which causes the tool to
  // generate a manual page.
  if (argc > 1 && strcmp(argv[1], "-write-man") == 0) {
    if (argc == 2) {
      write_man_page(cout);

    } else if (argc == 3) {
      if (strlen(argv[2]) == 1 && argv[2][0] == '-') {
        write_man_page(cout);

      } else {
        pofstream man_out(argv[2], std::ios::out | std::ios::trunc);
        if (!man_out) {
          cerr << "Failed to open output file " << argv[2] << "!\n";
        }
        write_man_page(man_out);
        man_out.close();
      }
    } else {
      cerr << "Invalid number of options for -write-man!\n";
      exit(1);
    }
    exit(0);
  }

  // Build up the long options list and the short options string for
  // getopt_long_only().
  pvector<struct option> long_options;
  string short_options;

  // We also need to build a temporary map of int index numbers to Option
  // pointers.  We'll pass these index numbers to GNU's getopt_long() so we
  // can tell one option from another.
  typedef pmap<int, const Option *> Options;
  Options options;

  OptionsByName::const_iterator oi;
  int next_index = 256;

  // Let's prefix the option string with "-" to tell getopt that we want it to
  // tell us the post-option arguments, instead of trying to meddle with ARGC
  // and ARGV (which we aren't using directly).
  short_options = "-";

  for (oi = _options_by_name.begin(); oi != _options_by_name.end(); ++oi) {
    const Option &opt = (*oi).second;

    int index;
    if (opt._option.length() == 1) {
      // This is a "short" option; its option string consists of only one
      // letter.  Its index is the letter itself.
      index = (int)opt._option[0];

      short_options += opt._option;
      if (!opt._parm_name.empty()) {
        // This option takes an argument.
        short_options += ':';
      }
    } else {
      // This is a "long" option; we'll assign it the next available index.
      index = ++next_index;
    }

    // Now add it to the GNU data structures.
    struct option gopt;
    gopt.name = (char *)opt._option.c_str();
    gopt.has_arg = (opt._parm_name.empty()) ?
      no_argument : required_argument;
    gopt.flag = nullptr;

    // Return an index into the _options_by_index array, offset by 256 so we
    // don't confuse it with '?'.
    gopt.val = index;

    long_options.push_back(gopt);

    options[index] = &opt;
  }

  // Finally, add one more structure, all zeroes, to indicate the end of the
  // options.
  struct option gopt;
  memset(&gopt, 0, sizeof(gopt));
  long_options.push_back(gopt);

  // We'll use this vector to save the non-option arguments.  Generally, these
  // will all be at the end, but with the GNU extensions, they need not be.
  Args remaining_args;

  // Now call getopt_long() to actually parse the arguments.
  extern char *optarg;
  const struct option *long_opts = &long_options[0];

  int flag =
    getopt_long_only(argc, argv, short_options.c_str(), long_opts, nullptr);
  while (flag != EOF) {
    string arg;
    if (optarg != nullptr) {
      arg = optarg;
    }

    switch (flag) {
    case '?':
      // Invalid option or parameter.
      show_usage();
      exit(1);

    case '\x1':
      // A special return value from getopt() indicating a non-option
      // argument.
      remaining_args.push_back(arg);
      break;

    default:
      {
        // A normal option.  Figure out which one it is.
        Options::const_iterator ii;
        ii = options.find(flag);
        if (ii == options.end()) {
          nout << "Internal error!  Invalid option index returned.\n";
          abort();
        }

        const Option &opt = *(*ii).second;
        bool okflag = true;
        if (opt._option_function != (OptionDispatchFunction)nullptr) {
          okflag = (*opt._option_function)(opt._option, arg, opt._option_data);
        }
        if (opt._option_method != (OptionDispatchMethod)nullptr) {
          okflag = (*opt._option_method)(this, opt._option, arg, opt._option_data);
        }
        if (opt._bool_var != nullptr) {
          (*opt._bool_var) = true;
        }

        if (!okflag) {
          show_usage();
          exit(1);
        }
      }
    }

    flag =
      getopt_long_only(argc, argv, short_options.c_str(), long_opts, nullptr);
  }

  if (!handle_args(remaining_args)) {
    show_usage();
    exit(1);
  }

  if (!post_command_line()) {
    show_usage();
    exit(1);
  }
}

/**
 * Returns the command that invoked this program, as a shell-friendly string,
 * suitable for pasting into the comments of output files.
 */
string ProgramBase::
get_exec_command() const {
  string command;

  command = _program_name.get_basename_wo_extension();
  Args::const_iterator ai;
  for (ai = _program_args.begin(); ai != _program_args.end(); ++ai) {
    const string &arg = (*ai);

    // First, check to see if the string is shell-acceptable.
    bool legal = true;
    string::const_iterator si;
    for (si = arg.begin(); legal && si != arg.end(); ++si) {
      switch (*si) {
      case ' ':
      case '\n':
      case '\t':
      case '*':
      case '?':
      case '\\':
      case '(':
      case ')':
      case '|':
      case '&':
      case '<':
      case '>':
      case '"':
      case ';':
      case '$':
        legal = false;
      }
    }

    if (legal) {
      command += " " + arg;
    } else {
      command += " '" + arg + "'";
    }
  }

  return command;
}


/**
 * Does something with the additional arguments on the command line (after all
 * the -options have been parsed).  Returns true if the arguments are good,
 * false otherwise.
 */
bool ProgramBase::
handle_args(ProgramBase::Args &args) {
  if (!args.empty()) {
    nout << "Unexpected arguments on command line:\n";
    Args::const_iterator ai;
    for (ai = args.begin(); ai != args.end(); ++ai) {
      nout << (*ai) << " ";
    }
    nout << "\r";
    return false;
  }

  return true;
}

/**
 * This is called after the command line has been completely processed, and it
 * gives the program a chance to do some last-minute processing and validation
 * of the options and arguments.  It should return true if everything is fine,
 * false if there is an error.
 */
bool ProgramBase::
post_command_line() {
  return true;
}

/**
 * Sets a brief synopsis of the program's function.  This is currently only
 * used for generating the synopsis of the program's man page.
 *
 * This should be of the format: "perform operation foo on bar files"
 */
void ProgramBase::
set_program_brief(const string &brief) {
  _brief = brief;
}

/**
 * Sets the description of the program that will be reported by show_usage().
 * The description should be one long string of text.  Embedded newline
 * characters are interpreted as paragraph breaks and printed as blank lines.
 */
void ProgramBase::
set_program_description(const string &description) {
  _description = description;
}

/**
 * Removes all of the runlines that were previously added, presumably before
 * adding some new ones.
 */
void ProgramBase::
clear_runlines() {
  _runlines.clear();
}

/**
 * Adds an additional line to the list of lines that will be displayed to
 * describe briefly how the program is to be run.  Each line should be
 * something like "[opts] arg1 arg2", that is, it does *not* include the name
 * of the program, but it includes everything that should be printed after the
 * name of the program.
 *
 * Normally there is only one runline for a given program, but it is possible
 * to define more than one.
 */
void ProgramBase::
add_runline(const string &runline) {
  _runlines.push_back(runline);
}

/**
 * Removes all of the options that were previously added, presumably before
 * adding some new ones.  Normally you wouldn't want to do this unless you
 * want to completely replace all of the options defined by base classes.
 */
void ProgramBase::
clear_options() {
  _options_by_name.clear();
}

/**
 * Adds (or redefines) a command line option.  When parse_command_line() is
 * executed it will look for these options (followed by a hyphen) on the
 * command line; when a particular option is found it will call the indicated
 * option_function, supplying the provided option_data.  This allows the user
 * to define a function that does some special behavior for any given option,
 * or to use any of a number of generic pre-defined functions to fill in data
 * for each option.
 *
 * Each option may or may not take a parameter.  If parm_name is nonempty, it
 * is assumed that the option does take a parameter (and parm_name contains
 * the name that will be printed by show_options()).  This parameter will be
 * supplied as the second parameter to the dispatch function.  If parm_name is
 * empty, it is assumed that the option does not take a parameter.  There is
 * no provision for optional parameters.
 *
 * The options are listed first in order by their index_group number, and then
 * in the order that add_option() was called.  This provides a mechanism for
 * listing the options defined in derived classes before those of the base
 * classes.
 */
void ProgramBase::
add_option(const string &option, const string &parm_name,
           int index_group, const string &description,
           OptionDispatchFunction option_function,
           bool *bool_var, void *option_data) {
  Option opt;
  opt._option = option;
  opt._parm_name = parm_name;
  opt._index_group = index_group;
  opt._sequence = ++_next_sequence;
  opt._description = description;
  opt._option_function = option_function;
  opt._option_method = (OptionDispatchMethod)nullptr;
  opt._bool_var = bool_var;
  opt._option_data = option_data;

  _options_by_name[option] = opt;
  _sorted_options = false;

  if (bool_var != nullptr) {
    (*bool_var) = false;
  }
}

/**
 * This is another variant on add_option(), above, except that it receives a
 * pointer to a "method", which is really just another static (or global)
 * function, whose first parameter is a ProgramBase *.
 *
 * We can't easily add a variant that accepts a real method, because the C++
 * syntax for methods requires us to know exactly what class object the method
 * is defined for, and we want to support adding pointers for methods that are
 * defined in other classes.  So we have this hacky thing, which requires the
 * "method" to be declared static, and receive its this pointer explicitly, as
 * the first argument.
 */
void ProgramBase::
add_option(const string &option, const string &parm_name,
           int index_group, const string &description,
           OptionDispatchMethod option_method,
           bool *bool_var, void *option_data) {
  Option opt;
  opt._option = option;
  opt._parm_name = parm_name;
  opt._index_group = index_group;
  opt._sequence = ++_next_sequence;
  opt._description = description;
  opt._option_function = (OptionDispatchFunction)nullptr;
  opt._option_method = option_method;
  opt._bool_var = bool_var;
  opt._option_data = option_data;

  _options_by_name[option] = opt;
  _sorted_options = false;

  if (bool_var != nullptr) {
    (*bool_var) = false;
  }
}

/**
 * Changes the description associated with a previously-defined option.
 * Returns true if the option was changed, false if it hadn't been defined.
 */
bool ProgramBase::
redescribe_option(const string &option, const string &description) {
  OptionsByName::iterator oi = _options_by_name.find(option);
  if (oi == _options_by_name.end()) {
    return false;
  }
  (*oi).second._description = description;
  return true;
}

/**
 * Removes a previously-defined option.  Returns true if the option was
 * removed, false if it hadn't existed.
 */
bool ProgramBase::
remove_option(const string &option) {
  OptionsByName::iterator oi = _options_by_name.find(option);
  if (oi == _options_by_name.end()) {
    return false;
  }
  _options_by_name.erase(oi);
  _sorted_options = false;
  return true;
}

/**
 * Adds -pr etc.  as valid options for this program.  These are appropriate
 * for a model converter or model reader type program, and specify how to
 * locate possibly-invalid pathnames in the source model file.
 */
void ProgramBase::
add_path_replace_options() {
  add_option
    ("pr", "path_replace", 40,
     "Sometimes references to other files (textures, external references) "
     "are stored with a full path that is appropriate for some other system, "
     "but does not exist here.  This option may be used to specify how "
     "those invalid paths map to correct paths.  Generally, this is of "
     "the form 'orig_prefix=replacement_prefix', which indicates a "
     "particular initial sequence of characters that should be replaced "
     "with a new sequence; e.g. '/c/home/models=/beta/fish'.  "
     "If the replacement prefix does not begin with a slash, the file "
     "will then be searched for along the search path specified by -pp.  "
     "You may use standard filename matching characters ('*', '?', etc.) in "
     "the original prefix, and '**' as a component by itself stands for "
     "any number of components.\n\n"

     "This option may be repeated as necessary; each file will be tried "
     "against each specified method, in the order in which they appear in "
     "the command line, until the file is found.  If the file is not found, "
     "the last matching prefix is used anyway.",
     &ProgramBase::dispatch_path_replace, nullptr, _path_replace.p());

  add_option
    ("pp", "dirname", 40,
     "Adds the indicated directory name to the list of directories to "
     "search for filenames referenced by the source file.  This is used "
     "only for relative paths, or for paths that are made relative by a "
     "-pr replacement string that doesn't begin with a leading slash.  "
     "The model-path is always implicitly searched anyway.",
     &ProgramBase::dispatch_search_path, nullptr, &(_path_replace->_path));
}

/**
 * Adds -ps etc.  as valid options for this program.  These are appropriate
 * for a model converter type program, and specify how to represent filenames
 * in the output file.
 */
void ProgramBase::
add_path_store_options() {
  // If a program has path store options at all, the default path store is
  // relative.
  _path_replace->_path_store = PS_relative;

  add_option
    ("ps", "path_store", 40,
     "Specifies the way an externally referenced file is to be "
     "represented in the resulting output file.  This "
     "assumes the named filename actually exists; "
     "see -pr to indicate how to deal with external "
     "references that have bad pathnames.  "
     "This option will not help you to find a missing file, but simply "
     "controls how filenames are represented in the output.\n\n"

     "The option may be one of: rel, abs, rel_abs, strip, or keep.  If "
     "either rel or rel_abs is specified, the files are made relative to "
     "the directory specified by -pd.  The default is rel.",
     &ProgramBase::dispatch_path_store, &_got_path_store,
     &(_path_replace->_path_store));

  add_option
    ("pd", "path_directory", 40,
     "Specifies the name of a directory to make paths relative to, if "
     "'-ps rel' or '-ps rel_abs' is specified.  If this is omitted, the "
     "directory name is taken from the name of the output file.",
     &ProgramBase::dispatch_filename, &_got_path_directory,
     &(_path_replace->_path_directory));

  add_option
    ("pc", "target_directory", 40,
     "Copies textures and other dependent files into the indicated "
     "directory.  If a relative pathname is specified, it is relative "
     "to the directory specified with -pd, above.",
     &ProgramBase::dispatch_filename, &(_path_replace->_copy_files),
     &(_path_replace->_copy_into_directory));
}

/**
 * Standard dispatch function for an option that takes no parameters, and does
 * nothing special.  Typically this would be used for a boolean flag, whose
 * presence means something and whose absence means something else.  Use the
 * bool_var parameter to add_option() to determine whether the option appears
 * on the command line or not.
 */
bool ProgramBase::
dispatch_none(const string &, const string &, void *) {
  return true;
}

/**
 * Standard dispatch function for an option that takes no parameters, and when
 * it is present sets a bool variable to the 'true' value.  This is another
 * way to handle a boolean flag.  See also dispatch_none() and
 * dispatch_false().
 *
 * The data pointer is to a bool variable.
 */
bool ProgramBase::
dispatch_true(const string &, const string &, void *var) {
  bool *bp = (bool *)var;
  (*bp) = true;
  return true;
}

/**
 * Standard dispatch function for an option that takes no parameters, and when
 * it is present sets a bool variable to the 'false' value.  This is another
 * way to handle a boolean flag.  See also dispatch_none() and
 * dispatch_true().
 *
 * The data pointer is to a bool variable.
 */
bool ProgramBase::
dispatch_false(const string &, const string &, void *var) {
  bool *bp = (bool *)var;
  (*bp) = false;
  return true;
}

/**
 * Standard dispatch function for an option that takes no parameters, but
 * whose presence on the command line increments an integer counter for each
 * time it appears.  -v is often an option that works this way.  The data
 * pointer is to an int counter variable.
 */
bool ProgramBase::
dispatch_count(const string &, const string &, void *var) {
  int *ip = (int *)var;
  (*ip)++;

  return true;
}

/**
 * Standard dispatch function for an option that takes one parameter, which is
 * to be interpreted as an integer.  The data pointer is to an int variable.
 */
bool ProgramBase::
dispatch_int(const string &opt, const string &arg, void *var) {
  int *ip = (int *)var;

  if (!string_to_int(arg, *ip)) {
    nout << "Invalid integer parameter for -" << opt << ": "
         << arg << "\n";
    return false;
  }

  return true;
}

/**
 * Standard dispatch function for an option that takes a pair of integer
 * parameters.  The data pointer is to an array of two integers.
 */
bool ProgramBase::
dispatch_int_pair(const string &opt, const string &arg, void *var) {
  int *ip = (int *)var;

  vector_string words;
  tokenize(arg, words, ",");

  bool okflag = false;
  if (words.size() == 2) {
    okflag =
      string_to_int(words[0], ip[0]) &&
      string_to_int(words[1], ip[1]);
  }

  if (!okflag) {
    nout << "-" << opt
         << " requires a pair of integers separated by a comma.\n";
    return false;
  }

  return true;
}

/**
 * Standard dispatch function for an option that takes a quad of integer
 * parameters.  The data pointer is to an array of four integers.
 */
bool ProgramBase::
dispatch_int_quad(const string &opt, const string &arg, void *var) {
  int *ip = (int *)var;

  vector_string words;
  tokenize(arg, words, ",");

  bool okflag = false;
  if (words.size() == 4) {
    okflag =
      string_to_int(words[0], ip[0]) &&
      string_to_int(words[1], ip[1]) &&
      string_to_int(words[1], ip[2]) &&
      string_to_int(words[1], ip[3]);
  }

  if (!okflag) {
    nout << "-" << opt
         << " requires a quad of integers separated by a comma.\n";
    return false;
  }

  return true;
}

/**
 * Standard dispatch function for an option that takes one parameter, which is
 * to be interpreted as a double.  The data pointer is to an double variable.
 */
bool ProgramBase::
dispatch_double(const string &opt, const string &arg, void *var) {
  double *ip = (double *)var;

  if (!string_to_double(arg, *ip)) {
    nout << "Invalid numeric parameter for -" << opt << ": "
         << arg << "\n";
    return false;
  }

  return true;
}

/**
 * Standard dispatch function for an option that takes a pair of double
 * parameters.  The data pointer is to an array of two doubles.
 */
bool ProgramBase::
dispatch_double_pair(const string &opt, const string &arg, void *var) {
  double *ip = (double *)var;

  vector_string words;
  tokenize(arg, words, ",");

  bool okflag = false;
  if (words.size() == 2) {
    okflag =
      string_to_double(words[0], ip[0]) &&
      string_to_double(words[1], ip[1]);
  }

  if (!okflag) {
    nout << "-" << opt
         << " requires a pair of numbers separated by a comma.\n";
    return false;
  }

  return true;
}

/**
 * Standard dispatch function for an option that takes a triple of double
 * parameters.  The data pointer is to an array of three doubles.
 */
bool ProgramBase::
dispatch_double_triple(const string &opt, const string &arg, void *var) {
  double *ip = (double *)var;

  vector_string words;
  tokenize(arg, words, ",");

  bool okflag = false;
  if (words.size() == 3) {
    okflag =
      string_to_double(words[0], ip[0]) &&
      string_to_double(words[1], ip[1]) &&
      string_to_double(words[2], ip[2]);
  }

  if (!okflag) {
    nout << "-" << opt
         << " requires three numbers separated by commas.\n";
    return false;
  }

  return true;
}

/**
 * Standard dispatch function for an option that takes a quad of double
 * parameters.  The data pointer is to an array of four doubles.
 */
bool ProgramBase::
dispatch_double_quad(const string &opt, const string &arg, void *var) {
  double *ip = (double *)var;

  vector_string words;
  tokenize(arg, words, ",");

  bool okflag = false;
  if (words.size() == 4) {
    okflag =
      string_to_double(words[0], ip[0]) &&
      string_to_double(words[1], ip[1]) &&
      string_to_double(words[2], ip[2]) &&
      string_to_double(words[3], ip[3]);
  }

  if (!okflag) {
    nout << "-" << opt
         << " requires four numbers separated by commas.\n";
    return false;
  }

  return true;
}

/**
 * Standard dispatch function for an option that takes a color, as l or l,a or
 * r,g,b or r,g,b,a.  The data pointer is to an array of four floats, e.g.  a
 * LColor.
 */
bool ProgramBase::
dispatch_color(const string &opt, const string &arg, void *var) {
  PN_stdfloat *ip = (PN_stdfloat *)var;

  vector_string words;
  tokenize(arg, words, ",");

  bool okflag = false;
  switch (words.size()) {
  case 4:
    okflag =
      string_to_stdfloat(words[0], ip[0]) &&
      string_to_stdfloat(words[1], ip[1]) &&
      string_to_stdfloat(words[2], ip[2]) &&
      string_to_stdfloat(words[3], ip[3]);
    break;

  case 3:
    okflag =
      string_to_stdfloat(words[0], ip[0]) &&
      string_to_stdfloat(words[1], ip[1]) &&
      string_to_stdfloat(words[2], ip[2]);
    ip[3] = 1.0;
    break;

  case 2:
    okflag =
      string_to_stdfloat(words[0], ip[0]) &&
      string_to_stdfloat(words[1], ip[3]);
    ip[1] = ip[0];
    ip[2] = ip[0];
    break;

  case 1:
    okflag =
      string_to_stdfloat(words[0], ip[0]);
    ip[1] = ip[0];
    ip[2] = ip[0];
    ip[3] = 1.0;
    break;
  }

  if (!okflag) {
    nout << "-" << opt
         << " requires one through four numbers separated by commas.\n";
    return false;
  }

  return true;
}

/**
 * Standard dispatch function for an option that takes one parameter, which is
 * to be interpreted as a string.  The data pointer is to a string variable.
 */
bool ProgramBase::
dispatch_string(const string &, const string &arg, void *var) {
  string *ip = (string *)var;
  (*ip) = arg;

  return true;
}

/**
 * Standard dispatch function for an option that takes one parameter, which is
 * to be interpreted as a string.  This is different from dispatch_string in
 * that the parameter may be repeated multiple times, and each time the string
 * value is appended to a vector.
 *
 * The data pointer is to a vector_string variable.
 */
bool ProgramBase::
dispatch_vector_string(const string &, const string &arg, void *var) {
  vector_string *ip = (vector_string *)var;
  (*ip).push_back(arg);

  return true;
}

/**
 * Similar to dispatch_vector_string, but a comma is allowed to separate
 * multiple tokens in one argument, without having to repeat the argument for
 * each token.
 *
 * The data pointer is to a vector_string variable.
 */
bool ProgramBase::
dispatch_vector_string_comma(const string &, const string &arg, void *var) {
  vector_string *ip = (vector_string *)var;

  vector_string words;
  tokenize(arg, words, ",");

  vector_string::const_iterator wi;
  for (wi = words.begin(); wi != words.end(); ++wi) {
    (*ip).push_back(*wi);
  }

  return true;
}

/**
 * Standard dispatch function for an option that takes one parameter, which is
 * to be interpreted as a filename.  The data pointer is to a Filename
 * variable.
 */
bool ProgramBase::
dispatch_filename(const string &opt, const string &arg, void *var) {
  if (arg.empty()) {
    nout << "-" << opt << " requires a filename parameter.\n";
    return false;
  }

  Filename *ip = (Filename *)var;
  (*ip) = Filename::from_os_specific(arg);

  return true;
}

/**
 * Standard dispatch function for an option that takes one parameter, which is
 * to be interpreted as a single directory name to add to a search path.  The
 * data pointer is to a DSearchPath variable.  This kind of option may appear
 * multiple times on the command line; each time, the new directory is
 * appended.
 */
bool ProgramBase::
dispatch_search_path(const string &opt, const string &arg, void *var) {
  if (arg.empty()) {
    nout << "-" << opt << " requires a search path parameter.\n";
    return false;
  }

  DSearchPath *ip = (DSearchPath *)var;
  ip->append_directory(Filename::from_os_specific(arg));

  return true;
}

/**
 * Standard dispatch function for an option that takes one parameter, which is
 * to be interpreted as a coordinate system string.  The data pointer is to a
 * CoordinateSystem variable.
 */
bool ProgramBase::
dispatch_coordinate_system(const string &opt, const string &arg, void *var) {
  CoordinateSystem *ip = (CoordinateSystem *)var;
  (*ip) = parse_coordinate_system_string(arg);

  if ((*ip) == CS_invalid) {
    nout << "Invalid coordinate system for -" << opt << ": " << arg << "\n"
         << "Valid coordinate system strings are any of 'y-up', 'z-up', "
      "'y-up-left', or 'z-up-left'.\n";
    return false;
  }

  return true;
}

/**
 * Standard dispatch function for an option that takes one parameter, which is
 * to be interpreted as a unit of distance measurement.  The data pointer is
 * to a DistanceUnit variable.
 */
bool ProgramBase::
dispatch_units(const string &opt, const string &arg, void *var) {
  DistanceUnit *ip = (DistanceUnit *)var;
  (*ip) = string_distance_unit(arg);

  if ((*ip) == DU_invalid) {
    nout << "Invalid units for -" << opt << ": " << arg << "\n"
         << "Valid units are mm, cm, m, km, yd, ft, in, nmi, and mi.\n";
    return false;
  }

  return true;
}

/**
 * Standard dispatch function for an option that takes one parameter, which is
 * to indicate an image file type, like rgb, bmp, jpg, etc.  The data pointer
 * is to a PNMFileType pointer.
 */
bool ProgramBase::
dispatch_image_type(const string &opt, const string &arg, void *var) {
  PNMFileType **ip = (PNMFileType **)var;

  PNMFileTypeRegistry *reg = PNMFileTypeRegistry::get_global_ptr();

  (*ip) = reg->get_type_from_extension(arg);

  if ((*ip) == nullptr) {
    nout << "Invalid image type for -" << opt << ": " << arg << "\n"
         << "The following image types are known:\n";
    reg->write(nout, 2);
    return false;
  }

  return true;
}

/**
 * Standard dispatch function for an option that takes one parameter, which is
 * to be interpreted as a single component of a path replace request.  The
 * data pointer is to a PathReplace variable.
 */
bool ProgramBase::
dispatch_path_replace(const string &opt, const string &arg, void *var) {
  PathReplace *ip = (PathReplace *)var;
  size_t equals = arg.find('=');
  if (equals == string::npos) {
    nout << "Invalid path replacement string for -" << opt << ": " << arg << "\n"
         << "String should be of the form 'old-prefix=new-prefix'.\n";
    return false;
  }
  ip->add_pattern(arg.substr(0, equals), arg.substr(equals + 1));

  return true;
}

/**
 * Standard dispatch function for an option that takes one parameter, which is
 * to be interpreted as a path store string.  The data pointer is to a
 * PathStore variable.
 */
bool ProgramBase::
dispatch_path_store(const string &opt, const string &arg, void *var) {
  PathStore *ip = (PathStore *)var;
  (*ip) = string_path_store(arg);

  if ((*ip) == PS_invalid) {
    nout << "Invalid path store for -" << opt << ": " << arg << "\n"
         << "Valid path store strings are any of 'rel', 'abs', "
         << "'rel_abs', 'strip', or 'keep'.\n";
    return false;
  }

  return true;
}

/**
 * Called when the user enters '-h', this describes how to use the program and
 * then exits.
 */
bool ProgramBase::
handle_help_option(const string &, const string &, void *data) {
  ProgramBase *me = (ProgramBase *)data;
  me->show_description();
  me->show_usage();
  me->show_options();
  exit(0);

  return false;
}


/**
 * Word-wraps the indicated text to the indicated output stream.  The first
 * line is prefixed with the indicated prefix, then tabbed over to
 * indent_width where the text actually begins.  A newline is inserted at or
 * before column line_width.  Each subsequent line begins with indent_width
 * spaces.
 *
 * An embedded newline character ('\n') forces a line break, while an embedded
 * carriage-return character ('\r'), or two or more consecutive newlines,
 * marks a paragraph break, which is usually printed as a blank line.
 * Redundant newline and carriage-return characters are generally ignored.
 *
 * The flag last_newline should be initialized to false for the first call to
 * format_text, and then preserved for future calls; it tracks the state of
 * trailing newline characters between calls so we can correctly identify
 * doubled newlines.
 */
void ProgramBase::
format_text(std::ostream &out, bool &last_newline,
            const string &prefix, int indent_width,
            const string &text, int line_width) {
  indent_width = min(indent_width, line_width - 20);
  int indent_amount = indent_width;
  bool initial_break = false;

  if (!prefix.empty()) {
    out << prefix;
    indent_amount = indent_width - prefix.length();
    if ((int)prefix.length() + 1 > indent_width) {
      out << "\n";
      initial_break = true;
      indent_amount = indent_width;
    }
  }

  size_t p = 0;

  // Skip any initial whitespace and newlines.
  while (p < text.length() && isspace(text[p])) {
    if (text[p] == '\r' ||
        (p > 0 && text[p] == '\n' && text[p - 1] == '\n') ||
        (p == 0 && text[p] == '\n' && last_newline)) {
      if (!initial_break) {
        // Here's an initial paragraph break, however.
        out << "\n";
        initial_break = true;
      }
      indent_amount = indent_width;

    } else if (text[p] == '\n') {
      // Largely ignore an initial newline.
      indent_amount = indent_width;

    } else if (text[p] == ' ') {
      // Do count up leading spaces.
      indent_amount++;
    }
    p++;
  }

  last_newline = (!text.empty() && text[text.length() - 1] == '\n');

  while (p < text.length()) {
    // Look for the paragraph or line break--the next newline character, if
    // any.
    size_t par = text.find_first_of("\n\r", p);
    bool is_paragraph_break = false;
    if (par == string::npos) {
      par = text.length();
      /*
        This shouldn't be necessary.
    } else {
      is_paragraph_break = (text[par] == '\r');
      */
    }

    indent(out, indent_amount);

    size_t eol = p + (line_width - indent_width);
    if (eol >= par) {
      // The rest of the paragraph fits completely on the line.
      eol = par;

    } else {
      // The paragraph doesn't fit completely on the line.  Determine the best
      // place to break the line.  Look for the last space before the ideal
      // eol.
      size_t min_eol = max((int)p, (int)eol - 25);
      size_t q = eol;
      while (q > min_eol && !isspace(text[q])) {
        q--;
      }
      // Now roll back to the last non-space before this one.
      while (q > min_eol && isspace(text[q])) {
        q--;
      }

      if (q != min_eol) {
        // Here's a good place to stop!
        eol = q + 1;

      } else {
        // The line cannot be broken cleanly.  Just let it keep going; don't
        // try to wrap it.
        eol = par;
      }
    }
    out << text.substr(p, eol - p) << "\n";
    p = eol;

    // Skip additional whitespace between the lines.
    while (p < text.length() && isspace(text[p])) {
      if (text[p] == '\r' ||
          (p > 0 && text[p] == '\n' && text[p - 1] == '\n')) {
        is_paragraph_break = true;
      }
      p++;
    }

    if (eol == par && is_paragraph_break) {
      // Print the paragraph break as a blank line.
      out << "\n";
      if (p >= text.length()) {
        // If we end on a paragraph break, don't try to insert a new one in
        // the next pass.
        last_newline = false;
      }
    }

    indent_amount = indent_width;
  }
}


/**
 * Puts all the options in order by index number (e.g.  in the order they were
 * added, within index_groups), for output by show_options().
 */
void ProgramBase::
sort_options() {
  if (!_sorted_options) {
    _options_by_index.clear();

    OptionsByName::const_iterator oi;
    for (oi = _options_by_name.begin(); oi != _options_by_name.end(); ++oi) {
      _options_by_index.push_back(&(*oi).second);
    }

    sort(_options_by_index.begin(), _options_by_index.end(),
         SortOptionsByIndex());
    _sorted_options = true;
  }
}

/**
 * Attempts to determine the ideal terminal width for formatting output.
 */
void ProgramBase::
get_terminal_width() {
  if (!_got_terminal_width) {
    _got_terminal_width = true;
    _got_option_indent = false;

#ifdef IOCTL_TERMINAL_WIDTH
    if (use_terminal_width) {
      struct winsize size;
      int result = ioctl(STDIN_FILENO, TIOCGWINSZ, (char *)&size);
      if (result < 0 || size.ws_col < 10) {
        // Couldn't determine the width for some reason.  Instead of
        // complaining, just punt.
        _terminal_width = default_terminal_width;
      } else {

        // Subtract 10% for the comfort margin at the edge.
        _terminal_width = size.ws_col - min(8, (int)(size.ws_col * 0.1));
      }
      return;
    }
#endif  // IOCTL_TERMINAL_WIDTH

    _terminal_width = default_terminal_width;
  }
}
