"""Notifier module: contains methods for handling information output
   for the programmer/user"""

from LoggerGlobal import *
import time

class Notifier:
    serverDelta = 0

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

        # Global default levels are initialized here
        self.__info = 1
        self.__warning = 1
        self.__debug = 0
        self.__logging = 0

    def setServerDelta(self, delta, timezone):
        """
        Call this method on any Notify object to globally change the
        timestamp printed for each line of all Notify objects.

        This synchronizes the timestamp with the server's known time
        of day, and also switches into the server's timezone.
        """
        delta = int(round(delta))
        Notifier.serverDelta = delta + timezone - time.timezone

        import NotifyCategory
        # Temporary try .. except for old Panda.
        try:
            NotifyCategory.NotifyCategory.setServerDelta(self.serverDelta)
        except:
            pass
            
        self.info("Notify clock adjusted by %s seconds (and timezone adjusted by %s hours) to synchronize with server." % (delta, (timezone - time.timezone) / 3600))

    def getTime(self):
        """
        Return the time as a string suitable for printing at the
        head of any notify message
        """

        # for some strange reason, time.time() updates only once/minute if
        # the task is out of focus on win32.  time.clock doesnt have this prob
        return time.strftime(":%m-%d-%Y %H:%M:%S ", time.localtime(time.time() + self.serverDelta))

    def __str__(self):
        """__str__(self)
        Print handling routine"""
        return "%s: info = %d, warning = %d, debug = %d, logging = %d" % \
               (self.__name, self.__info, self.__warning, self.__debug, self.__logging)

    # Severity funcs
    def setSeverity(self, severity):
        import NotifySeverity
        if severity >= NotifySeverity.NSError:
            self.setWarning(0)
            self.setInfo(0)
            self.setDebug(0)
        elif severity == NotifySeverity.NSWarning:
            self.setWarning(1)
            self.setInfo(0)
            self.setDebug(0)
        elif severity == NotifySeverity.NSInfo:
            self.setWarning(1)
            self.setInfo(1)
            self.setDebug(0)
        elif severity <= NotifySeverity.NSDebug:
            self.setWarning(1)
            self.setInfo(1)
            self.setDebug(1)

    def getSeverity(self):
        import NotifySeverity
        if self.getDebug():
            return NotifySeverity.NSDebug
        elif self.getInfo():
            return NotifySeverity.NSInfo
        elif self.getWarning():
            return NotifySeverity.NSWarning
        else:
            return NotifySeverity.NSError

    # error funcs
    def error(self, errorString, exception=StandardError):
        """error(self, string, Exception=StandardError)
        Raise an exception with given string and optional type:
        Exception: error"""
        string = (self.getTime() + str(exception) + ": " + self.__name + ": " + errorString)
        self.__log(string)
        raise exception(errorString)

    # warning funcs
    def warning(self, warningString):
        """warning(self, string)
        Issue the warning message if warn flag is on"""
        if (self.__warning):
            string = (self.getTime() + self.__name + '(warning): ' + warningString)
            self.__log(string)
            print(string)
        return 1 # to allow assert(myNotify.warning("blah"))

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
            string = (self.getTime() + self.__name + '(debug): ' + debugString)
            self.__log(string)
            print(string)
        return 1 # to allow assert(myNotify.debug("blah"))

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
        Print the given informational string, if info flag is on"""
        if (self.__info):
            string = (self.getTime() + self.__name + '(info): ' + infoString)
            self.__log(string)
            print(string)
        return 1 # to allow assert(myNotify.info("blah"))

    def getInfo(self):
        """getInfo(self)
        Return whether the printing of info messages is on or off"""
        return(self.__info)

    def setInfo(self, bool):
        """setInfo(self, int)
        Enable/Disable informational message  printing"""
        self.__info = bool

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









