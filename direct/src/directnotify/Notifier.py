"""
Notifier module: contains methods for handling information output
for the programmer/user
"""
from LoggerGlobal import defaultLogger
from direct.showbase import PythonUtil
import time
import types

class Notifier:
    serverDelta = 0

    # If this object is set to something, it is used to print output
    # messages instead of writing them to the console.  This is
    # particularly useful for integrating the Python notify system
    # with the C++ notify system.
    streamWriter = None
    showTime = 0

    def __init__(self, name, logger=None):
        """
        name is a string
        logger is a Logger

        Create a new instance of the Notifier class with a given name
        and an optional Logger class for piping output to. If no logger
        specified, use the global default
        """
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
        Notifier.serverDelta = delta + time.timezone - timezone

        # The following call is necessary to make the output from C++
        # notify messages show the same timestamp as those generated
        # from Python-level notify messages.
        from pandac.PandaModules import NotifyCategory
        NotifyCategory.setServerDelta(self.serverDelta)

        self.info("Notify clock adjusted by %s (and timezone adjusted by %s hours) to synchronize with server." % (PythonUtil.formatElapsedSeconds(delta), (time.timezone - timezone) / 3600))

    def getTime(self):
        """
        Return the time as a string suitable for printing at the
        head of any notify message
        """
        # for some strange reason, time.time() updates only once/minute if
        # the task is out of focus on win32.  time.clock doesn't have this problem.
        return time.strftime(":%m-%d-%Y %H:%M:%S ", time.localtime(time.time() + self.serverDelta))

    def getOnlyTime(self):
        """
        Return the time as a string.
        The Only in the name is referring to not showing the date.
        """
        return time.strftime("%H:%M:%S", time.localtime(time.time() + self.serverDelta))

    def __str__(self):
        """
        Print handling routine
        """
        return "%s: info = %d, warning = %d, debug = %d, logging = %d" % \
               (self.__name, self.__info, self.__warning, self.__debug, self.__logging)

    # Severity funcs
    def setSeverity(self, severity):
        from pandac.PandaModules import NSDebug, NSInfo, NSWarning, NSError
        if severity >= NSError:
            self.setWarning(0)
            self.setInfo(0)
            self.setDebug(0)
        elif severity == NSWarning:
            self.setWarning(1)
            self.setInfo(0)
            self.setDebug(0)
        elif severity == NSInfo:
            self.setWarning(1)
            self.setInfo(1)
            self.setDebug(0)
        elif severity <= NSDebug:
            self.setWarning(1)
            self.setInfo(1)
            self.setDebug(1)

    def getSeverity(self):
        from pandac.PandaModules import NSDebug, NSInfo, NSWarning, NSError
        if self.getDebug():
            return NSDebug
        elif self.getInfo():
            return NSInfo
        elif self.getWarning():
            return NSWarning
        else:
            return NSError

    # error funcs
    def error(self, errorString, exception=StandardError):
        """
        Raise an exception with given string and optional type:
        Exception: error
        """
        message = str(errorString)
        if Notifier.showTime:
            string = (self.getTime() + str(exception) + ": " + self.__name + "(error): " + message)
        else:
            string = (str(exception) + ": " + self.__name + "(error): " + message)
        self.__log(string)
        raise exception(errorString)

    # warning funcs
    def warning(self, warningString):
        """
        Issue the warning message if warn flag is on
        """
        if self.__warning:
            message = str(warningString)
            if Notifier.showTime:
                string = (self.getTime() + self.__name + '(warning): ' + message)
            else:
                string = (":" + self.__name + '(warning): ' + message)
            self.__log(string)
            self.__print(string)
        return 1 # to allow assert myNotify.warning("blah")

    def setWarning(self, bool):
        """
        Enable/Disable the printing of warning messages
        """
        self.__warning = bool

    def getWarning(self):
        """
        Return whether the printing of warning messages is on or off
        """
        return(self.__warning)

    # debug funcs
    def debug(self, debugString):
        """
        Issue the debug message if debug flag is on
        """
        if self.__debug:
            message = str(debugString)
            if Notifier.showTime:
                string = (self.getTime() + self.__name + '(debug): ' + message)
            else:
                string = (':' + self.__name + '(debug): ' + message)
            self.__log(string)
            self.__print(string)
        return 1 # to allow assert myNotify.debug("blah")

    def setDebug(self, bool):
        """
        Enable/Disable the printing of debug messages
        """
        self.__debug = bool

    def getDebug(self):
        """
        Return whether the printing of debug messages is on or off
        """
        return self.__debug

    # info funcs
    def info(self, infoString):
        """
        Print the given informational string, if info flag is on
        """
        if self.__info:
            message = str(infoString)
            if Notifier.showTime:
                string = (self.getTime() + self.__name + ': ' + message)
            else:
                string = (':' + self.__name + ': ' + message)
            self.__log(string)
            self.__print(string)
        return 1 # to allow assert myNotify.info("blah")

    def getInfo(self):
        """
        Return whether the printing of info messages is on or off
        """
        return self.__info

    def setInfo(self, bool):
        """
        Enable/Disable informational message  printing
        """
        self.__info = bool

    # log funcs
    def __log(self, logEntry):
        """
        Determine whether to send informational message to the logger
        """
        if self.__logging:
            self.__logger.log(logEntry)

    def getLogging(self):
        """
        Return 1 if logging enabled, 0 otherwise
        """
        return (self.__logging)

    def setLogging(self, bool):
        """
        Set the logging flag to int (1=on, 0=off)
        """
        self.__logging = bool

    def __print(self, string):
        """
        Prints the string to output followed by a newline.
        """
        if self.streamWriter:
            self.streamWriter.appendData(string + '\n')
        else:
            print string

    def debugStateCall(self, obj=None, fsmMemberName='fsm',
            secondaryFsm='secondaryFSM'):
        """
        If this notify is in debug mode, print the time of the
        call followed by the [fsm state] notifier category and
        the function call (with parameters).
        """
        #f.f_locals['self'].__init__.im_class.__name__
        if self.__debug:
            state = ''
            doId = ''
            if obj is not None:

                fsm=obj.__dict__.get(fsmMemberName)
                if fsm is not None:
                    stateObj = fsm.getCurrentState()
                    if stateObj is not None:
                        #state = "%s=%s"%(fsmMemberName, stateObj.getName())
                        state = stateObj.getName()

                fsm=obj.__dict__.get(secondaryFsm)
                if fsm is not None:
                    stateObj = fsm.getCurrentState()
                    if stateObj is not None:
                        #state = "%s=%s"%(fsmMemberName, stateObj.getName())
                        state = "%s, %s"%(state, stateObj.getName())

                if hasattr(obj, 'doId'):
                    doId = " doId:%s"%(obj.doId,)
            #if type(obj) == types.ClassType:
            #    name = "%s."%(obj.__class__.__name__,)
            string = ":%s:%s [%-7s] id(%s)%s %s"%(
                self.getOnlyTime(),
                self.__name,
                state,
                id(obj),
                doId,
                PythonUtil.traceParentCall())
            self.__log(string)
            self.__print(string)
        return 1 # to allow assert self.notify.debugStateCall(self)

    def debugCall(self, debugString=''):
        """
        If this notify is in debug mode, print the time of the
        call followed by the notifier category and
        the function call (with parameters).
        """
        if self.__debug:
            message = str(debugString)
            string = ":%s:%s \"%s\" %s"%(
                self.getOnlyTime(),
                self.__name,
                message,
                PythonUtil.traceParentCall())
            self.__log(string)
            self.__print(string)
        return 1 # to allow assert self.notify.debugCall("blah")

