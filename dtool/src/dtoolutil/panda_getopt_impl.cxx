/**
 * PANDA 3D SOFTWARE
 * Copyright (c) Carnegie Mellon University.  All rights reserved.
 *
 * All use of this software is subject to the terms of the revised BSD
 * license.  You should have received a copy of this license along
 * with this source code in a file named "LICENSE."
 *
 * @file panda_getopt_impl.cxx
 * @author drose
 * @date 2011-07-19
 */

#include "panda_getopt_impl.h"
#include "pvector.h"

#if defined(HAVE_GETOPT) && defined(HAVE_GETOPT_LONG_ONLY)
// If the system provides both of these functions, we don't need to provide
// our own implementation, so in that case this file does nothing.

#else
// If the system does lack one or the other of these functions, then we'll go
// ahead and provide it instead.

using std::string;

char *optarg = nullptr;
int optind = 0;
int opterr = 1;
int optopt = 0;

/**
 * The implementation within this file of the various getopt() functions.
 * This class is not visible outside of this file; instead, the interface is
 * via the getopt() functions themselves.
 */
class PandaGetopt {
public:
  PandaGetopt(int argc, char *const argv[], const char *optstring,
              const struct option *longopts, bool allow_one_hyphen_long);

  void permute(int argc, char **mutable_argv);
  int process(int opterr, int *longindex, char *&optarg, int &optind, int &optopt);

private:
  size_t find_short_option(char short_option);
  size_t find_long_option(const string &long_option);

  void scan_options(const char *optstring, const struct option *longopts);
  void scan_args(int argc, char *const argv[]);

  // We build a list of Options, which correspond to the input defined in
  // optstring andor in the longopts list.  These are the short and long
  // options that are available, whether or not the user tries to use any of
  // them.  This list is populated by scan_options().
  class Option {
  public:
    Option(char short_option, int has_arg);
    Option(const struct option *longopts, int longopts_index);

    char _short_option;
    string _long_option;
    int _has_arg;
    const struct option *_option;
    int _longopts_index;
  };

  // We next build a list of Params, which are the parameter options that are
  // parsed out of the argv array--those options that the user has actually
  // specified.  This list does not contain the non-option arguments, the
  // words that follow the options on the command line (those end up in the
  // _arguments list instead).  This list is populated by scan_args().
  class Param {
  public:
    Param(size_t opt_index, size_t argv_index,
          char short_option, char *argument = nullptr);

    size_t _opt_index;
    size_t _argv_index;
    char _short_option;
    char *_argument;
  };

  // The list of available options.
  typedef pvector<Option> Options;
  Options _options;

  // The list of invoked options.
  typedef pvector<Param> Params;
  Params _params;

  typedef pvector<char *> Arguments;

  // The list of option arguments on the command line, with pointers back into
  // the original argv array.  This is similar to the _params list, above, but
  // it is the pointers to the original unprocessed strings.  We use this list
  // to premute the argv array into proper order if needed.
  Arguments _output_argv;

  // The list of non-option arguments on the command line, following the
  // options.  The vector contains the actual pointers back into the original
  // argv array; we use it to permute the argv array into proper order if
  // needed.
  Arguments _arguments;

  // See the PandaGetopt constructor for an explanation of these two flags.
  bool _return_in_order;
  bool _require_order;

  // If we are invoked via getopt_long_only(), then a single hyphen is allowed
  // to introduce a long option, as well as a double hyphen.
  bool _allow_one_hyphen_long;

  // This member is used to hold our place in the parameters list across
  // multiple calls to process().
  size_t _next_param;

  // This is the index of the first non-option argument in the argv list.
  // It's filled into optind when process() reaches the end of its processing.
  size_t _next_argv_index;
};

// This global pointer is used to differentiate between getopt() being called
// the first time, vs.  subsequent times.
static PandaGetopt *pgetopt = nullptr;

/**
 *
 */
PandaGetopt::
PandaGetopt(int argc, char *const argv[], const char *optstring,
            const struct option *longopts, bool allow_one_hyphen_long) {
  assert(optstring != nullptr);

  _return_in_order = false;
  _require_order = false;
  _allow_one_hyphen_long = allow_one_hyphen_long;
  _next_param = 0;

  // _options[0] is used for invalid characters.
  _options.push_back(Option('?', no_argument));

  if (optstring[0] == '-') {
    // RETURN_IN_ORDER: Non-option arguments (operands) are handled as if they
    // were the argument to an option with the value 1 ('\001').
    ++optstring;
    _return_in_order = true;

    // _options[1] is option '\001'.
    _options.push_back(Option('\001', required_argument));

  } else if (optstring[0] == '+') {
    // REQUIRE_ORDER: option processing stops when the first non-option
    // argument is reached, or when the element of argv is "--".
    ++optstring;
    _require_order = true;

  } else if (getenv("POSIXLY_CORRECT") != nullptr) {
    // REQUIRE_ORDER.
    _require_order = true;

  } else {
    // PERMUTE: the order of arguments in argv is altered so that all options
    // (and their arguments) are moved in front of all of the operands.
  }

  scan_options(optstring, longopts);
  scan_args(argc, argv);
}

/**
 * Permutes the argv array so that the non-option arguments are at the end of
 * the list (if POSIXLY_CORRECT is not set), as the gnu implementation does.
 */
void PandaGetopt::
permute(int argc, char **mutable_argv) {
  if (!_require_order && !_return_in_order) {
    // Rebuild the argv array to reflect the reordered options.
    size_t i = 1;
    Arguments::const_iterator gi;
    for (gi = _output_argv.begin(); gi != _output_argv.end(); ++gi) {
      assert((int)i < argc);
      mutable_argv[i] = (*gi);
      ++i;
    }
    _next_argv_index = i;
    for (gi = _arguments.begin(); gi != _arguments.end(); ++gi) {
      assert((int)i < argc);
      mutable_argv[i] = (*gi);
      ++i;
    }
    assert((int)i == argc);
  }
}

/**
 * Can be called repeatedly to extract out the option arguments scanned from
 * the argv list, one at a time.  Sets *longindex, optarg, optind, optopt.
 * Returns EOF when finished.
 */
int PandaGetopt::
process(int opterr, int *longindex, char *&optarg, int &optind, int &optopt) {
  if (_next_param >= _params.size()) {
    optind = _next_argv_index;
    return EOF;
  }

  const Param &param = _params[_next_param];
  ++_next_param;
  const Option &option = _options[param._opt_index];

  optarg = param._argument;
  optind = (int)param._argv_index;
  if (longindex != nullptr) {
    *longindex = option._longopts_index;
  }

  if (option._option != nullptr) {
    // This was a long option.  Check the special longopt handling parameters.
    if (option._option->flag == nullptr) {
      return option._option->val;
    }
    *(option._option->flag) = option._option->val;
    return 0;
  }

  if (param._opt_index == 0 && opterr) {
    // This was an invalid character.
    optopt = param._short_option;
    std::cerr << "Illegal option: -" << param._short_option << "\n";
    return '?';
  }

  // This was a short option.  Return the option itself.
  return param._short_option;
}

/**
 * Returns the index within the _options array of the option with the
 * indicated short_option letter, or 0 if the option is not found.
 */
size_t PandaGetopt::
find_short_option(char short_option) {
  size_t opt_index = 1;
  while (opt_index < _options.size()) {
    if (_options[opt_index]._short_option == short_option) {
      return opt_index;
    }
    ++opt_index;
  }

  return 0;
}

/**
 * Returns the index within the _options array of the option with the
 * indicated long_option word, or 0 if the option is not found.  If the word
 * contains an '=' sign, only the text before this sign is considered.
 */
size_t PandaGetopt::
find_long_option(const string &long_option) {
  string search = long_option;
  size_t equals = search.find('=');
  if (equals != string::npos) {
    search = search.substr(0, equals);
  }

  size_t opt_index = 1;
  while (opt_index < _options.size()) {
    if (_options[opt_index]._long_option == search) {
      return opt_index;
    }
    ++opt_index;
  }

  return 0;
}

/**
 * Parses the optstring and longopts list to understand the options we should
 * be searching for, and populate the internal _options array.
 */
void PandaGetopt::
scan_options(const char *optstring, const struct option *longopts) {
  const char *p = optstring;
  while (*p != '\0') {
    char short_option = *p;
    int has_arg = no_argument;
    ++p;
    if (*p == ':') {
      has_arg = required_argument;
      ++p;
      if (*p == ':') {
        has_arg = optional_argument;
        ++p;
      }
    }

    _options.push_back(Option(short_option, has_arg));
  }

  if (longopts != nullptr) {
    int longopts_index = 0;
    while (longopts[longopts_index].name != nullptr) {
      _options.push_back(Option(longopts, longopts_index));
      ++longopts_index;
    }
  }
}

/**
 * Parses the argv list to understand the arguments passed by the user, and
 * populates the _params and _arguments arrays.
 */
void PandaGetopt::
scan_args(int argc, char *const argv[]) {
  size_t ai = 1;
  bool end_of_processing = false;

  while ((int)ai < argc) {
    assert(argv[ai] != nullptr);

    if (argv[ai][0] != '-' || end_of_processing) {
      // This is a non-option argument.
      if (_require_order) {
        break;
      }
      if (_return_in_order) {
        // Record it as an argument of _options[1], which is '\001'.
        _params.push_back(Param(1, ai, '\001', argv[ai]));
        _output_argv.push_back(argv[ai]);
      } else {
        // Push the non-option argument onto its list, and continue scanning.
        _arguments.push_back(argv[ai]);
      }

    } else if (strcmp(argv[ai], "--") == 0) {
      // Special case: this ends processing.  Everything after this is a non-
      // option argument.
      _output_argv.push_back(argv[ai]);
      end_of_processing = true;

    } else {
      // An option argument.

      char *option = nullptr;
      char *argument = nullptr;
      size_t opt_index = 0;
      bool is_long_option = false;
      bool has_argument = false;

      if (argv[ai][1] == '-') {
        // This is a long option.
        option = argv[ai] + 2;
        opt_index = find_long_option(option);
        is_long_option = true;
      } else {
        // This is one or more short options, or a short option and its
        // argument.
        option = argv[ai] + 1;
        if (_allow_one_hyphen_long) {
          // Or maybe it's a long option.
          opt_index = find_long_option(option);
          if (opt_index != 0) {
            is_long_option = true;
          }
        }
        if (!is_long_option) {
          opt_index = find_short_option(option[0]);
          while (opt_index != 0 &&
                 _options[opt_index]._has_arg == no_argument &&
                 option[1] != '\0') {
            // There are multiple short options jammed into a single word.
            _params.push_back(Param(opt_index, ai, option[0]));
            ++option;
            opt_index = find_short_option(option[0]);
          }

          if (opt_index != 0 && _options[opt_index]._has_arg != no_argument) {
            if (option[1] != '\0') {
              // There's an argument embedded in the same word.
              argument = option + 1;
              has_argument = true;
            }
          }
        }
      }

      if (is_long_option) {
        char *equals = strchr(option, '=');
        if (equals != nullptr) {
          argument = equals + 1;
          has_argument = true;
        }
      }

      size_t argv_index = ai;

      if (opt_index != 0 && _options[opt_index]._has_arg == required_argument &&
          !has_argument) {
        // Check the next word for an argument.
        _output_argv.push_back(argv[ai]);
        ++ai;
        if ((int)ai < argc) {
          argument = argv[ai];
          has_argument = true;
        }
      }

      _params.push_back(Param(opt_index, argv_index, option[0], argument));
      _output_argv.push_back(argv[ai]);
    }
    ++ai;
  }

  _next_argv_index = ai;

  // Now record the non-option arguments that followed the option arguments.
  while ((int)ai < argc) {
    assert(argv[ai] != nullptr);
    _arguments.push_back(argv[ai]);
    ++ai;
  }
}

/**
 * The constructor for a short_option.  Receives the letter that is the short
 * option, and one of no_argument, required_argument, or optional_argument.
 */
PandaGetopt::Option::
Option(char short_option, int has_arg) :
  _short_option(short_option),
  _has_arg(has_arg),
  _option(nullptr),
  _longopts_index(-1)
{
}

/**
 * The constructor for a long_option.  Receives the longopts array and the
 * index within the array for this particular option.
 */
PandaGetopt::Option::
Option(const struct option *longopts, int longopts_index) :
  _short_option(0),
  _long_option(longopts[longopts_index].name),
  _has_arg(longopts[longopts_index].has_arg),
  _option(&longopts[longopts_index]),
  _longopts_index(longopts_index)
{
}

/**
 *
 */
PandaGetopt::Param::
Param(size_t opt_index, size_t argv_index, char short_option, char *argument) :
  _opt_index(opt_index),
  _argv_index(argv_index),
  _short_option(short_option),
  _argument(argument)
{
}

int
getopt(int argc, char *const argv[], const char *optstring) {
  if (pgetopt == nullptr) {
    pgetopt = new PandaGetopt(argc, argv, optstring, nullptr, false);
    pgetopt->permute(argc, (char **)argv);
  }
  return pgetopt->process(opterr, nullptr, optarg, optind, optopt);
}

int
getopt_long(int argc, char *const argv[], const char *optstring,
            const struct option *longopts, int *longindex) {
  if (pgetopt == nullptr) {
    pgetopt = new PandaGetopt(argc, argv, optstring, longopts, false);
    pgetopt->permute(argc, (char **)argv);
  }
  return pgetopt->process(opterr, longindex, optarg, optind, optopt);
}

int
getopt_long_only(int argc, char *const argv[], const char *optstring,
                 const struct option *longopts, int *longindex) {
  if (pgetopt == nullptr) {
    pgetopt = new PandaGetopt(argc, argv, optstring, longopts, true);
    pgetopt->permute(argc, (char **)argv);
  }
  return pgetopt->process(opterr, longindex, optarg, optind, optopt);
}

#endif  // defined(HAVE_GETOPT) && defined(HAVE_GETOPT_LONG_ONLY)
