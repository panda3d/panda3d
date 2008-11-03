from pandac.libpandaexpressModules import TrueClock
from direct.directnotify.DirectNotifyGlobal import directNotify
from direct.showbase.PythonUtil import (
    StdoutCapture, _installProfileCustomFuncs,_removeProfileCustomFuncs,
    _profileWithoutGarbageLeak, _getProfileResultFileInfo, _setProfileResultsFileInfo,
    _clearProfileResultFileInfo, )
import __builtin__
import profile
import pstats
from StringIO import StringIO
import marshal

class PercentStats(pstats.Stats):
    # prints more useful output when sampled durations are shorter than a millisecond
    # lots of this is copied from Python's pstats.py
    def setTotalTime(self, tt):
        # use this to set 'total time' to base time percentages on
        # allows profiles to show timing based on percentages of duration of another profile
        self._totalTime = tt

    def add(self, *args, **kArgs):
        pstats.Stats.add(self, *args, **kArgs)
        # DCR -- don't need to record filenames
        self.files = []

    def print_stats(self, *amount):
        for filename in self.files:
            print filename
        if self.files: print
        indent = ' ' * 8
        for func in self.top_level:
            print indent, func_get_function_name(func)

        print indent, self.total_calls, "function calls",
        if self.total_calls != self.prim_calls:
            print "(%d primitive calls)" % self.prim_calls,
        # DCR
        #print "in %.3f CPU seconds" % self.total_tt
        print "in %s CPU milliseconds" % (self.total_tt * 1000.)
        if self._totalTime != self.total_tt:
            print indent, 'percentages are of %s CPU milliseconds' % (self._totalTime * 1000)
        print
        width, list = self.get_print_list(amount)
        if list:
            self.print_title()
            for func in list:
                self.print_line(func)
            print
            # DCR
            #print
        return self

    def f8(self, x):
        if self._totalTime == 0.:
            # profiling was too quick for clock resolution...
            return '    Inf%'
        return "%7.2f%%" % ((x*100.) / self._totalTime)

    @staticmethod
    def func_std_string(func_name): # match what old profile produced
        return "%s:%d(%s)" % func_name

    def print_line(self, func):
        cc, nc, tt, ct, callers = self.stats[func]
        c = str(nc)
        # DCR
        f8 = self.f8
        if nc != cc:
            c = c + '/' + str(cc)
        print c.rjust(9),
        print f8(tt),
        if nc == 0:
            print ' '*8,
        else:
            print f8(tt/nc),
        print f8(ct),
        if cc == 0:
            print ' '*8,
        else:
            print f8(ct/cc),
        # DCR
        #print func_std_string(func)
        print PercentStats.func_std_string(func)

class ProfileSession:
    # class that encapsulates a profile of a single callable using Python's standard
    # 'profile' module
    #
    # defers formatting of profile results until they are requested
    # 
    # implementation sidesteps memory leak in Python profile module,
    # and redirects file output to RAM file for efficiency
    TrueClock = TrueClock.getGlobalPtr()
    
    notify = directNotify.newCategory("ProfileSession")

    def __init__(self, name, func=None, logAfterProfile=False):
        self._func = func
        self._name = name
        self._logAfterProfile = logAfterProfile
        self._filenameBase = 'profileData-%s-%s' % (self._name, id(self))
        self._refCount = 0
        # if true, accumulate profile results every time we run
        # if false, throw out old results every time we run
        self._aggregate = False
        self._lines = 500
        self._sorts = ['cumulative', 'time', 'calls']
        self._callInfo = True
        self._totalTime = None
        self._reset()
        self.acquire()

    def getReference(self):
        # call this when you want to store a new reference to this session that will
        # manage its acquire/release reference count independently of an existing reference
        self.acquire()
        return self

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

    def run(self):
        # make sure this instance doesn't get destroyed inside self._func
        self.acquire()

        if not self._aggregate:
            self._reset()

        # if we're already profiling, just run the func and don't profile
        if 'globalProfileSessionFunc' in __builtin__.__dict__:
            self.notify.warning('could not profile %s' % self._func)
            result = self._func()
            if self._duration is None:
                self._duration = 0.
        else:
            # put the function in the global namespace so that profile can find it
            assert callable(self._func)
            __builtin__.globalProfileSessionFunc = self._func
            __builtin__.globalProfileSessionResult = [None]

            # set up the RAM file
            self._filenames.append(self._getNextFilename())
            filename = self._filenames[-1]
            _installProfileCustomFuncs(filename)

            # do the profiling
            Profile = profile.Profile
            statement = 'globalProfileSessionResult[0]=globalProfileSessionFunc()'
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
            result = globalProfileSessionResult[0]
            del __builtin__.__dict__['globalProfileSessionFunc']
            del __builtin__.__dict__['globalProfileSessionResult']

            self._successfulProfiles += 1
            
            if self._logAfterProfile:
                self.notify.info(self.getResults())

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

    def setName(self, name):
        self._name = name
    def getName(self):
        return self._name

    def setFunc(self, func):
        self._func = func
    def getFunc(self):
        return self._func

    def setAggregate(self, aggregate):
        self._aggregate = aggregate
    def getAggregate(self):
        return self._aggregate

    def setLogAfterProfile(self, logAfterProfile):
        self._logAfterProfile = logAfterProfile
    def getLogAfterProfile(self):
        return self._logAfterProfile
    
    def setLines(self, lines):
        self._lines = lines
    def getLines(self):
        return self._lines

    def setSorts(self, sorts):
        self._sorts = sorts
    def getSorts(self):
        return self._sorts

    def setShowCallInfo(self, showCallInfo):
        self._showCallInfo = showCallInfo
    def getShowCallInfo(self):
        return self._showCallInfo

    def setTotalTime(self, totalTime=None):
        self._totalTime = totalTime
    def resetTotalTime(self):
        self._totalTime = None
    def getTotalTime(self):
        return self._totalTime

    def aggregate(self, other):
        # pull in stats from another ProfileSession
        other._compileStats()
        self._compileStats()
        self._stats.add(other._stats)

    def _compileStats(self):
        # make sure our stats object exists and is up-to-date
        statsChanged = (self._statFileCounter < len(self._filenames))

        if self._stats is None:
            for filename in self._filenames:
                self._restoreRamFile(filename)
            self._stats = PercentStats(*self._filenames)
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

        return statsChanged

    def getResults(self,
                   lines=Default,
                   sorts=Default,
                   callInfo=Default,
                   totalTime=Default):
        if not self.profileSucceeded():
            output = '%s: profiler already running, could not profile' % self._name
        else:
            if lines is Default:
                lines = self._lines
            if sorts is Default:
                sorts = self._sorts
            if callInfo is Default:
                callInfo = self._callInfo
            if totalTime is Default:
                totalTime = self._totalTime
            
            self._compileStats()

            if totalTime is None:
                totalTime = self._stats.total_tt

            # make sure the arguments will hash efficiently if callers provide different types
            lines = int(lines)
            sorts = list(sorts)
            callInfo = bool(callInfo)
            totalTime = float(totalTime)
            k = str((lines, sorts, callInfo, totalTime))
            if k in self._resultCache:
                # we've already created this output string, get it from the cache
                output = self._resultCache[k]
            else:
                # now get human-readable output from the profile stats

                # capture print output to a string
                sc = StdoutCapture()

                # print the info to stdout
                s = self._stats
                # make sure our percentages are relative to the correct total time
                s.setTotalTime(totalTime)
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
    
