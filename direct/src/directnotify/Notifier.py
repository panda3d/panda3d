"""
Notifier module: contains methods for handling information output
for the programmer/user
"""

from __future__ import annotations

from .Logger import Logger
from .LoggerGlobal import defaultLogger
from direct.showbase import PythonUtil
from panda3d.core import ConfigVariableBool, NotifyCategory, StreamWriter, Notify
import time
import sys
from typing import NoReturn


class NotifierException(Exception):
    pass


class Notifier:
    serverDelta = 0

    # If this object is set to something, it is used to print output
    # messages instead of writing them to the console.  This is
    # particularly useful for integrating the Python notify system
    # with the C++ notify system.
    streamWriter: StreamWriter | None = None
    if ConfigVariableBool('notify-integrate', True):
        streamWriter = StreamWriter(Notify.out(), False)

    showTime = ConfigVariableBool('notify-timestamp', False)

    def __init__(self, name: str, logger: Logger | None = None) -> None:
        """
        Parameters:
            name (str): a string name given to this Notifier instance.
            logger (Logger, optional): an optional Logger object for
                piping output to.  If none is specified, the global
                :data:`~.LoggerGlobal.defaultLogger` is used.
        """
        self.__name = name

        if logger is None:
            self.__logger = defaultLogger
        else:
            self.__logger = logger

        # Global default levels are initialized here
        self.__info = True
        self.__warning = True
        self.__debug = False
        self.__logging = False

    def setServerDelta(self, delta: float, timezone: int) -> None:
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
        NotifyCategory.setServerDelta(self.serverDelta)

        self.info("Notify clock adjusted by %s (and timezone adjusted by %s hours) to synchronize with server." % (PythonUtil.formatElapsedSeconds(delta), (time.timezone - timezone) / 3600))

    def getTime(self) -> str:
        """
        Return the time as a string suitable for printing at the
        head of any notify message
        """
        # for some strange reason, time.time() updates only once/minute if
        # the task is out of focus on win32.  time.clock doesn't have this problem.
        return time.strftime(":%m-%d-%Y %H:%M:%S ", time.localtime(time.time() + self.serverDelta))

    def getOnlyTime(self) -> str:
        """
        Return the time as a string.
        The Only in the name is referring to not showing the date.
        """
        return time.strftime("%H:%M:%S", time.localtime(time.time() + self.serverDelta))

    def __str__(self) -> str:
        """
        Print handling routine
        """
        return "%s: info = %d, warning = %d, debug = %d, logging = %d" % \
               (self.__name, self.__info, self.__warning, self.__debug, self.__logging)

    # Severity funcs
    def setSeverity(self, severity: int) -> None:
        from panda3d.core import NSDebug, NSInfo, NSWarning, NSError
        if severity >= NSError:
            self.setWarning(False)
            self.setInfo(False)
            self.setDebug(False)
        elif severity == NSWarning:
            self.setWarning(True)
            self.setInfo(False)
            self.setDebug(False)
        elif severity == NSInfo:
            self.setWarning(True)
            self.setInfo(True)
            self.setDebug(False)
        elif severity <= NSDebug:
            self.setWarning(True)
            self.setInfo(True)
            self.setDebug(True)

    def getSeverity(self) -> int:
        from panda3d.core import NSDebug, NSInfo, NSWarning, NSError
        if self.getDebug():
            return NSDebug
        elif self.getInfo():
            return NSInfo
        elif self.getWarning():
            return NSWarning
        else:
            return NSError

    # error funcs
    def error(self, errorString: object, exception: type[Exception] = NotifierException) -> NoReturn:
        """
        Raise an exception with given string and optional type:
        Exception: error
        """
        message = str(errorString)
        if Notifier.showTime:
            string = f'{self.getTime()}{exception!s}: {self.__name}(error): {message}'
        else:
            string = f'{exception!s}: {self.__name}(error): {message}'
        self.__log(string)
        raise exception(errorString)

    # warning funcs
    def warning(self, warningString: object) -> int:
        """
        Issue the warning message if warn flag is on
        """
        if self.__warning:
            message = str(warningString)
            if Notifier.showTime:
                string = f'{self.getTime()}{self.__name}(warning): {message}'
            else:
                string = f':{self.__name}(warning): {message}'
            self.__log(string)
            self.__print(string)
        return 1 # to allow assert myNotify.warning("blah")

    def setWarning(self, enable: bool) -> None:
        """
        Enable/Disable the printing of warning messages
        """
        self.__warning = enable

    def getWarning(self) -> bool:
        """
        Return whether the printing of warning messages is on or off
        """
        return self.__warning

    # debug funcs
    def debug(self, debugString: object) -> int:
        """
        Issue the debug message if debug flag is on
        """
        if self.__debug:
            message = str(debugString)
            if Notifier.showTime:
                string = f'{self.getTime()}{self.__name}(debug): {message}'
            else:
                string = f':{self.__name}(debug): {message}'
            self.__log(string)
            self.__print(string)
        return 1 # to allow assert myNotify.debug("blah")

    def setDebug(self, enable: bool) -> None:
        """
        Enable/Disable the printing of debug messages
        """
        self.__debug = enable

    def getDebug(self) -> bool:
        """
        Return whether the printing of debug messages is on or off
        """
        return self.__debug

    # info funcs
    def info(self, infoString: object) -> int:
        """
        Print the given informational string, if info flag is on
        """
        if self.__info:
            message = str(infoString)
            if Notifier.showTime:
                string = f'{self.getTime()}{self.__name}: {message}'
            else:
                string = f':{self.__name}: {message}'
            self.__log(string)
            self.__print(string)
        return 1 # to allow assert myNotify.info("blah")

    def getInfo(self) -> bool:
        """
        Return whether the printing of info messages is on or off
        """
        return self.__info

    def setInfo(self, enable: bool) -> None:
        """
        Enable/Disable informational message  printing
        """
        self.__info = enable

    # log funcs
    def __log(self, logEntry: str) -> None:
        """
        Determine whether to send informational message to the logger
        """
        if self.__logging:
            self.__logger.log(logEntry)

    def getLogging(self) -> bool:
        """
        Return 1 if logging enabled, 0 otherwise
        """
        return self.__logging

    def setLogging(self, enable: bool) -> None:
        """
        Set the logging flag to int (1=on, 0=off)
        """
        self.__logging = enable

    def __print(self, string: str) -> None:
        """
        Prints the string to output followed by a newline.
        """
        if self.streamWriter:
            self.streamWriter.write(string + '\n')
        else:
            sys.stderr.write(string + '\n')

    def debugStateCall(self, obj=None, fsmMemberName='fsm',
            secondaryFsm='secondaryFSM'):
        """
        If this notify is in debug mode, print the time of the
        call followed by the [fsm state] notifier category and
        the function call (with parameters).
        """
        #f.f_locals['self'].__init__.im_class.__name__
        if __debug__ and self.__debug:
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
                    doId = f" doId:{obj.doId}"
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

    def debugCall(self, debugString: object = '') -> int:
        """
        If this notify is in debug mode, print the time of the
        call followed by the notifier category and
        the function call (with parameters).
        """
        if __debug__ and self.__debug:
            message = str(debugString)
            string = ":%s:%s \"%s\" %s"%(
                self.getOnlyTime(),
                self.__name,
                message,
                PythonUtil.traceParentCall())
            self.__log(string)
            self.__print(string)
        return 1 # to allow assert self.notify.debugCall("blah")
