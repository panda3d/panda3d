"""Contains miscellaneous utility functions and classes."""

__all__ = [

    'indent', 'doc', 'adjust', 'difference', 'intersection', 'union',
    'sameElements', 'makeList', 'makeTuple', 'list2dict', 'invertDict',
    'invertDictLossless', 'uniqueElements', 'disjoint', 'contains', 'replace',
    'reduceAngle', 'fitSrcAngle2Dest', 'fitDestAngle2Src', 'closestDestAngle2',
    'closestDestAngle', 'getSetterName', 'getSetter', 'Functor', 'Stack',
    'Queue', 'bound', 'clamp', 'lerp', 'average', 'addListsByValue',
    'boolEqual', 'lineupPos', 'formatElapsedSeconds', 'solveQuadratic',
    'findPythonModule', 'mostDerivedLast', 'clampScalar', 'weightedChoice',
    'randFloat', 'normalDistrib', 'weightedRand', 'randUint31', 'randInt32',
    'SerialNumGen', 'serialNum', 'uniqueName', 'Enum', 'Singleton',
    'SingletonError', 'printListEnum', 'safeRepr', 'fastRepr',
    'isDefaultValue', 'ScratchPad', 'Sync', 'itype', 'getNumberedTypedString',
    'getNumberedTypedSortedString', 'printNumberedTyped', 'DelayedCall',
    'DelayedFunctor', 'FrameDelayedCall', 'SubframeCall', 'getBase',
    'GoldenRatio', 'GoldenRectangle', 'rad90', 'rad180', 'rad270', 'rad360',
    'nullGen', 'loopGen', 'makeFlywheelGen', 'flywheel', 'listToIndex2item',
    'listToItem2index', 'formatTimeCompact', 'deeptype', 'StdoutCapture',
    'StdoutPassthrough', 'Averager', 'getRepository', 'formatTimeExact',
    'startSuperLog', 'endSuperLog', 'typeName', 'safeTypeName',
    'histogramDict', 'unescapeHtmlString',
]

if __debug__:
    __all__ += ['StackTrace', 'traceFunctionCall', 'traceParentCall',
                'printThisCall', 'stackEntryInfo', 'lineInfo', 'callerInfo',
                'lineTag', 'profileFunc', 'profiled', 'startProfile',
                'printProfile', 'getProfileResultString', 'printStack',
                'printReverseStack']

import types
import math
import os
import sys
import random
import time

__report_indent = 3

from panda3d.core import ConfigVariableBool

if sys.version_info >= (3, 0):
    import builtins
    from functools import reduce
    xrange = range
else:
    import __builtin__ as builtins


"""
# with one integer positional arg, this uses about 4/5 of the memory of the Functor class below
def Functor(function, *args, **kArgs):
    argsCopy = args[:]
    def functor(*cArgs, **ckArgs):
        kArgs.update(ckArgs)
        return function(*(argsCopy + cArgs), **kArgs)
    return functor
"""

try:
    import importlib
except ImportError:
    # Backward compatibility for Python 2.6.
    def _resolve_name(name, package, level):
        if not hasattr(package, 'rindex'):
            raise ValueError("'package' not set to a string")
        dot = len(package)
        for x in xrange(level, 1, -1):
            try:
                dot = package.rindex('.', 0, dot)
            except ValueError:
                raise ValueError("attempted relative import beyond top-level "
                                  "package")
        return "%s.%s" % (package[:dot], name)

    def import_module(name, package=None):
        if name.startswith('.'):
            if not package:
                raise TypeError("relative imports require the 'package' argument")
            level = 0
            for character in name:
                if character != '.':
                    break
                level += 1
            name = _resolve_name(name[level:], package, level)
        __import__(name)
        return sys.modules[name]

    imp = import_module('imp')
    importlib = imp.new_module("importlib")
    importlib._resolve_name = _resolve_name
    importlib.import_module = import_module
    sys.modules['importlib'] = importlib


class Functor:
    def __init__(self, function, *args, **kargs):
        assert callable(function), "function should be a callable obj"
        self._function = function
        self._args = args
        self._kargs = kargs
        if hasattr(self._function, '__name__'):
            self.__name__ = self._function.__name__
        else:
            self.__name__ = str(itype(self._function))
        if hasattr(self._function, '__doc__'):
            self.__doc__ = self._function.__doc__
        else:
            self.__doc__ = self.__name__

    def destroy(self):
        del self._function
        del self._args
        del self._kargs
        del self.__name__
        del self.__doc__

    def _do__call__(self, *args, **kargs):
        _kargs = self._kargs.copy()
        _kargs.update(kargs)
        return self._function(*(self._args + args), **_kargs)

    __call__ = _do__call__

    def __repr__(self):
        s = 'Functor(%s' % self._function.__name__
        for arg in self._args:
            try:
                argStr = repr(arg)
            except:
                argStr = 'bad repr: %s' % arg.__class__
            s += ', %s' % argStr
        for karg, value in list(self._kargs.items()):
            s += ', %s=%s' % (karg, repr(value))
        s += ')'
        return s

class Stack:
    def __init__(self):
        self.__list = []
    def push(self, item):
        self.__list.append(item)
    def top(self):
        # return the item on the top of the stack without popping it off
        return self.__list[-1]
    def pop(self):
        return self.__list.pop()
    def clear(self):
        self.__list = []
    def isEmpty(self):
        return len(self.__list) == 0
    def __len__(self):
        return len(self.__list)

class Queue:
    # FIFO queue
    # interface is intentionally identical to Stack (LIFO)
    def __init__(self):
        self.__list = []
    def push(self, item):
        self.__list.append(item)
    def top(self):
        # return the next item at the front of the queue without popping it off
        return self.__list[0]
    def front(self):
        return self.__list[0]
    def back(self):
        return self.__list[-1]
    def pop(self):
        return self.__list.pop(0)
    def clear(self):
        self.__list = []
    def isEmpty(self):
        return len(self.__list) == 0
    def __len__(self):
        return len(self.__list)


def indent(stream, numIndents, str):
    """
    Write str to stream with numIndents in front of it
    """
    # To match emacs, instead of a tab character we will use 4 spaces
    stream.write('    ' * numIndents + str)


if __debug__:
    import traceback
    import marshal

    class StackTrace:
        def __init__(self, label="", start=0, limit=None):
            """
            label is a string (or anything that be be a string)
            that is printed as part of the trace back.
            This is just to make it easier to tell what the
            stack trace is referring to.
            start is an integer number of stack frames back
            from the most recent.  (This is automatically
            bumped up by one to skip the __init__ call
            to the StackTrace).
            limit is an integer number of stack frames
            to record (or None for unlimited).
            """
            self.label = label
            if limit is not None:
                self.trace = traceback.extract_stack(sys._getframe(1+start),
                                                     limit=limit)
            else:
                self.trace = traceback.extract_stack(sys._getframe(1+start))

        def compact(self):
            r = ''
            comma = ','
            for filename, lineNum, funcName, text in self.trace:
                r += '%s.%s:%s%s' % (filename[:filename.rfind('.py')][filename.rfind('\\')+1:], funcName, lineNum, comma)
            if len(r):
                r = r[:-len(comma)]
            return r

        def reverseCompact(self):
            r = ''
            comma = ','
            for filename, lineNum, funcName, text in self.trace:
                r = '%s.%s:%s%s%s' % (filename[:filename.rfind('.py')][filename.rfind('\\')+1:], funcName, lineNum, comma, r)
            if len(r):
                r = r[:-len(comma)]
            return r

        def __str__(self):
            r = "Debug stack trace of %s (back %s frames):\n"%(
                self.label, len(self.trace),)
            for i in traceback.format_list(self.trace):
                r+=i
            r+="***** NOTE: This is not a crash. This is a debug stack trace. *****"
            return r

    def printStack():
        print(StackTrace(start=1).compact())
        return True
    def printReverseStack():
        print(StackTrace(start=1).reverseCompact())
        return True
    def printVerboseStack():
        print(StackTrace(start=1))
        return True

    #-----------------------------------------------------------------------------

    def traceFunctionCall(frame):
        """
        return a string that shows the call frame with calling arguments.
        e.g.
        foo(x=234, y=135)
        """
        f = frame
        co = f.f_code
        dict = f.f_locals
        n = co.co_argcount
        if co.co_flags & 4: n = n+1
        if co.co_flags & 8: n = n+1
        r=''
        if 'self' in dict:
            r = '%s.'%(dict['self'].__class__.__name__,)
        r+="%s("%(f.f_code.co_name,)
        comma=0 # formatting, whether we should type a comma.
        for i in range(n):
            name = co.co_varnames[i]
            if name=='self':
                continue
            if comma:
                r+=', '
            else:
                # ok, we skipped the first one, the rest get commas:
                comma=1
            r+=name
            r+='='
            if name in dict:
                v=safeRepr(dict[name])
                if len(v)>2000:
                    # r+="<too big for debug>"
                    r += (v[:2000] + "...")
                else:
                    r+=v
            else: r+="*** undefined ***"
        return r+')'

    def traceParentCall():
        return traceFunctionCall(sys._getframe(2))

    def printThisCall():
        print(traceFunctionCall(sys._getframe(1)))
        return 1 # to allow "assert printThisCall()"

# Magic numbers: These are the bit masks in func_code.co_flags that
# reveal whether or not the function has a *arg or **kw argument.
_POS_LIST = 4
_KEY_DICT = 8

def doc(obj):
    if (isinstance(obj, types.MethodType)) or \
       (isinstance(obj, types.FunctionType)):
        print(obj.__doc__)

def adjust(command = None, dim = 1, parent = None, **kw):
    """
    adjust(command = None, parent = None, **kw)
    Popup and entry scale to adjust a parameter

    Accepts any Slider keyword argument.  Typical arguments include:
    command: The one argument command to execute
    min: The min value of the slider
    max: The max value of the slider
    resolution: The resolution of the slider
    text: The label on the slider

    These values can be accessed and/or changed after the fact
    >>> vg = adjust()
    >>> vg['min']
    0.0
    >>> vg['min'] = 10.0
    >>> vg['min']
    10.0
    """
    # Make sure we enable Tk
    # Don't use a regular import, to prevent ModuleFinder from picking
    # it up as a dependency when building a .p3d package.
    Valuator = importlib.import_module('direct.tkwidgets.Valuator')
    # Set command if specified
    if command:
        kw['command'] = lambda x: command(*x)
        if parent is None:
            kw['title'] = command.__name__
    kw['dim'] = dim
    # Create toplevel if needed
    if not parent:
        vg = Valuator.ValuatorGroupPanel(parent, **kw)
    else:
        vg = Valuator.ValuatorGroup(parent, **kw)
        vg.pack(expand = 1, fill = 'x')
    return vg

def difference(a, b):
    """
    difference(list, list):
    """
    if not a: return b
    if not b: return a
    d = []
    for i in a:
        if (i not in b) and (i not in d):
            d.append(i)
    for i in b:
        if (i not in a) and (i not in d):
            d.append(i)
    return d

def intersection(a, b):
    """
    intersection(list, list):
    """
    if not a: return []
    if not b: return []
    d = []
    for i in a:
        if (i in b) and (i not in d):
            d.append(i)
    for i in b:
        if (i in a) and (i not in d):
            d.append(i)
    return d

def union(a, b):
    """
    union(list, list):
    """
    # Copy a
    c = a[:]
    for i in b:
        if (i not in c):
            c.append(i)
    return c

def sameElements(a, b):
    if len(a) != len(b):
        return 0
    for elem in a:
        if elem not in b:
            return 0
    for elem in b:
        if elem not in a:
            return 0
    return 1

def makeList(x):
    """returns x, converted to a list"""
    if type(x) is list:
        return x
    elif type(x) is tuple:
        return list(x)
    else:
        return [x,]

def makeTuple(x):
    """returns x, converted to a tuple"""
    if type(x) is list:
        return tuple(x)
    elif type(x) is tuple:
        return x
    else:
        return (x,)

def list2dict(L, value=None):
    """creates dict using elements of list, all assigned to same value"""
    return dict([(k, value) for k in L])

def listToIndex2item(L):
    """converts list to dict of list index->list item"""
    d = {}
    for i, item in enumerate(L):
        d[i] = item
    return d

assert listToIndex2item(['a','b']) == {0: 'a', 1: 'b',}

def listToItem2index(L):
    """converts list to dict of list item->list index
    This is lossy if there are duplicate list items"""
    d = {}
    for i, item in enumerate(L):
        d[item] = i
    return d

assert listToItem2index(['a','b']) == {'a': 0, 'b': 1,}

def invertDict(D, lossy=False):
    """creates a dictionary by 'inverting' D; keys are placed in the new
    dictionary under their corresponding value in the old dictionary.
    It is an error if D contains any duplicate values.

    >>> old = {'key1':1, 'key2':2}
    >>> invertDict(old)
    {1: 'key1', 2: 'key2'}
    """
    n = {}
    for key, value in D.items():
        if not lossy and value in n:
            raise Exception('duplicate key in invertDict: %s' % value)
        n[value] = key
    return n

def invertDictLossless(D):
    """similar to invertDict, but values of new dict are lists of keys from
    old dict. No information is lost.

    >>> old = {'key1':1, 'key2':2, 'keyA':2}
    >>> invertDictLossless(old)
    {1: ['key1'], 2: ['key2', 'keyA']}
    """
    n = {}
    for key, value in D.items():
        n.setdefault(value, [])
        n[value].append(key)
    return n

def uniqueElements(L):
    """are all elements of list unique?"""
    return len(L) == len(list2dict(L))

def disjoint(L1, L2):
    """returns non-zero if L1 and L2 have no common elements"""
    used = dict([(k, None) for k in L1])
    for k in L2:
        if k in used:
            return 0
    return 1

def contains(whole, sub):
    """
    Return 1 if whole contains sub, 0 otherwise
    """
    if (whole == sub):
        return 1
    for elem in sub:
        # The first item you find not in whole, return 0
        if elem not in whole:
            return 0
    # If you got here, whole must contain sub
    return 1

def replace(list, old, new, all=0):
    """
    replace 'old' with 'new' in 'list'
    if all == 0, replace first occurrence
    otherwise replace all occurrences
    returns the number of items replaced
    """
    if old not in list:
        return 0

    if not all:
        i = list.index(old)
        list[i] = new
        return 1
    else:
        numReplaced = 0
        for i in xrange(len(list)):
            if list[i] == old:
                numReplaced += 1
                list[i] = new
        return numReplaced

rad90 = math.pi / 2.
rad180 = math.pi
rad270 = 1.5 * math.pi
rad360 = 2. * math.pi

def reduceAngle(deg):
    """
    Reduces an angle (in degrees) to a value in [-180..180)
    """
    return (((deg + 180.) % 360.) - 180.)

def fitSrcAngle2Dest(src, dest):
    """
    given a src and destination angle, returns an equivalent src angle
    that is within [-180..180) of dest
    examples:
    fitSrcAngle2Dest(30, 60) == 30
    fitSrcAngle2Dest(60, 30) == 60
    fitSrcAngle2Dest(0, 180) == 0
    fitSrcAngle2Dest(-1, 180) == 359
    fitSrcAngle2Dest(-180, 180) == 180
    """
    return dest + reduceAngle(src - dest)

def fitDestAngle2Src(src, dest):
    """
    given a src and destination angle, returns an equivalent dest angle
    that is within [-180..180) of src
    examples:
    fitDestAngle2Src(30, 60) == 60
    fitDestAngle2Src(60, 30) == 30
    fitDestAngle2Src(0, 180) == -180
    fitDestAngle2Src(1, 180) == 180
    """
    return src + (reduceAngle(dest - src))

def closestDestAngle2(src, dest):
    # The function above didn't seem to do what I wanted. So I hacked
    # this one together. I can't really say I understand it. It's more
    # from impirical observation... GRW
    diff = src - dest
    if diff > 180:
        # if the difference is greater that 180 it's shorter to go the other way
        return dest - 360
    elif diff < -180:
        # or perhaps the OTHER other way...
        return dest + 360
    else:
        # otherwise just go to the original destination
        return dest

def closestDestAngle(src, dest):
    # The function above didn't seem to do what I wanted. So I hacked
    # this one together. I can't really say I understand it. It's more
    # from impirical observation... GRW
    diff = src - dest
    if diff > 180:
        # if the difference is greater that 180 it's shorter to go the other way
        return src - (diff - 360)
    elif diff < -180:
        # or perhaps the OTHER other way...
        return src - (360 + diff)
    else:
        # otherwise just go to the original destination
        return dest

class StdoutCapture:
    # redirects stdout to a string
    def __init__(self):
        self._oldStdout = sys.stdout
        sys.stdout = self
        self._string = ''
    def destroy(self):
        sys.stdout = self._oldStdout
        del self._oldStdout

    def getString(self):
        return self._string

    # internal
    def write(self, string):
        self._string = ''.join([self._string, string])

class StdoutPassthrough(StdoutCapture):
    # like StdoutCapture but also allows output to go through to the OS as normal

    # internal
    def write(self, string):
        self._string = ''.join([self._string, string])
        self._oldStdout.write(string)

# constant profile defaults
if __debug__:
    from io import StringIO

    PyUtilProfileDefaultFilename = 'profiledata'
    PyUtilProfileDefaultLines = 80
    PyUtilProfileDefaultSorts = ['cumulative', 'time', 'calls']

    _ProfileResultStr = ''

    def getProfileResultString():
        # if you called profile with 'log' not set to True,
        # you can call this function to get the results as
        # a string
        global _ProfileResultStr
        return _ProfileResultStr

    def profileFunc(callback, name, terse, log=True):
        global _ProfileResultStr
        if 'globalProfileFunc' in builtins.__dict__:
            # rats. Python profiler is not re-entrant...
            base.notify.warning(
                'PythonUtil.profileStart(%s): aborted, already profiling %s'
                #'\nStack Trace:\n%s'
                % (name, builtins.globalProfileFunc,
                #StackTrace()
                ))
            return
        builtins.globalProfileFunc = callback
        builtins.globalProfileResult = [None]
        prefix = '***** START PROFILE: %s *****' % name
        if log:
            print(prefix)
        startProfile(cmd='globalProfileResult[0]=globalProfileFunc()', callInfo=(not terse), silent=not log)
        suffix = '***** END PROFILE: %s *****' % name
        if log:
            print(suffix)
        else:
            _ProfileResultStr = '%s\n%s\n%s' % (prefix, _ProfileResultStr, suffix)
        result = globalProfileResult[0]
        del builtins.__dict__['globalProfileFunc']
        del builtins.__dict__['globalProfileResult']
        return result

    def profiled(category=None, terse=False):
        """ decorator for profiling functions
        turn categories on and off via "want-profile-categoryName 1"

        e.g.::

            @profiled('particles')
            def loadParticles():
                ...

        ::

            want-profile-particles 1
        """
        assert type(category) in (str, type(None)), "must provide a category name for @profiled"

        # allow profiling in published versions
        """
        try:
            null = not __dev__
        except:
            null = not __debug__
        if null:
            # if we're not in __dev__, just return the function itself. This
            # results in zero runtime overhead, since decorators are evaluated
            # at module-load.
            def nullDecorator(f):
                return f
            return nullDecorator
        """

        def profileDecorator(f):
            def _profiled(*args, **kArgs):
                name = '(%s) %s from %s' % (category, f.__name__, f.__module__)

                # showbase might not be loaded yet, so don't use
                # base.config.  Instead, query the ConfigVariableBool.
                if (category is None) or ConfigVariableBool('want-profile-%s' % category, 0).getValue():
                    return profileFunc(Functor(f, *args, **kArgs), name, terse)
                else:
                    return f(*args, **kArgs)
            _profiled.__doc__ = f.__doc__
            return _profiled
        return profileDecorator

    # intercept profile-related file operations to avoid disk access
    movedOpenFuncs = []
    movedDumpFuncs = []
    movedLoadFuncs = []
    profileFilenames = set()
    profileFilenameList = Stack()
    profileFilename2file = {}
    profileFilename2marshalData = {}

    def _profileOpen(filename, *args, **kArgs):
        # this is a replacement for the file open() builtin function
        # for use during profiling, to intercept the file open
        # operation used by the Python profiler and profile stats
        # systems
        if filename in profileFilenames:
            # if this is a file related to profiling, create an
            # in-RAM file object
            if filename not in profileFilename2file:
                file = StringIO()
                file._profFilename = filename
                profileFilename2file[filename] = file
            else:
                file = profileFilename2file[filename]
        else:
            file = movedOpenFuncs[-1](filename, *args, **kArgs)
        return file

    def _profileMarshalDump(data, file):
        # marshal.dump doesn't work with StringIO objects
        # simulate it
        if isinstance(file, StringIO) and hasattr(file, '_profFilename'):
            if file._profFilename in profileFilenames:
                profileFilename2marshalData[file._profFilename] = data
                return None
        return movedDumpFuncs[-1](data, file)

    def _profileMarshalLoad(file):
        # marshal.load doesn't work with StringIO objects
        # simulate it
        if isinstance(file, StringIO) and hasattr(file, '_profFilename'):
            if file._profFilename in profileFilenames:
                return profileFilename2marshalData[file._profFilename]
        return movedLoadFuncs[-1](file)

    def _installProfileCustomFuncs(filename):
        assert filename not in profileFilenames
        profileFilenames.add(filename)
        profileFilenameList.push(filename)
        movedOpenFuncs.append(builtins.open)
        builtins.open = _profileOpen
        movedDumpFuncs.append(marshal.dump)
        marshal.dump = _profileMarshalDump
        movedLoadFuncs.append(marshal.load)
        marshal.load = _profileMarshalLoad

    def _getProfileResultFileInfo(filename):
        return (profileFilename2file.get(filename, None),
                profileFilename2marshalData.get(filename, None))

    def _setProfileResultsFileInfo(filename, info):
        f, m = info
        if f:
            profileFilename2file[filename] = f
        if m:
            profileFilename2marshalData[filename] = m

    def _clearProfileResultFileInfo(filename):
        profileFilename2file.pop(filename, None)
        profileFilename2marshalData.pop(filename, None)

    def _removeProfileCustomFuncs(filename):
        assert profileFilenameList.top() == filename
        marshal.load = movedLoadFuncs.pop()
        marshal.dump = movedDumpFuncs.pop()
        builtins.open = movedOpenFuncs.pop()
        profileFilenames.remove(filename)
        profileFilenameList.pop()
        profileFilename2file.pop(filename, None)
        # don't let marshalled data pile up
        profileFilename2marshalData.pop(filename, None)


    # call this from the prompt, and break back out to the prompt
    # to stop profiling
    #
    # OR to do inline profiling, you must make a globally-visible
    # function to be profiled, i.e. to profile 'self.load()', do
    # something like this:
    #
    #        def func(self=self):
    #            self.load()
    #        import builtins
    #        builtins.func = func
    #        PythonUtil.startProfile(cmd='func()', filename='profileData')
    #        del builtins.func
    #
    def _profileWithoutGarbageLeak(cmd, filename):
        # The profile module isn't necessarily installed on every Python
        # installation, so we import it here, instead of in the module
        # scope.
        import profile
        # this is necessary because the profile module creates a memory leak
        Profile = profile.Profile
        statement = cmd
        sort = -1
        retVal = None
        #### COPIED FROM profile.run ####
        prof = Profile()
        try:
            prof = prof.run(statement)
        except SystemExit:
            pass
        if filename is not None:
            prof.dump_stats(filename)
        else:
            #return prof.print_stats(sort)  #DCR
            retVal = prof.print_stats(sort) #DCR
        #################################
        # eliminate the garbage leak
        del prof.dispatcher
        return retVal

    def startProfile(filename=PyUtilProfileDefaultFilename,
                     lines=PyUtilProfileDefaultLines,
                     sorts=PyUtilProfileDefaultSorts,
                     silent=0,
                     callInfo=1,
                     useDisk=False,
                     cmd='run()'):
        # uniquify the filename to allow multiple processes to profile simultaneously
        filename = '%s.%s%s' % (filename, randUint31(), randUint31())
        if not useDisk:
            # use a RAM file
            _installProfileCustomFuncs(filename)
        _profileWithoutGarbageLeak(cmd, filename)
        if silent:
            extractProfile(filename, lines, sorts, callInfo)
        else:
            printProfile(filename, lines, sorts, callInfo)
        if not useDisk:
            # discard the RAM file
            _removeProfileCustomFuncs(filename)
        else:
            os.remove(filename)

    # call these to see the results again, as a string or in the log
    def printProfile(filename=PyUtilProfileDefaultFilename,
                     lines=PyUtilProfileDefaultLines,
                     sorts=PyUtilProfileDefaultSorts,
                     callInfo=1):
        import pstats
        s = pstats.Stats(filename)
        s.strip_dirs()
        for sort in sorts:
            s.sort_stats(sort)
            s.print_stats(lines)
            if callInfo:
                s.print_callees(lines)
                s.print_callers(lines)

    # same args as printProfile
    def extractProfile(*args, **kArgs):
        global _ProfileResultStr
        # capture print output
        sc = StdoutCapture()
        # print the profile output, redirected to the result string
        printProfile(*args, **kArgs)
        # make a copy of the print output
        _ProfileResultStr = sc.getString()
        # restore stdout to what it was before
        sc.destroy()

def getSetterName(valueName, prefix='set'):
    # getSetterName('color') -> 'setColor'
    # getSetterName('color', 'get') -> 'getColor'
    return '%s%s%s' % (prefix, valueName[0].upper(), valueName[1:])
def getSetter(targetObj, valueName, prefix='set'):
    # getSetter(smiley, 'pos') -> smiley.setPos
    return getattr(targetObj, getSetterName(valueName, prefix))

def mostDerivedLast(classList):
    """pass in list of classes. sorts list in-place, with derived classes
    appearing after their bases"""

    class ClassSortKey(object):
        __slots__ = 'classobj',
        def __init__(self, classobj):
            self.classobj = classobj
        def __lt__(self, other):
            return issubclass(other.classobj, self.classobj)

    classList.sort(key=ClassSortKey)

def bound(value, bound1, bound2):
    """
    returns value if value is between bound1 and bound2
    otherwise returns bound that is closer to value
    """
    if bound1 > bound2:
        return min(max(value, bound2), bound1)
    else:
        return min(max(value, bound1), bound2)
clamp = bound

def lerp(v0, v1, t):
    """
    returns a value lerped between v0 and v1, according to t
    t == 0 maps to v0, t == 1 maps to v1
    """
    return v0 + ((v1 - v0) * t)

def getShortestRotation(start, end):
    """
    Given two heading values, return a tuple describing
    the shortest interval from 'start' to 'end'.  This tuple
    can be used to lerp a camera between two rotations
    while avoiding the 'spin' problem.
    """
    start, end = start % 360, end % 360
    if abs(end - start) > 180:
        if end < start:
            end += 360
        else:
            start += 360
    return (start, end)

def average(*args):
    """ returns simple average of list of values """
    val = 0.
    for arg in args:
        val += arg
    return val / len(args)

class Averager:
    def __init__(self, name):
        self._name = name
        self.reset()
    def reset(self):
        self._total = 0.
        self._count = 0
    def addValue(self, value):
        self._total += value
        self._count += 1
    def getAverage(self):
        return self._total / self._count
    def getCount(self):
        return self._count

def addListsByValue(a, b):
    """
    returns a new array containing the sums of the two array arguments
    (c[0] = a[0 + b[0], etc.)
    """
    c = []
    for x, y in zip(a, b):
        c.append(x + y)
    return c

def boolEqual(a, b):
    """
    returns true if a and b are both true or both false.
    returns false otherwise
    (a.k.a. xnor -- eXclusive Not OR).
    """
    return (a and b) or not (a or b)

def lineupPos(i, num, spacing):
    """
    use to line up a series of 'num' objects, in one dimension,
    centered around zero
    'i' is the index of the object in the lineup
    'spacing' is the amount of space between objects in the lineup
    """
    assert num >= 1
    assert i >= 0 and i < num
    pos = float(i) * spacing
    return pos - ((float(spacing) * (num-1))/2.)

def formatElapsedSeconds(seconds):
    """
    Returns a string of the form "mm:ss" or "hh:mm:ss" or "n days",
    representing the indicated elapsed time in seconds.
    """
    sign = ''
    if seconds < 0:
        seconds = -seconds
        sign = '-'

    # We use math.floor() instead of casting to an int, so we avoid
    # problems with numbers that are too large to represent as
    # type int.
    seconds = math.floor(seconds)
    hours = math.floor(seconds / (60 * 60))
    if hours > 36:
        days = math.floor((hours + 12) / 24)
        return "%s%d days" % (sign, days)

    seconds -= hours * (60 * 60)
    minutes = (int)(seconds / 60)
    seconds -= minutes * 60
    if hours != 0:
        return "%s%d:%02d:%02d" % (sign, hours, minutes, seconds)
    else:
        return "%s%d:%02d" % (sign, minutes, seconds)

def solveQuadratic(a, b, c):
    # quadratic equation: ax^2 + bx + c = 0
    # quadratic formula:  x = [-b +/- sqrt(b^2 - 4ac)] / 2a
    # returns None, root, or [root1, root2]

    # a cannot be zero.
    if a == 0.:
        return None

    # calculate the determinant (b^2 - 4ac)
    D = (b * b) - (4. * a * c)

    if D < 0:
        # there are no solutions (sqrt(negative number) is undefined)
        return None
    elif D == 0:
        # only one root
        return (-b) / (2. * a)
    else:
        # OK, there are two roots
        sqrtD = math.sqrt(D)
        twoA = 2. * a
        root1 = ((-b) - sqrtD) / twoA
        root2 = ((-b) + sqrtD) / twoA
        return [root1, root2]

if __debug__:
    def stackEntryInfo(depth=0, baseFileName=1):
        """
        returns the sourcefilename, line number, and function name of
        an entry in the stack.
        'depth' is how far back to go in the stack; 0 is the caller of this
        function, 1 is the function that called the caller of this function, etc.
        by default, strips off the path of the filename; override with baseFileName
        returns (fileName, lineNum, funcName) --> (string, int, string)
        returns (None, None, None) on error
        """
        import inspect

        try:
            stack = None
            frame = None
            try:
                stack = inspect.stack()
                # add one to skip the frame associated with this function
                frame = stack[depth+1]
                filename = frame[1]
                if baseFileName:
                    filename = os.path.basename(filename)
                lineNum = frame[2]
                funcName = frame[3]
                result = (filename, lineNum, funcName)
            finally:
                del stack
                del frame
        except:
            result = (None, None, None)

        return result

    def lineInfo(baseFileName=1):
        """
        returns the sourcefilename, line number, and function name of the
        code that called this function
        (answers the question: 'hey lineInfo, where am I in the codebase?')
        see stackEntryInfo, above, for info on 'baseFileName' and return types
        """
        return stackEntryInfo(1, baseFileName)

    def callerInfo(baseFileName=1, howFarBack=0):
        """
        returns the sourcefilename, line number, and function name of the
        caller of the function that called this function
        (answers the question: 'hey callerInfo, who called me?')
        see stackEntryInfo, above, for info on 'baseFileName' and return types
        """
        return stackEntryInfo(2+howFarBack, baseFileName)

    def lineTag(baseFileName=1, verbose=0, separator=':'):
        """
        returns a string containing the sourcefilename and line number
        of the code that called this function
        (equivalent to lineInfo, above, with different return type)
        see stackEntryInfo, above, for info on 'baseFileName'

        if 'verbose' is false, returns a compact string of the form
        'fileName:lineNum:funcName'
        if 'verbose' is true, returns a longer string that matches the
        format of Python stack trace dumps

        returns empty string on error
        """
        fileName, lineNum, funcName = callerInfo(baseFileName)
        if fileName is None:
            return ''
        if verbose:
            return 'File "%s", line %s, in %s' % (fileName, lineNum, funcName)
        else:
            return '%s%s%s%s%s' % (fileName, separator, lineNum, separator,
                                   funcName)

def findPythonModule(module):
    # Look along the python load path for the indicated filename.
    # Returns the located pathname, or None if the filename is not
    # found.
    filename = module + '.py'
    for dir in sys.path:
        pathname = os.path.join(dir, filename)
        if os.path.exists(pathname):
            return pathname

    return None

def clampScalar(value, a, b):
    # calling this ought to be faster than calling both min and max
    if a < b:
        if value < a:
            return a
        elif value > b:
            return b
        else:
            return value
    else:
        if value < b:
            return b
        elif value > a:
            return a
        else:
            return value

def weightedChoice(choiceList, rng=random.random, sum=None):
    """given a list of (weight, item) pairs, chooses an item based on the
    weights. rng must return 0..1. if you happen to have the sum of the
    weights, pass it in 'sum'."""
    # Throw an IndexError if we got an empty list.
    if not choiceList:
        raise IndexError('Cannot choose from an empty sequence')

    # TODO: add support for dicts
    if sum is None:
        sum = 0.
        for weight, item in choiceList:
            sum += weight

    rand = rng()
    accum = rand * sum
    item = None
    for weight, item in choiceList:
        accum -= weight
        if accum <= 0.:
            return item
    # rand is ~1., and floating-point error prevented accum from hitting 0.
    # Or you passed in a 'sum' that was was too large.
    # Return the last item.
    return item

def randFloat(a, b=0., rng=random.random):
    """returns a random float in [a, b]
    call with single argument to generate random float between arg and zero
    """
    return lerp(a, b, rng())

def normalDistrib(a, b, gauss=random.gauss):
    """
    NOTE: assumes a < b

    Returns random number between a and b, using gaussian distribution, with
    mean=avg(a, b), and a standard deviation that fits ~99.7% of the curve
    between a and b.

    For ease of use, outlying results are re-computed until result is in [a, b]
    This should fit the remaining .3% of the curve that lies outside [a, b]
    uniformly onto the curve inside [a, b]

    ------------------------------------------------------------------------
    The 68-95-99.7% Rule
    ====================
    All normal density curves satisfy the following property which is often
      referred to as the Empirical Rule:
    68% of the observations fall within 1 standard deviation of the mean.
    95% of the observations fall within 2 standard deviations of the mean.
    99.7% of the observations fall within 3 standard deviations of the mean.

    Thus, for a normal distribution, almost all values lie within 3 standard
      deviations of the mean.
    ------------------------------------------------------------------------

    In calculating our standard deviation, we divide (b-a) by 6, since the
    99.7% figure includes 3 standard deviations _on_either_side_ of the mean.
    """
    while True:
        r = gauss((a+b)*.5, (b-a)/6.)
        if (r >= a) and (r <= b):
            return r

def weightedRand(valDict, rng=random.random):
    """
    pass in a dictionary with a selection -> weight mapping.  E.g.::

        {"Choice 1": 10,
         "Choice 2": 30,
         "bear":     100}

    - Weights need not add up to any particular value.
    - The actual selection will be returned.
    """
    selections = list(valDict.keys())
    weights = list(valDict.values())

    totalWeight = 0
    for weight in weights:
        totalWeight += weight

    # get a random value between 0 and the total of the weights
    randomWeight = rng() * totalWeight

    # find the index that corresponds with this weight
    for i in range(len(weights)):
        totalWeight -= weights[i]
        if totalWeight <= randomWeight:
            return selections[i]

    assert True, "Should never get here"
    return selections[-1]

def randUint31(rng=random.random):
    """returns a random integer in [0..2^31).
    rng must return float in [0..1]"""
    return int(rng() * 0x7FFFFFFF)

def randInt32(rng=random.random):
    """returns a random integer in [-2147483648..2147483647].
    rng must return float in [0..1]
    """
    i = int(rng() * 0x7FFFFFFF)
    if rng() < .5:
        i *= -1
    return i

class SerialNumGen:
    """generates serial numbers"""
    def __init__(self, start=None):
        if start is None:
            start = 0
        self.__counter = start-1

    def next(self):
        self.__counter += 1
        return self.__counter

    __next__ = next

class SerialMaskedGen(SerialNumGen):
    def __init__(self, mask, start=None):
        self._mask = mask
        SerialNumGen.__init__(self, start)

    def next(self):
        v = SerialNumGen.next(self)
        return v & self._mask

    __next__ = next

_serialGen = SerialNumGen()
def serialNum():
    global _serialGen
    return _serialGen.next()
def uniqueName(name):
    global _serialGen
    return '%s-%s' % (name, _serialGen.next())

class EnumIter:
    def __init__(self, enum):
        self._values = list(enum._stringTable.keys())
        self._index = 0
    def __iter__(self):
        return self
    def __next__(self):
        if self._index >= len(self._values):
            raise StopIteration
        self._index += 1
        return self._values[self._index-1]
    next = __next__

class Enum:
    """Pass in list of strings or string of comma-separated strings.
    Items are accessible as instance.item, and are assigned unique,
    increasing integer values. Pass in integer for 'start' to override
    starting value.

    Example:

    >>> colors = Enum('red, green, blue')
    >>> colors.red
    0
    >>> colors.green
    1
    >>> colors.blue
    2
    >>> colors.getString(colors.red)
    'red'
    """

    if __debug__:
        # chars that cannot appear within an item string.
        def _checkValidIdentifier(item):
            import string
            invalidChars = string.whitespace + string.punctuation
            invalidChars = invalidChars.replace('_', '')
            invalidFirstChars = invalidChars+string.digits
            if item[0] in invalidFirstChars:
                raise SyntaxError("Enum '%s' contains invalid first char" %
                                    item)
            if not disjoint(item, invalidChars):
                for char in item:
                    if char in invalidChars:
                        raise SyntaxError(
                            "Enum\n'%s'\ncontains illegal char '%s'" %
                            (item, char))
            return 1
        _checkValidIdentifier = staticmethod(_checkValidIdentifier)

    def __init__(self, items, start=0):
        if isinstance(items, str):
            items = items.split(',')

        self._stringTable = {}

        # make sure we don't overwrite an existing element of the class
        assert self._checkExistingMembers(items)
        assert uniqueElements(items)

        i = start
        for item in items:
            # remove leading/trailing whitespace
            item = item.strip()
            # is there anything left?
            if len(item) == 0:
                continue
            # make sure there are no invalid characters
            assert Enum._checkValidIdentifier(item)
            self.__dict__[item] = i
            self._stringTable[i] = item
            i += 1

    def __iter__(self):
        return EnumIter(self)

    def hasString(self, string):
        return string in set(self._stringTable.values())

    def fromString(self, string):
        if self.hasString(string):
            return self.__dict__[string]
        # throw an error
        {}[string]

    def getString(self, value):
        return self._stringTable[value]

    def __contains__(self, value):
        return value in self._stringTable

    def __len__(self):
        return len(self._stringTable)

    def copyTo(self, obj):
        # copies all members onto obj
        for name, value in self._stringTable:
            setattr(obj, name, value)

    if __debug__:
        def _checkExistingMembers(self, items):
            for item in items:
                if hasattr(self, item):
                    return 0
            return 1

############################################################
# class: Singleton
# Purpose: This provides a base metaclass for all classes
#          that require one and only one instance.
#
# Example: class mySingleton:
#              __metaclass__ = PythonUtil.Singleton
#              def __init__(self, ...):
#                  ...
#
# Note: This class is based on Python's New-Style Class
#       design. An error will occur if a defined class
#       attemps to inherit from a Classic-Style Class only,
#       ie: class myClassX:
#               def __init__(self, ...):
#                   ...
#
#           class myNewClassX(myClassX):
#               __metaclass__ = PythonUtil.Singleton
#               def __init__(self, ...):
#                   myClassX.__init__(self, ...)
#                   ...
#
#           This causes problems because myNewClassX is a
#           New-Style class that inherits from only a
#           Classic-Style base class. There are two ways
#           simple ways to resolve this issue.
#
#           First, if possible, make myClassX a
#           New-Style class by inheriting from object
#           object. IE:  class myClassX(object):
#
#           If for some reason that is not an option, make
#           myNewClassX inherit from object and myClassX.
#           IE: class myNewClassX(object, myClassX):
############################################################
class Singleton(type):
    def __init__(cls, name, bases, dic):
        super(Singleton, cls).__init__(name, bases, dic)
        cls.instance=None
    def __call__(cls, *args, **kw):
        if cls.instance is None:
            cls.instance=super(Singleton, cls).__call__(*args, **kw)
        return cls.instance

class SingletonError(ValueError):
    """ Used to indicate an inappropriate value for a Singleton."""

def printListEnumGen(l):
    # log each individual item with a number in front of it
    digits = 0
    n = len(l)
    while n > 0:
        digits += 1
        n //= 10
    format = '%0' + '%s' % digits + 'i:%s'
    for i in range(len(l)):
        print(format % (i, l[i]))
        yield None

def printListEnum(l):
    for result in printListEnumGen(l):
        pass

# base class for all Panda C++ objects
# libdtoolconfig doesn't seem to have this, grab it off of TypedObject
dtoolSuperBase = None

def _getDtoolSuperBase():
    global dtoolSuperBase
    from panda3d.core import TypedObject
    dtoolSuperBase = TypedObject.__bases__[0]
    assert dtoolSuperBase.__name__ == 'DTOOL_SUPER_BASE'

safeReprNotify = None

def _getSafeReprNotify():
    global safeReprNotify
    from direct.directnotify.DirectNotifyGlobal import directNotify
    safeReprNotify = directNotify.newCategory("safeRepr")
    return safeReprNotify

def safeRepr(obj):
    global dtoolSuperBase
    if dtoolSuperBase is None:
        _getDtoolSuperBase()

    global safeReprNotify
    if safeReprNotify is None:
        _getSafeReprNotify()

    if isinstance(obj, dtoolSuperBase):
        # repr of C++ object could crash, particularly if the object has been deleted
        # log that we're calling repr
        safeReprNotify.info('calling repr on instance of %s.%s' % (obj.__class__.__module__, obj.__class__.__name__))
        sys.stdout.flush()

    try:
        return repr(obj)
    except:
        return '<** FAILED REPR OF %s instance at %s **>' % (obj.__class__.__name__, hex(id(obj)))

def safeReprTypeOnFail(obj):
    global dtoolSuperBase
    if dtoolSuperBase is None:
        _getDtoolSuperBase()

    global safeReprNotify
    if safeReprNotify is None:
        _getSafeReprNotify()

    if isinstance(obj, dtoolSuperBase):
        return type(obj)

    try:
        return repr(obj)
    except:
        return '<** FAILED REPR OF %s instance at %s **>' % (obj.__class__.__name__, hex(id(obj)))



def fastRepr(obj, maxLen=200, strFactor=10, _visitedIds=None):
    """ caps the length of iterable types, so very large objects will print faster.
    also prevents infinite recursion """
    try:
        if _visitedIds is None:
            _visitedIds = set()
        if id(obj) in _visitedIds:
            return '<ALREADY-VISITED %s>' % itype(obj)
        if type(obj) in (tuple, list):
            s = ''
            s += {tuple: '(',
                  list:  '[',}[type(obj)]
            if maxLen is not None and len(obj) > maxLen:
                o = obj[:maxLen]
                ellips = '...'
            else:
                o = obj
                ellips = ''
            _visitedIds.add(id(obj))
            for item in o:
                s += fastRepr(item, maxLen, _visitedIds=_visitedIds)
                s += ', '
            _visitedIds.remove(id(obj))
            s += ellips
            s += {tuple: ')',
                  list:  ']',}[type(obj)]
            return s
        elif type(obj) is dict:
            s = '{'
            if maxLen is not None and len(obj) > maxLen:
                o = list(obj.keys())[:maxLen]
                ellips = '...'
            else:
                o = list(obj.keys())
                ellips = ''
            _visitedIds.add(id(obj))
            for key in o:
                value = obj[key]
                s += '%s: %s, ' % (fastRepr(key, maxLen, _visitedIds=_visitedIds),
                                   fastRepr(value, maxLen, _visitedIds=_visitedIds))
            _visitedIds.remove(id(obj))
            s += ellips
            s += '}'
            return s
        elif type(obj) is str:
            if maxLen is not None:
                maxLen *= strFactor
            if maxLen is not None and len(obj) > maxLen:
                return safeRepr(obj[:maxLen])
            else:
                return safeRepr(obj)
        else:
            r = safeRepr(obj)
            maxLen *= strFactor
            if len(r) > maxLen:
                r = r[:maxLen]
            return r
    except:
        return '<** FAILED REPR OF %s **>' % obj.__class__.__name__

def convertTree(objTree, idList):
    newTree = {}
    for key in list(objTree.keys()):
        obj = (idList[key],)
        newTree[obj] = {}
        r_convertTree(objTree[key], newTree[obj], idList)
    return newTree

def r_convertTree(oldTree, newTree, idList):
    for key in list(oldTree.keys()):

        obj = idList.get(key)
        if(not obj):
            continue
        obj = str(obj)[:100]

        newTree[obj] = {}
        r_convertTree(oldTree[key], newTree[obj], idList)


def pretty_print(tree):
    for name in tree.keys():
        print(name)
        r_pretty_print(tree[name], 0)


def r_pretty_print(tree, num):
    num+=1
    for name in tree.keys():
        print("  "*num,name)
        r_pretty_print(tree[name],num)


def isDefaultValue(x):
    return x == type(x)()


def appendStr(obj, st):
    """adds a string onto the __str__ output of an instance"""
    def appendedStr(oldStr, st, self):
        return oldStr() + st
    oldStr = getattr(obj, '__str__', None)
    if oldStr is None:
        def stringer(s):
            return s
        oldStr = Functor(stringer, str(obj))
        stringer = None
    if sys.version_info >= (3, 0):
        obj.__str__ = types.MethodType(Functor(appendedStr, oldStr, st), obj)
    else:
        obj.__str__ = types.MethodType(Functor(appendedStr, oldStr, st), obj, obj.__class__)
    appendedStr = None
    return obj


class ScratchPad:
    """empty class to stick values onto"""
    def __init__(self, **kArgs):
        for key, value in kArgs.items():
            setattr(self, key, value)
        self._keys = set(kArgs.keys())
    def add(self, **kArgs):
        for key, value in kArgs.items():
            setattr(self, key, value)
        self._keys.update(list(kArgs.keys()))
    def destroy(self):
        for key in self._keys:
            delattr(self, key)

    # allow dict [] syntax
    def __getitem__(self, itemName):
        return getattr(self, itemName)
    def get(self, itemName, default=None):
        return getattr(self, itemName, default)
    # allow 'in'
    def __contains__(self, itemName):
        return itemName in self._keys

class Sync:
    _SeriesGen = SerialNumGen()
    def __init__(self, name, other=None):
        self._name = name
        if other is None:
            self._series = self._SeriesGen.next()
            self._value = 0
        else:
            self._series = other._series
            self._value = other._value
    def invalidate(self):
        self._value = None
    def change(self):
        self._value += 1
    def sync(self, other):
        if (self._series != other._series) or (self._value != other._value):
            self._series = other._series
            self._value = other._value
            return True
        else:
            return False
    def isSynced(self, other):
        return ((self._series == other._series) and
                (self._value == other._value))
    def __repr__(self):
        return '%s(%s)<family=%s,value=%s>' % (self.__class__.__name__,
                              self._name, self._series, self._value)

def itype(obj):
    # version of type that gives more complete information about instance types
    global dtoolSuperBase
    t = type(obj)
    if sys.version_info < (3, 0) and t is types.InstanceType:
        return "<type 'instance' of <class %s>>" % (obj.__class__)
    else:
        # C++ object instances appear to be types via type()
        # check if this is a C++ object
        if dtoolSuperBase is None:
            _getDtoolSuperBase()
        if isinstance(obj, dtoolSuperBase):
            return "<type 'instance' of %s>" % (obj.__class__)
        return t

def deeptype(obj, maxLen=100, _visitedIds=None):
    if _visitedIds is None:
        _visitedIds = set()
    if id(obj) in _visitedIds:
        return '<ALREADY-VISITED %s>' % itype(obj)
    t = type(obj)
    if t in (tuple, list):
        s = ''
        s += {tuple: '(',
              list:  '[',}[type(obj)]
        if maxLen is not None and len(obj) > maxLen:
            o = obj[:maxLen]
            ellips = '...'
        else:
            o = obj
            ellips = ''
        _visitedIds.add(id(obj))
        for item in o:
            s += deeptype(item, maxLen, _visitedIds=_visitedIds)
            s += ', '
        _visitedIds.remove(id(obj))
        s += ellips
        s += {tuple: ')',
              list:  ']',}[type(obj)]
        return s
    elif type(obj) is dict:
        s = '{'
        if maxLen is not None and len(obj) > maxLen:
            o = list(obj.keys())[:maxLen]
            ellips = '...'
        else:
            o = list(obj.keys())
            ellips = ''
        _visitedIds.add(id(obj))
        for key in o:
            value = obj[key]
            s += '%s: %s, ' % (deeptype(key, maxLen, _visitedIds=_visitedIds),
                               deeptype(value, maxLen, _visitedIds=_visitedIds))
        _visitedIds.remove(id(obj))
        s += ellips
        s += '}'
        return s
    else:
        return str(itype(obj))

def getNumberedTypedString(items, maxLen=5000, numPrefix=''):
    """get a string that has each item of the list on its own line,
    and each item is numbered on the left from zero"""
    digits = 0
    n = len(items)
    while n > 0:
        digits += 1
        n //= 10
    digits = digits
    format = numPrefix + '%0' + '%s' % digits + 'i:%s \t%s'
    first = True
    s = ''
    snip = '<SNIP>'
    for i in xrange(len(items)):
        if not first:
            s += '\n'
        first = False
        objStr = fastRepr(items[i])
        if len(objStr) > maxLen:
            objStr = '%s%s' % (objStr[:(maxLen-len(snip))], snip)
        s += format % (i, itype(items[i]), objStr)
    return s

def getNumberedTypedSortedString(items, maxLen=5000, numPrefix=''):
    """get a string that has each item of the list on its own line,
    the items are stringwise-sorted, and each item is numbered on
    the left from zero"""
    digits = 0
    n = len(items)
    while n > 0:
        digits += 1
        n //= 10
    digits = digits
    format = numPrefix + '%0' + '%s' % digits + 'i:%s \t%s'
    snip = '<SNIP>'
    strs = []
    for item in items:
        objStr = fastRepr(item)
        if len(objStr) > maxLen:
            objStr = '%s%s' % (objStr[:(maxLen-len(snip))], snip)
        strs.append(objStr)
    first = True
    s = ''
    strs.sort()
    for i in xrange(len(strs)):
        if not first:
            s += '\n'
        first = False
        objStr = strs[i]
        s += format % (i, itype(items[i]), strs[i])
    return s

def printNumberedTyped(items, maxLen=5000):
    """print out each item of the list on its own line,
    with each item numbered on the left from zero"""
    digits = 0
    n = len(items)
    while n > 0:
        digits += 1
        n //= 10
    digits = digits
    format = '%0' + '%s' % digits + 'i:%s \t%s'
    for i in xrange(len(items)):
        objStr = fastRepr(items[i])
        if len(objStr) > maxLen:
            snip = '<SNIP>'
            objStr = '%s%s' % (objStr[:(maxLen-len(snip))], snip)
        print(format % (i, itype(items[i]), objStr))

def printNumberedTypesGen(items, maxLen=5000):
    digits = 0
    n = len(items)
    while n > 0:
        digits += 1
        n //= 10
    digits = digits
    format = '%0' + '%s' % digits + 'i:%s'
    for i in xrange(len(items)):
        print(format % (i, itype(items[i])))
        yield None

def printNumberedTypes(items, maxLen=5000):
    """print out the type of each item of the list on its own line,
    with each item numbered on the left from zero"""
    for result in printNumberedTypesGen(items, maxLen):
        yield result

class DelayedCall:
    """ calls a func after a specified delay """
    def __init__(self, func, name=None, delay=None):
        if name is None:
            name = 'anonymous'
        if delay is None:
            delay = .01
        self._func = func
        self._taskName = 'DelayedCallback-%s' % name
        self._delay = delay
        self._finished = False
        self._addDoLater()
    def destroy(self):
        self._finished = True
        self._removeDoLater()
    def finish(self):
        if not self._finished:
            self._doCallback(None)
        self.destroy()
    def _addDoLater(self):
        taskMgr.doMethodLater(self._delay, self._doCallback, self._taskName)
    def _removeDoLater(self):
        taskMgr.remove(self._taskName)
    def _doCallback(self, task):
        self._finished = True
        func = self._func
        del self._func
        func()

class FrameDelayedCall:
    """ calls a func after N frames """
    def __init__(self, name, callback, frames=None, cancelFunc=None):
        # checkFunc is optional; called every frame, if returns True, FrameDelay is cancelled
        # and callback is not called
        if frames is None:
            frames = 1
        self._name = name
        self._frames = frames
        self._callback = callback
        self._cancelFunc = cancelFunc
        self._taskName = uniqueName('%s-%s' % (self.__class__.__name__, self._name))
        self._finished = False
        self._startTask()
    def destroy(self):
        self._finished = True
        self._stopTask()
    def finish(self):
        if not self._finished:
            self._finished = True
            self._callback()
        self.destroy()
    def _startTask(self):
        taskMgr.add(self._frameTask, self._taskName)
        self._counter = 0
    def _stopTask(self):
        taskMgr.remove(self._taskName)
    def _frameTask(self, task):
        if self._cancelFunc and self._cancelFunc():
            self.destroy()
            return task.done
        self._counter += 1
        if self._counter >= self._frames:
            self.finish()
            return task.done
        return task.cont

class DelayedFunctor:
    """ Waits for this object to be called, then calls supplied functor after a delay.
    Effectively inserts a time delay between the caller and the functor. """
    def __init__(self, functor, name=None, delay=None):
        self._functor = functor
        self._name = name
        # FunctionInterval requires __name__
        self.__name__ = self._name
        self._delay = delay
    def _callFunctor(self):
        cb = Functor(self._functor, *self._args, **self._kwArgs)
        del self._functor
        del self._name
        del self._delay
        del self._args
        del self._kwArgs
        del self._delayedCall
        del self.__name__
        cb()
    def __call__(self, *args, **kwArgs):
        self._args = args
        self._kwArgs = kwArgs
        self._delayedCall = DelayedCall(self._callFunctor, self._name, self._delay)

class SubframeCall:
    """Calls a callback at a specific time during the frame using the
    task system"""
    def __init__(self, functor, taskPriority, name=None):
        self._functor = functor
        self._name = name
        self._taskName = uniqueName('SubframeCall-%s' % self._name)
        taskMgr.add(self._doCallback,
                    self._taskName,
                    priority=taskPriority)
    def _doCallback(self, task):
        functor = self._functor
        del self._functor
        functor()
        del self._name
        self._taskName = None
        return task.done
    def cleanup(self):
        if (self._taskName):
            taskMgr.remove(self._taskName)
            self._taskName = None

class PStatScope:
    collectors = {}

    def __init__(self, level = None):
        self.levels = []
        if level:
            self.levels.append(level)

    def copy(self, push = None):
        c = PStatScope()
        c.levels = self.levels[:]
        if push:
            c.push(push)
        return c

    def __repr__(self):
        return 'PStatScope - \'%s\'' % (self,)

    def __str__(self):
        return ':'.join(self.levels)

    def push(self, level):
        self.levels.append(level.replace('_',''))

    def pop(self):
        return self.levels.pop()

    def start(self, push = None):
        if push:
            self.push(push)
            pass
        self.getCollector().start()

    def stop(self, pop = False):
        self.getCollector().stop()
        if pop:
            self.pop()

    def getCollector(self):
        label = str(self)
        if label not in self.collectors:
            from panda3d.core import PStatCollector
            self.collectors[label] = PStatCollector(label)
            pass
        # print '  ',self.collectors[label]
        return self.collectors[label]

def pstatcollect(scope, level = None):
    def decorator(f):
        return f

    try:

        if not (__dev__ or ConfigVariableBool('force-pstatcollect', False)) or \
           not scope:
            return decorator

        def decorator(f):
            def wrap(*args, **kw):
                scope.start(push = (level or f.__name__))
                val = f(*args, **kw)
                scope.stop(pop = True)
                return val
            return wrap

        pass
    except:
        pass

    return decorator

__report_indent = 0
def report(types = [], prefix = '', xform = None, notifyFunc = None, dConfigParam = []):
    """
    This is a decorator generating function.  Use is similar to
    a @decorator, except you must be sure to call it as a function.
    It actually returns the decorator which is then used to transform
    your decorated function. Confusing at first, I know.

    Decoration occurs at function definition time.

    If __dev__ is not defined, or resolves to False, this function
    has no effect and no wrapping/transform occurs.  So in production,
    it's as if the report has been asserted out.

    Parameters:
        types: A subset list of ['timeStamp', 'frameCount', 'avLocation']
            This allows you to specify certain useful bits of info:

              - *module*: Prints the module that this report statement
                can be found in.
              - *args*: Prints the arguments as they were passed to this
                function.
              - *timeStamp*: Adds the current frame time to the output.
              - *deltaStamp*: Adds the current AI synched frame time to
                the output
              - *frameCount*: Adds the current frame count to the output.
                Usually cleaner than the timeStamp output.
              - *avLocation*: Adds the localAvatar's network location to
                the output.  Useful for interest debugging.
              - *interests*: Prints the current interest state after the
                report.
              - *stackTrace*: Prints a stack trace after the report.

        prefix: Optional string to prepend to output, just before the
            function.  Allows for easy grepping and is useful when
            merging AI/Client reports into a single file.

        xform:  Optional callback that accepts a single parameter:
            argument 0 to the decorated function. (assumed to be 'self')
            It should return a value to be inserted into the report
            output string.

        notifyFunc: A notify function such as info, debug, warning, etc.
            By default the report will be printed to stdout. This will
            allow you send the report to a designated 'notify' output.

        dConfigParam: A list of Config.prc string variables.
            By default the report will always print.  If you specify
            this param, it will only print if one of the specified
            config strings resolve to True.
    """


    def indent(str):
        global __report_indent
        return ' '*__report_indent+str

    def decorator(f):
        return f

    try:
        if not (__dev__ or config.GetBool('force-reports', 0)):
            return decorator

        # determine whether we should use the decorator
        # based on the value of dConfigParam.
        dConfigParamList = []
        doPrint = False
        if not dConfigParam:
            doPrint = True
        else:

            if not isinstance(dConfigParam, (list,tuple)):
                dConfigParams = (dConfigParam,)
            else:
                dConfigParams = dConfigParam

            dConfigParamList = [param for param in dConfigParams \
                                if config.GetBool('want-%s-report' % (param,), 0)]

            doPrint = bool(dConfigParamList)
            pass

        if not doPrint:
            return decorator

        # Determine any prefixes defined in our Config.prc.
        if prefix:
            prefixes = set([prefix])
        else:
            prefixes = set()
            pass

        for param in dConfigParamList:
            prefix = config.GetString('prefix-%s-report' % (param,), '')
            if prefix:
                prefixes.add(prefix)
                pass
            pass

    except NameError as e:
        return decorator

    globalClockDelta = importlib.import_module("direct.distributed.ClockDelta").globalClockDelta

    def decorator(f):
        def wrap(*args,**kwargs):
            if args:
                rArgs = [args[0].__class__.__name__ + ', ']
            else:
                rArgs = []

            if 'args' in types:
                rArgs += [repr(x)+', ' for x in args[1:]] + \
                         [ x + ' = ' + '%s, ' % repr(y) for x,y in kwargs.items()]

            if not rArgs:
                rArgs = '()'
            else:
                rArgs = '(' + reduce(str.__add__,rArgs)[:-2] + ')'


            outStr = '%s%s' % (f.__name__, rArgs)

            # Insert prefix place holder, if needed
            if prefixes:
                outStr = '%%s %s' % (outStr,)


            if 'module' in types:
                outStr = '%s {M:%s}' % (outStr, f.__module__.split('.')[-1])

            if 'frameCount' in types:
                outStr = '%-8d : %s' % (globalClock.getFrameCount(), outStr)

            if 'timeStamp' in types:
                outStr = '%-8.3f : %s' % (globalClock.getFrameTime(), outStr)

            if 'deltaStamp' in types:
                outStr = '%-8.2f : %s' % (globalClock.getRealTime() - \
                                         globalClockDelta.delta, outStr)
            if 'avLocation' in types:
                outStr = '%s : %s' % (outStr, str(localAvatar.getLocation()))

            if xform:
                outStr = '%s : %s' % (outStr, xform(args[0]))

            if prefixes:
                # This will print the same report once for each prefix
                for prefix in prefixes:
                    if notifyFunc:
                        notifyFunc(outStr % (prefix,))
                    else:
                        print(indent(outStr % (prefix,)))
            else:
                if notifyFunc:
                    notifyFunc(outStr)
                else:
                    print(indent(outStr))

            if 'interests' in types:
                base.cr.printInterestSets()

            if 'stackTrace' in types:
                print(StackTrace())

            global __report_indent
            rVal = None
            try:
                __report_indent += 1
                rVal = f(*args,**kwargs)
            finally:
                __report_indent -= 1
                if rVal is not None:
                    print(indent(' -> '+repr(rVal)))
                    pass
                pass
            return rVal

        wrap.__name__ = f.__name__
        wrap.__dict__ = f.__dict__
        wrap.__doc__ = f.__doc__
        wrap.__module__ = f.__module__
        return wrap
    return decorator

def getBase():
    try:
        return base
    except:
        return simbase

def getRepository():
    try:
        return base.cr
    except:
        return simbase.air

exceptionLoggedNotify = None
if __debug__:
    def exceptionLogged(append=True):
        """decorator that outputs the function name and all arguments
        if an exception passes back through the stack frame
        if append is true, string is appended to the __str__ output of
        the exception. if append is false, string is printed to the log
        directly. If the output will take up many lines, it's recommended
        to set append to False so that the exception stack is not hidden
        by the output of this decorator.
        """
        try:
            null = not __dev__
        except:
            null = not __debug__
        if null:
            # if we're not in __dev__, just return the function itself. This
            # results in zero runtime overhead, since decorators are evaluated
            # at module-load.
            def nullDecorator(f):
                return f
            return nullDecorator

        def _decoratorFunc(f, append=append):
            global exceptionLoggedNotify
            if exceptionLoggedNotify is None:
                from direct.directnotify.DirectNotifyGlobal import directNotify
                exceptionLoggedNotify = directNotify.newCategory("ExceptionLogged")
            def _exceptionLogged(*args, **kArgs):
                try:
                    return f(*args, **kArgs)
                except Exception as e:
                    try:
                        s = '%s(' % f.__name__
                        for arg in args:
                            s += '%s, ' % arg
                        for key, value in list(kArgs.items()):
                            s += '%s=%s, ' % (key, value)
                        if len(args) or len(kArgs):
                            s = s[:-2]
                        s += ')'
                        if append:
                            appendStr(e, '\n%s' % s)
                        else:
                            exceptionLoggedNotify.info(s)
                    except:
                        exceptionLoggedNotify.info(
                            '%s: ERROR IN PRINTING' % f.__name__)
                    raise
            _exceptionLogged.__doc__ = f.__doc__
            return _exceptionLogged
        return _decoratorFunc

# http://en.wikipedia.org/wiki/Golden_ratio
GoldenRatio = (1. + math.sqrt(5.)) / 2.
class GoldenRectangle:
    @staticmethod
    def getLongerEdge(shorter):
        return shorter * GoldenRatio
    @staticmethod
    def getShorterEdge(longer):
        return longer / GoldenRatio

def nullGen():
    # generator that ends immediately
    if False:
        # yield that never runs but still exists, making this func a generator
        yield None

def loopGen(l):
    # generator that yields the items of an iterable object forever
    def _gen(l):
        while True:
            for item in l:
                yield item
    gen = _gen(l)
    # don't leak
    _gen = None
    return gen

def makeFlywheelGen(objects, countList=None, countFunc=None, scale=None):
    # iterates and finally yields a flywheel generator object
    # the number of appearances for each object is controlled by passing in
    # a list of counts, or a functor that returns a count when called with
    # an object from the 'objects' list.
    # if scale is provided, all counts are scaled by the scale value and then int()'ed.
    def flywheel(index2objectAndCount):
        # generator to produce a sequence whose elements appear a specific number of times
        while len(index2objectAndCount):
            keyList = list(index2objectAndCount.keys())
            for key in keyList:
                if index2objectAndCount[key][1] > 0:
                    yield index2objectAndCount[key][0]
                    index2objectAndCount[key][1] -= 1
                if index2objectAndCount[key][1] <= 0:
                    del index2objectAndCount[key]
    # if we were not given a list of counts, create it by calling countFunc
    if countList is None:
        countList = []
        for object in objects:
            yield None
            countList.append(countFunc(object))
    if scale is not None:
        # scale the counts if we've got a scale factor
        for i in xrange(len(countList)):
            yield None
            if countList[i] > 0:
                countList[i] = max(1, int(countList[i] * scale))
    # create a dict for the flywheel to use during its iteration to efficiently select
    # the objects for the sequence
    index2objectAndCount = {}
    for i in xrange(len(countList)):
        yield None
        index2objectAndCount[i] = [objects[i], countList[i]]
    # create the flywheel generator
    yield flywheel(index2objectAndCount)

def flywheel(*args, **kArgs):
    # create a flywheel generator
    # see arguments and comments in flywheelGen above
    # example usage:
    """
    >>> for i in flywheel([1,2,3], countList=[10, 5, 1]):
    ...   print i,
    ...
    1 2 3 1 2 1 2 1 2 1 2 1 1 1 1 1
    """
    for flywheel in makeFlywheelGen(*args, **kArgs):
        pass
    return flywheel


if __debug__:
    def quickProfile(name="unnamed"):
        import pstats
        def profileDecorator(f):
            if(not config.GetBool("use-profiler",0)):
                return f
            def _profiled(*args, **kArgs):
                # must do this in here because we don't have base/simbase
                # at the time that PythonUtil is loaded
                if(not config.GetBool("profile-debug",0)):
                    #dumb timings
                    st=globalClock.getRealTime()
                    f(*args,**kArgs)
                    s=globalClock.getRealTime()-st
                    print("Function %s.%s took %s seconds"%(f.__module__, f.__name__,s))
                else:
                    import profile as prof, pstats

                    #detailed profile, stored in base.stats under (
                    if(not hasattr(base,"stats")):
                        base.stats={}
                    if(not base.stats.get(name)):
                        base.stats[name]=[]

                    prof.runctx('f(*args, **kArgs)', {'f':f,'args':args,'kArgs':kArgs},None,"t.prof")
                    s=pstats.Stats("t.prof")
                    #p=hotshot.Profile("t.prof")
                    #p.runctx('f(*args, **kArgs)', {'f':f,'args':args,'kArgs':kArgs},None)
                    #s = hotshot.stats.load("t.prof")
                    s.strip_dirs()
                    s.sort_stats("cumulative")
                    base.stats[name].append(s)

            _profiled.__doc__ = f.__doc__
            return _profiled
        return profileDecorator

def getTotalAnnounceTime():
    td=0
    for objs in base.stats.values():
        for stat in objs:
            td+=getAnnounceGenerateTime(stat)
    return td

def getAnnounceGenerateTime(stat):
    val=0
    stats=stat.stats
    for i in list(stats.keys()):
        if(i[2]=="announceGenerate"):
            newVal=stats[i][3]
            if(newVal>val):
                val=newVal
    return val


class MiniLog:
    def __init__(self, name):
        self.indent = 1
        self.name = name
        self.lines = []

    def __str__(self):
        return '%s\nMiniLog: %s\n%s\n%s\n%s' % \
               ('*'*50, self.name, '-'*50, '\n'.join(self.lines), '*'*50)

    def enterFunction(self, funcName, *args, **kw):
        rArgs = [repr(x)+', ' for x in args] + \
                [ x + ' = ' + '%s, ' % repr(y) for x,y in kw.items()]

        if not rArgs:
            rArgs = '()'
        else:
            rArgs = '(' + reduce(str.__add__,rArgs)[:-2] + ')'

        line = '%s%s' % (funcName, rArgs)
        self.appendFunctionCall(line)
        self.indent += 1

        return line

    def exitFunction(self):
        self.indent -= 1
        return self.indent

    def appendFunctionCall(self, line):
        self.lines.append(' '*(self.indent*2) + line)
        return line

    def appendLine(self, line):
        self.lines.append(' '*(self.indent*2) + '<< ' + line + ' >>')
        return line

    def flush(self):
        outStr = str(self)
        self.indent = 0
        self.lines = []
        return outStr

class MiniLogSentry:
    def __init__(self, log, funcName, *args, **kw):
        self.log = log
        if self.log:
            self.log.enterFunction(funcName, *args, **kw)

    def __del__(self):
        if self.log:
            self.log.exitFunction()
        del self.log

def logBlock(id, msg):
    print('<< LOGBLOCK(%03d)' % id)
    print(str(msg))
    print('/LOGBLOCK(%03d) >>' % id)

class HierarchyException(Exception):
    JOSWILSO = 0
    def __init__(self, owner, description):
        self.owner = owner
        self.desc = description

    def __str__(self):
        return '(%s): %s' % (self.owner, self.desc)

    def __repr__(self):
        return 'HierarchyException(%s)' % (self.owner, )

def formatTimeCompact(seconds):
    # returns string in format '1d3h22m43s'
    result = ''
    a = int(seconds)
    seconds = a % 60
    a //= 60
    if a > 0:
        minutes = a % 60
        a //= 60
        if a > 0:
            hours = a % 24
            a //= 24
            if a > 0:
                days = a
                result += '%sd' % days
            result += '%sh' % hours
        result += '%sm' % minutes
    result += '%ss' % seconds
    return result

if __debug__ and __name__ == '__main__':
    ftc = formatTimeCompact
    assert ftc(0) == '0s'
    assert ftc(1) == '1s'
    assert ftc(60) == '1m0s'
    assert ftc(64) == '1m4s'
    assert ftc(60*60) == '1h0m0s'
    assert ftc(24*60*60) == '1d0h0m0s'
    assert ftc(24*60*60 + 2*60*60 + 34*60 + 12) == '1d2h34m12s'
    del ftc

def formatTimeExact(seconds):
    # like formatTimeCompact but leaves off '0 seconds', '0 minutes' etc. for
    # times that are e.g. 1 hour, 3 days etc.
    # returns string in format '1d3h22m43s'
    result = ''
    a = int(seconds)
    seconds = a % 60
    a //= 60
    if a > 0:
        minutes = a % 60
        a //= 60
        if a > 0:
            hours = a % 24
            a //= 24
            if a > 0:
                days = a
                result += '%sd' % days
            if hours or minutes or seconds:
                result += '%sh' % hours
        if minutes or seconds:
            result += '%sm' % minutes
    if seconds or result == '':
        result += '%ss' % seconds
    return result

if __debug__ and __name__ == '__main__':
    fte = formatTimeExact
    assert fte(0) == '0s'
    assert fte(1) == '1s'
    assert fte(2) == '2s'
    assert fte(61) == '1m1s'
    assert fte(60) == '1m'
    assert fte(60*60) == '1h'
    assert fte(24*60*60) == '1d'
    assert fte((24*60*60) + (2 * 60)) == '1d0h2m'
    del fte

class AlphabetCounter:
    # object that produces 'A', 'B', 'C', ... 'AA', 'AB', etc.
    def __init__(self):
        self._curCounter = ['A']

    def next(self):
        result = ''.join([c for c in self._curCounter])
        index = -1
        while True:
            curChar = self._curCounter[index]
            if curChar == 'Z':
                nextChar = 'A'
                carry = True
            else:
                nextChar = chr(ord(self._curCounter[index])+1)
                carry = False
            self._curCounter[index] = nextChar
            if carry:
                if (-index) == len(self._curCounter):
                    self._curCounter = ['A',] + self._curCounter
                    break
                else:
                    index -= 1
                carry = False
            else:
                break
        return result

    __next__ = next

if __debug__ and __name__ == '__main__':
    def testAlphabetCounter():
        tempList = []
        ac = AlphabetCounter()
        for i in xrange(26*3):
            tempList.append(ac.next())
        assert tempList == [ 'A', 'B', 'C', 'D', 'E', 'F', 'G', 'H', 'I', 'J', 'K', 'L', 'M', 'N', 'O', 'P', 'Q', 'R', 'S', 'T', 'U', 'V', 'W', 'X', 'Y', 'Z',
                            'AA','AB','AC','AD','AE','AF','AG','AH','AI','AJ','AK','AL','AM','AN','AO','AP','AQ','AR','AS','AT','AU','AV','AW','AX','AY','AZ',
                            'BA','BB','BC','BD','BE','BF','BG','BH','BI','BJ','BK','BL','BM','BN','BO','BP','BQ','BR','BS','BT','BU','BV','BW','BX','BY','BZ',]
        ac = AlphabetCounter()
        num  = 26 # A-Z
        num += (26*26) # AA-ZZ
        num += 26 # AAZ
        num += 1 # ABA
        num += 2 # ABC
        for i in xrange(num):
            x = ac.next()
        assert x == 'ABC'
    testAlphabetCounter()
    del testAlphabetCounter


class Default:
    # represents 'use the default value'
    # useful for keyword arguments to virtual methods
    pass

superLogFile = None
def startSuperLog(customFunction = None):
    global superLogFile

    if(not superLogFile):
        superLogFile = open("c:\\temp\\superLog.txt", "w")
        def trace_dispatch(a,b,c):
            if(b=='call' and a.f_code.co_name != '?' and a.f_code.co_name.find("safeRepr")<0):
                vars = dict(a.f_locals)
                if 'self' in vars:
                    del vars['self']
                if '__builtins__' in vars:
                    del vars['__builtins__']
                for i in vars:
                    vars[i] = safeReprTypeOnFail(vars[i])

                if customFunction:
                    superLogFile.write( "before = %s\n"%customFunction())

                superLogFile.write( "%s(%s):%s:%s\n"%(a.f_code.co_filename.split("\\")[-1],a.f_code.co_firstlineno, a.f_code.co_name, vars))

                if customFunction:
                    superLogFile.write( "after = %s\n"%customFunction())



                return trace_dispatch
        sys.settrace(trace_dispatch)

def endSuperLog():
    global superLogFile
    if(superLogFile):
        sys.settrace(None)
        superLogFile.close()
        superLogFile = None

def configIsToday(configName):
    # TODO: replace usage of strptime with something else
    # returns true if config string is a valid representation of today's date
    today = time.localtime()
    confStr = config.GetString(configName, '')
    for format in ('%m/%d/%Y', '%m-%d-%Y', '%m.%d.%Y'):
        try:
            confDate = time.strptime(confStr, format)
        except ValueError:
            pass
        else:
            if (confDate.tm_year == today.tm_year and
                confDate.tm_mon == today.tm_mon and
                confDate.tm_mday == today.tm_mday):
                return True
    return False

def typeName(o):
    if hasattr(o, '__class__'):
        return o.__class__.__name__
    else:
        return o.__name__

def safeTypeName(o):
    try:
        return typeName(o)
    except:
        pass
    try:
        return type(o)
    except:
        pass
    return '<failed safeTypeName()>'

def histogramDict(l):
    d = {}
    for e in l:
        d.setdefault(e, 0)
        d[e] += 1
    return d

def unescapeHtmlString(s):
    # converts %## to corresponding character
    # replaces '+' with ' '
    result = ''
    i = 0
    while i < len(s):
        char = s[i]
        if char == '+':
            char = ' '
        elif char == '%':
            if i < (len(s)-2):
                num = int(s[i+1:i+3], 16)
                char = chr(num)
                i += 2
        i += 1
        result += char
    return result

class PriorityCallbacks:
    """ manage a set of prioritized callbacks, and allow them to be invoked in order of priority """
    def __init__(self):
        self._callbacks = []

    def clear(self):
        del self._callbacks[:]

    def add(self, callback, priority=None):
        if priority is None:
            priority = 0
        callbacks = self._callbacks
        lo = 0
        hi = len(callbacks)
        while lo < hi:
            mid = (lo + hi) // 2
            if priority < callbacks[mid][0]:
                hi = mid
            else:
                lo = mid + 1
        item = (priority, callback)
        callbacks.insert(lo, item)
        return item

    def remove(self, item):
        self._callbacks.remove(item)

    def __call__(self):
        for priority, callback in self._callbacks:
            callback()

builtins.Functor = Functor
builtins.Stack = Stack
builtins.Queue = Queue
builtins.Enum = Enum
builtins.SerialNumGen = SerialNumGen
builtins.SerialMaskedGen = SerialMaskedGen
builtins.ScratchPad = ScratchPad
builtins.uniqueName = uniqueName
builtins.serialNum = serialNum
if __debug__:
    builtins.profiled = profiled
    builtins.exceptionLogged = exceptionLogged
builtins.itype = itype
builtins.appendStr = appendStr
builtins.bound = bound
builtins.clamp = clamp
builtins.lerp = lerp
builtins.makeList = makeList
builtins.makeTuple = makeTuple
if __debug__:
    builtins.printStack = printStack
    builtins.printReverseStack = printReverseStack
    builtins.printVerboseStack = printVerboseStack
builtins.DelayedCall = DelayedCall
builtins.DelayedFunctor = DelayedFunctor
builtins.FrameDelayedCall = FrameDelayedCall
builtins.SubframeCall = SubframeCall
builtins.invertDict = invertDict
builtins.invertDictLossless = invertDictLossless
builtins.getBase = getBase
builtins.getRepository = getRepository
builtins.safeRepr = safeRepr
builtins.fastRepr = fastRepr
builtins.nullGen = nullGen
builtins.flywheel = flywheel
builtins.loopGen = loopGen
if __debug__:
    builtins.StackTrace = StackTrace
builtins.report = report
builtins.pstatcollect = pstatcollect
builtins.MiniLog = MiniLog
builtins.MiniLogSentry = MiniLogSentry
builtins.logBlock = logBlock
builtins.HierarchyException = HierarchyException
builtins.deeptype = deeptype
builtins.Default = Default
builtins.configIsToday = configIsToday
builtins.typeName = typeName
builtins.safeTypeName = safeTypeName
builtins.histogramDict = histogramDict
