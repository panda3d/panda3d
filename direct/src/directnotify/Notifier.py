"""Notifier module: contains methods for handling information output
   for the programmer/user"""

from LoggerGlobal import *

class Notifier:

    def __init__(self, name, logger=None):
        """__init__(self, string, Logger=None)
        Create a new instance of the Notifier class with a given name
        and an optional Logger class for piping output to. If no logger
        specified, use the global default"""
        self.__name = name

        if (logger==None):
            self.__logger = defaultLogger
        else:
            self.__logger = logger

        self.__verbose = 0
        self.__warning = 1
        self.__debug = 0
        self.__logging = 0

    def __str__(self):
        """__str__(self)
        Print handling routine"""
        return "%s: verbose = %d, warning = %d, debug = %d, logging = %d" % \
               (self.__name, self.__verbose, self.__warning, self.__debug, self.__logging)
    
        
    # error funcs
    def error(self, errorString, exception=StandardError):
        """error(self, string, Exception=StandardError)
        Raise an exception with given string and optional type:
        Exception: error"""
        self.__log(str(exception) + ": " + self.__name + ": " + errorString)
        raise exception(errorString)


    # warning funcs
    def warning(self, warningString):
        """warning(self, string)
        Issue the warning message if warn flag is on"""
        if (self.__warning):
            str = ':' + self.__name + '(warning): ' + warningString
            self.__log(str)
            print(str)

    def setWarning(self, bool):
        """setWarning(self, int)
        Enable/Disable the printing of warning messages"""
        self.__warning = bool

    def getWarning(self):
        """getWarning(self)
        Return whether the printing of warning messages is on or off"""
        return(self.__warning)



    # debug funcs
    def debug(self, debugString):
        """debug(self, string)
        Issue the debug message if debug flag is on"""
        if (self.__debug):
            str = ':' + self.__name + '(debug): ' + debugString
            self.__log(str)
            print(str)

    def setDebug(self, bool):
        """setDebug(self, int)
        Enable/Disable the printing of debug messages"""
        self.__debug = bool

    def getDebug(self):
        """getDebug(self)
        Return whether the printing of debug messages is on or off"""
        return(self.__debug)



    # info funcs
    def info(self, infoString):
        """info(self, string)
        Print the given informational string, if verbose flag is on"""
        if (self.__verbose):
            str = ':' + self.__name + '(info): ' + infoString
            self.__log(str)
            print(str)

    def getVerbose(self):
        """getVerbose(self)
        Return whether the printing of info messages is on or off"""
        return(self.__verbose)

    def setVerbose(self, bool):
        """setVerbose(self, int)
        Enable/Disable informational message  printing"""
        self.__verbose = bool


    # log funcs
    def __log(self, logEntry):
        """__log(self, string)
        Determine whether to send informational message to the logger"""
        if (self.__logging):
            self.__logger.log(logEntry)

    def getLogging(self):
        """getLogging(self)
        Return 1 if logging enabled, 0 otherwise"""
        return (self.__logging)

    def setLogging(self, bool):
        """setLogging(self, int)
        Set the logging flag to int (1=on, 0=off)"""
        self.__logging = bool









