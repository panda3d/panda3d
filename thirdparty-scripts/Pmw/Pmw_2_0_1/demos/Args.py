"""Handle command line arguments.

This module contains functions to parse and access the arguments given
to the program on the command line.
"""

import types
import string
import sys

# Symbolic constants for the indexes into an argument specifier tuple.
NAME        = 0
MANDATORY   = 1
TYPE        = 2
HELP        = 3
DEFAULT     = 4
SPEC_LENGTH = 5

Bool = []

helpSpec = (
  ('help',            0, Bool,   'print help and exit'),
)

def parseArgs(title, argv, argSpecs, filesOK):
    """Parse and check command line arguments.

    Scan the command line arguments in *argv* according to the argument
    specifier *argSpecs*. Return **None** if there are no errors in
    the arguments, otherwise return an error string describing the error.

    This function must be called to initialise this module.

    title    -- The name of the program. This is used when returning
                error messages or help text.

    argv     -- A sequence containing the arguments given to the program.
                Normally **sys.argv**.

    argSpecs -- A sequence of argument specifiers.  Each specifier describes
                a valid command line argument and consists of 4 or 5 items:

                - The argument name (without a leading minus sign **-**).

                - A boolean value, true if the argument is mandatory.

                - This should be **Args.Bool** if the argument has no option.
                  Otherwise it should be a string describing the option
                  required for this argument. This is used when printing help.

                - A short string describing the argument.

                - The default value of the argument.  This should only be used
                  for non-mandatory arguments expecting an option.

                For example:
                  (
                    ('foreground', 0, 'colour',    'colour of text', 'black'),
                    ('geometry',   0, 'spec',      'geometry of initial window'),
                    ('server',     1, 'ompserver', 'ompserver to connect to'),
                    ('silent',     0, Args.Bool,   'do not sound bell'),
                  )
    """

    global programName
    global _fileList

    errMsg = title + ' command line error: '
    programName = argv[0];

    argSpecs = helpSpec + argSpecs
    argSpecDic = {}
    for spec in argSpecs:
        arg = spec[NAME]
        argSpecDic[arg] = spec
        if len(spec) >= SPEC_LENGTH:
            set(arg, spec[DEFAULT])
        elif spec[TYPE] is Bool:
            set(arg, 0)
        else:
            set(arg, None)

    knownKeys = list(argSpecDic.keys())

    i = 1
    _fileList = []
    argc = len(argv)
    while i < argc:
        arg = argv[i]
        key = arg[1:]
        if key in knownKeys:
            spec = argSpecDic[key]
            if spec[TYPE] is Bool:
                set(key, 1)
            else:
                i = i + 1
                if i >= argc:
                    return errMsg + 'missing argument to \'' + arg + '\' option.'
                value = argv[i]
                if len(spec) >= SPEC_LENGTH:
                    try:
                        if type(spec[DEFAULT]) == int:
                            typeStr = 'integer'
                            value = string.atoi(value)
                        elif type(spec[DEFAULT]) == float:
                            typeStr = 'float'
                            value = string.atof(value)
                    except:
                        sys.exc_info()[2] = None   # Clean up object references
                        return errMsg + 'cannot convert string \'' + value + \
                          '\' to ' + typeStr + ' for option \'-' + key + '\'.'
                set(key, value)
        else:
            _fileList.append(arg)
        i = i + 1

    if get('help'):
        return _helpString(title, argSpecs)

    if not filesOK and len(_fileList) > 0:
        if len(_fileList) == 1:
            return errMsg + 'unknown option \'' + str(_fileList[0]) + '\'.'
        else:
            return errMsg + 'unknown options ' + str(_fileList) + '.'


    _missing = []
    for spec in argSpecs:
        if spec[MANDATORY] and get(spec[NAME]) is None:
            _missing.append(spec[NAME])
    if len(_missing) == 1:
        return errMsg + 'required argument \'-' + \
          str(_missing[0]) + '\' is missing.'
    elif len(_missing) > 1:
        return errMsg + 'required arguments ' + \
          str(['-' + s for s in _missing]) + ' are missing.'

    return None

def fileList():
    return _fileList

def _helpString(title, argSpecs):
    max = 0
    for spec in argSpecs:
        if spec[TYPE] is Bool:
            width = len(spec[NAME]) + 1
        else:
            width = len(spec[NAME]) + 4 + len(spec[TYPE])
        if width > max:
            max = width

    rtn = title + ' command line arguments:'
    format = '\n  %-' + str(max) + 's %s'
    for mandatory in (1, 0):
        needHeader = 1
        for spec in argSpecs:
            if mandatory and spec[MANDATORY] or not mandatory and not spec[MANDATORY]:
                if needHeader:
                    if mandatory:
                        rtn = rtn + '\n Mandatory arguments:'
                    else:
                        rtn = rtn + '\n Optional arguments (defaults in parentheses):'
                    needHeader = 0
                if spec[TYPE] is Bool:
                    arg = '-%s' % spec[NAME]
                else:
                    arg = '-%s <%s>' % (spec[NAME], spec[TYPE])
                if len(spec) >= SPEC_LENGTH:
                    if type(spec[DEFAULT]) == bytes:
                        definition = spec[HELP] + ' (' + spec[DEFAULT] + ')'
                    else:
                        definition = spec[HELP] + ' (' + str(spec[DEFAULT]) + ')'
                else:
                    definition = spec[HELP]
                rtn = rtn + format % (arg, definition)

    return rtn

def exists(key):
    return key in configDict

def get(key):
    return configDict[key]

def set(key, value):
    global configDict

    configDict[key] = value

configDict = {}
