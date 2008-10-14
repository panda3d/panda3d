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
        self._filenameBase = 'profileData-%s-%s' % (self._name, id(self))
        self._refCount = 0
        self._reset()
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
        del self._filenameBase
        del self._filenameCounter
        del self._filenames
        del self._duration
        del self._filename2ramFile
        del self._resultCache
        del self._successfulProfiles

    def _reset(self):
        self._filenameCounter = 0
        self._filenames = []
        # index of next file to be added to stats object
        self._statFileCounter = 0
        self._successfulProfiles = 0
        self._duration = None
        self._filename2ramFile = {}
        self._stats = None
        self._resultCache = {}

    def _getNextFilename(self):
        filename = '%s-%s' % (self._filenameBase, self._filenameCounter)
        self._filenameCounter += 1
        return filename

    def run(self, aggregate=True):
        # make sure this instance doesn't get destroyed inside self._func
        self.acquire()

        if not aggregate:
            self._reset()

        # if we're already profiling, just run the func and don't profile
        if 'globalProfileFunc' in __builtin__.__dict__:
            result = self._func()
            if self._duration is None:
                self._duration = 0.
        else:
            # put the function in the global namespace so that profile can find it
            __builtin__.globalProfileFunc = self._func
            __builtin__.globalProfileResult = [None]

            # set up the RAM file
            self._filenames.append(self._getNextFilename())
            filename = self._filenames[-1]
            _installProfileCustomFuncs(filename)

            # do the profiling
            Profile = profile.Profile
            statement = 'globalProfileResult[0]=globalProfileFunc()'
            sort = -1
            retVal = None

            # this is based on profile.run, the code is replicated here to allow us to
            # eliminate a memory leak
            prof = Profile()
            try:
                prof = prof.run(statement)
            except SystemExit:
                pass
            # this has to be run immediately after profiling for the timings to be accurate
            # tell the Profile object to generate output to the RAM file
            prof.dump_stats(filename)

            # eliminate the memory leak
            del prof.dispatcher

            # store the RAM file for later
            profData = _getProfileResultFileInfo(filename)
            self._filename2ramFile[filename] = profData
            # calculate the duration (this is dependent on the internal Python profile data format.
            # see profile.py and pstats.py, this was copied from pstats.Stats.strip_dirs)
            maxTime = 0.
            for cc, nc, tt, ct, callers in profData[1].itervalues():
                if ct > maxTime:
                    maxTime = ct
            self._duration = maxTime
            # clean up the RAM file support
            _removeProfileCustomFuncs(filename)

            # clean up the globals
            result = globalProfileResult[0]
            del __builtin__.__dict__['globalProfileFunc']
            del __builtin__.__dict__['globalProfileResult']

            self._successfulProfiles += 1
            
        self.release()
        return result

    def getDuration(self):
        return self._duration

    def profileSucceeded(self):
        return self._successfulProfiles > 0

    def _restoreRamFile(self, filename):
        # set up the RAM file
        _installProfileCustomFuncs(filename)
        # install the stored RAM file from self.run()
        _setProfileResultsFileInfo(filename, self._filename2ramFile[filename])

    def _discardRamFile(self, filename):
        # take down the RAM file
        _removeProfileCustomFuncs(filename)
        # and discard it
        del self._filename2ramFile[filename]
    
    def getResults(self,
                   lines=80,
                   sorts=['cumulative', 'time', 'calls'],
                   callInfo=False):
        if not self.profileSucceeded():
            output = '%s: profiler already running, could not profile' % self._name
        else:
            # make sure our stats object exists and is up-to-date
            statsChanged = (self._statFileCounter < len(self._filenames))

            if self._stats is None:
                for filename in self._filenames:
                    self._restoreRamFile(filename)
                self._stats = pstats.Stats(*self._filenames)
                self._statFileCounter = len(self._filenames)
                for filename in self._filenames:
                    self._discardRamFile(filename)
            else:
                while self._statFileCounter < len(self._filenames):
                    filename = self._filenames[self._statFileCounter]
                    self._restoreRamFile(filename)
                    self._stats.add(filename)
                    self._discardRamFile(filename)

            if statsChanged:
                self._stats.strip_dirs()
                # throw out any cached result strings
                self._resultCache = {}

            # make sure the arguments will hash efficiently if callers provide different types
            lines = int(lines)
            sorts = list(sorts)
            callInfo = bool(callInfo)
            k = str((lines, sorts, callInfo))
            if k in self._resultCache:
                # we've already created this output string, get it from the cache
                output = self._resultCache[k]
            else:
                # now get human-readable output from the profile stats

                # capture print output to a string
                sc = StdoutCapture()

                # print the info to stdout
                s = self._stats
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

                # cache this result
                self._resultCache[k] = output

        return output
    
