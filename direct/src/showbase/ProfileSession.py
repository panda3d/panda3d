from pandac.libpandaexpressModules import TrueClock
from direct.showbase.PythonUtil import (
    StdoutCapture, _installProfileCustomFuncs,_removeProfileCustomFuncs,
    _profileWithoutGarbageLeak, _getProfileResultFileInfo, _setProfileResultsFileInfo,
    _clearProfileResultFileInfo, )
import __builtin__
import profile
import pstats
from StringIO import StringIO
import marshal

class ProfileSession:
    # class that encapsulates a profile of a single callable using Python's standard
    # 'profile' module
    #
    # defers formatting of profile results until they are requested
    # 
    # implementation sidesteps memory leak in Python profile module,
    # and redirects file output to RAM file for efficiency
    DefaultFilename = 'profilesession'
    DefaultLines = 80
    DefaultSorts = ['cumulative', 'time', 'calls']

    TrueClock = TrueClock.getGlobalPtr()
    
    def __init__(self, func, name):
        self._func = func
        self._name = name
        self._filename = 'profileData-%s-%s' % (self._name, id(self))
        self._profileSucceeded = False
        self._wallClockDur = None
        self._ramFile = None
        self._refCount = 0
        self._resultCache = {}
        self.acquire()

    def acquire(self):
        self._refCount += 1
    def release(self):
        self._refCount -= 1
        if not self._refCount:
            self._destroy()

    def _destroy(self):
        del self._func
        del self._name
        del self._filename
        del self._wallClockDur
        del self._ramFile
        del self._resultCache

    def run(self):
        # make sure this instance doesn't get destroyed inside self._func
        self.acquire()

        self._profileSucceeded = False

        # if we're already profiling, just run the func and don't profile
        if 'globalProfileFunc' in __builtin__.__dict__:
            result = self._func()
            self._wallClockDur = 0.
        else:
            # put the function in the global namespace so that profile can find it
            __builtin__.globalProfileFunc = self._func
            __builtin__.globalProfileResult = [None]

            # set up the RAM file
            _installProfileCustomFuncs(self._filename)

            # do the profiling
            Profile = profile.Profile
            statement = 'globalProfileResult[0]=globalProfileFunc()'
            sort = -1
            retVal = None

            # this is based on profile.run, the code is replicated here to allow us to
            # eliminate a memory leak
            prof = Profile()
            # try to get wall-clock duration that is as accurate as possible
            startT = self.TrueClock.getShortTime()
            try:
                prof = prof.run(statement)
            except SystemExit:
                pass
            # try to get wall-clock duration that is as accurate as possible
            endT = self.TrueClock.getShortTime()
            # this has to be run immediately after profiling for the timings to be accurate
            # tell the Profile object to generate output to the RAM file
            prof.dump_stats(self._filename)

            dur = endT - startT

            # eliminate the memory leak
            del prof.dispatcher

            # and store the results
            self._wallClockDur = dur

            # store the RAM file for later
            self._ramFile = _getProfileResultFileInfo(self._filename)
            # clean up the RAM file support
            _removeProfileCustomFuncs(self._filename)

            # clean up the globals
            result = globalProfileResult[0]
            del __builtin__.__dict__['globalProfileFunc']
            del __builtin__.__dict__['globalProfileResult']

            self._profileSucceeded = True
            
        self.release()
        return result

    def getWallClockDuration(self):
        # this might not be accurate, it may include time taken up by other processes
        return self._wallClockDur

    def profileSucceeded(self):
        return self._profileSucceeded
    
    def getResults(self,
                   lines=80,
                   sorts=['cumulative', 'time', 'calls'],
                   callInfo=False):
        if not self._profileSucceeded:
            output = '%s: profiler already running, could not profile' % self._name
        else:
            # make sure the arguments will hash efficiently if callers provide different types
            lines = int(lines)
            sorts = list(sorts)
            callInfo = bool(callInfo)
            k = str((lines, sorts, callInfo))
            if k in self._resultCache:
                # we've already created this output string, get it from the cache
                output = self._resultCache[k]
            else:
                # set up the RAM file
                _installProfileCustomFuncs(self._filename)
                # install the stored RAM file from self.run()
                _setProfileResultsFileInfo(self._filename, self._ramFile)

                # now extract human-readable output from the RAM file

                # capture print output to a string
                sc = StdoutCapture()

                # print the info to stdout
                s = pstats.Stats(self._filename)
                s.strip_dirs()
                for sort in sorts:
                    s.sort_stats(sort)
                    s.print_stats(lines)
                    if callInfo:
                        s.print_callees(lines)
                        s.print_callers(lines)

                # make a copy of the print output
                output = sc.getString()

                # restore stdout to what it was before
                sc.destroy()

                # clean up the RAM file support
                _removeProfileCustomFuncs(self._filename)

                # cache this result
                self._resultCache[k] = output

        return output
    
