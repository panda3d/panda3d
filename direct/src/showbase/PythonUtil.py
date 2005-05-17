import types
import string
import re
import math
import operator
import inspect
import os
import sys
import random
if __debug__:
    import traceback

from direct.directutil import Verify

ScalarTypes = (types.FloatType, types.IntType, types.LongType)

# NOTE: ifAbsentPut has been replaced with Python's dictionary's builtin setdefault
# before:
#     ifAbsentPut(dict, key, defaultValue)
# after:
#     dict.setdefault(key, defaultValue)
# Please use setdefault instead -- Joe

def enumerate(L):
    """Returns (0, L[0]), (1, L[1]), etc., allowing this syntax:
    for i, item in enumerate(L):
       ...

    enumerate is a built-in feature in Python 2.3, which implements it
    using an iterator. For now, we can use this quick & dirty
    implementation that returns a list of tuples that is completely
    constructed every time enumerate() is called.
    """
    return zip(xrange(len(L)), L)

import __builtin__
if hasattr(__builtin__, 'enumerate'):
    print 'enumerate is already present in __builtin__'
else:
    __builtin__.enumerate = enumerate

def unique(L1, L2):
    """Return a list containing all items in 'L1' that are not in 'L2'"""
    L2 = dict([(k,None) for k in L2])
    return [item for item in L1 if item not in L2]

def indent(stream, numIndents, str):
    """
    Write str to stream with numIndents in front of it
    """
    # To match emacs, instead of a tab character we will use 4 spaces
    stream.write('    ' * numIndents + str)



def writeFsmTree(instance, indent = 0):
    if hasattr(instance, 'parentFSM'):
        writeFsmTree(instance.parentFSM, indent-2)
    elif hasattr(instance, 'fsm'):
        name = ''
        if hasattr(instance.fsm, 'state'):
            name = instance.fsm.state.name
        print "%s: %s"%(instance.fsm.name, name)




if __debug__:
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

        def __str__(self):
            r = "Debug stack trace of %s (back %s frames):\n"%(
                self.label, len(self.trace),)
            for i in traceback.format_list(self.trace):
                r+=i
            return r

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
    if dict.has_key('self'):
        r = '%s.'%(dict['self'].__class__.__name__, )
    r+="%s("%(f.f_code.co_name, )
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
        if dict.has_key(name):
            v=str(dict[name])
            if len(v)>200:
                r+="<too big for debug>"
            else:
                r+=str(dict[name])
        else: r+="*** undefined ***"
    return r+')'

def traceParentCall():
    return traceFunctionCall(sys._getframe(2))

def printThisCall():
    print traceFunctionCall(sys._getframe(1))
    return 1 # to allow "assert printThisCall()"


if __debug__:
    def lineage(obj, verbose=0, indent=0):
        """
        return instance or class name in as a multiline string.

        Usage: print lineage(foo)

        (Based on getClassLineage())
        """
        r=""
        if type(obj) == types.ListType:
            r+=(" "*indent)+"python list\n"
        elif type(obj) == types.DictionaryType:
            r+=(" "*indent)+"python dictionary\n"
        elif type(obj) == types.ModuleType:
            r+=(" "*indent)+str(obj)+"\n"
        elif type(obj) == types.InstanceType:
            r+=lineage(obj.__class__, verbose, indent)
        elif type(obj) == types.ClassType:
            r+=(" "*indent)
            if verbose:
                r+=obj.__module__+"."
            r+=obj.__name__+"\n"
            for c in obj.__bases__:
                r+=lineage(c, verbose, indent+2)
        return r

def tron():
    sys.settrace(trace)
def trace(frame, event, arg):
    if event == 'line':
        pass
    elif event == 'call':
        print traceFunctionCall(sys._getframe(1))
    elif event == 'return':
        print "returning"
    elif event == 'exception':
        print "exception"
    return trace
def troff():
    sys.settrace(None)

#-----------------------------------------------------------------------------

def apropos(obj, *args):
    """
    Obsolete, use pdir
    """
    print 'Use pdir instead'

def getClassLineage(obj):
    """
    print object inheritance list
    """
    if type(obj) == types.DictionaryType:
        # Just a dictionary, return dictionary
        return [obj]
    elif type(obj) == types.InstanceType:
        # Instance, make a list with the instance and its class interitance
        return [obj] + getClassLineage(obj.__class__)
    elif type(obj) == types.ClassType:
        # Class, see what it derives from
        lineage = [obj]
        for c in obj.__bases__:
            lineage = lineage + getClassLineage(c)
        return lineage
    else:
        # Not what I'm looking for
        return []

def pdir(obj, str = None, fOverloaded = 0, width = None,
            fTruncate = 1, lineWidth = 75, wantPrivate = 0):
    # Remove redundant class entries
    uniqueLineage = []
    for l in getClassLineage(obj):
        if type(l) == types.ClassType:
            if l in uniqueLineage:
                break
        uniqueLineage.append(l)
    # Pretty print out directory info
    uniqueLineage.reverse()
    for obj in uniqueLineage:
        _pdir(obj, str, fOverloaded, width, fTruncate, lineWidth, wantPrivate)
        print

def _pdir(obj, str = None, fOverloaded = 0, width = None,
            fTruncate = 1, lineWidth = 75, wantPrivate = 0):
    """
    Print out a formatted list of members and methods of an instance or class
    """
    def printHeader(name):
        name = ' ' + name + ' '
        length = len(name)
        if length < 70:
            padBefore = int((70 - length)/2.0)
            padAfter = max(0,70 - length - padBefore)
            header = '*' * padBefore + name + '*' * padAfter
        print header
        print
    def printInstanceHeader(i, printHeader = printHeader):
        printHeader(i.__class__.__name__ + ' INSTANCE INFO')
    def printClassHeader(c, printHeader = printHeader):
        printHeader(c.__name__ + ' CLASS INFO')
    def printDictionaryHeader(d, printHeader = printHeader):
        printHeader('DICTIONARY INFO')
    # Print Header
    if type(obj) == types.InstanceType:
        printInstanceHeader(obj)
    elif type(obj) == types.ClassType:
        printClassHeader(obj)
    elif type (obj) == types.DictionaryType:
        printDictionaryHeader(obj)
    # Get dict
    if type(obj) == types.DictionaryType:
        dict = obj
    else:
        dict = obj.__dict__
    # Adjust width
    if width:
        maxWidth = width
    else:
        maxWidth = 10
    keyWidth = 0
    aproposKeys = []
    privateKeys = []
    remainingKeys = []
    for key in dict.keys():
        if not width:
            keyWidth = len(key)
        if str:
            if re.search(str, key, re.I):
                aproposKeys.append(key)
                if (not width) and (keyWidth > maxWidth):
                    maxWidth = keyWidth
        else:
            if key[:1] == '_':
                if wantPrivate:
                    privateKeys.append(key)
                    if (not width) and (keyWidth > maxWidth):
                        maxWidth = keyWidth
            else:
                remainingKeys.append(key)
                if (not width) and (keyWidth > maxWidth):
                    maxWidth = keyWidth
    # Sort appropriate keys
    if str:
        aproposKeys.sort()
    else:
        privateKeys.sort()
        remainingKeys.sort()
    # Print out results
    if wantPrivate:
        keys = aproposKeys + privateKeys + remainingKeys
    else:
        keys = aproposKeys + remainingKeys
    format = '%-' + `maxWidth` + 's'
    for key in keys:
        value = dict[key]
        if callable(value):
            strvalue = `Signature(value)`
        else:
            strvalue = `value`
        if fTruncate:
            # Cut off line (keeping at least 1 char)
            strvalue = strvalue[:max(1,lineWidth - maxWidth)]
        print (format % key)[:maxWidth] + '\t' + strvalue

# Magic numbers: These are the bit masks in func_code.co_flags that
# reveal whether or not the function has a *arg or **kw argument.
_POS_LIST = 4
_KEY_DICT = 8

def _is_variadic(function):
    return function.func_code.co_flags & _POS_LIST

def _has_keywordargs(function):
    return function.func_code.co_flags & _KEY_DICT

def _varnames(function):
    return function.func_code.co_varnames

def _getcode(f):
    """
    _getcode(f)
    This function returns the name and function object of a callable
    object.
    """
    def method_get(f):
        return f.__name__, f.im_func
    def function_get(f):
        return f.__name__, f
    def instance_get(f):
        if hasattr(f, '__call__'):
            method = f.__call__
            if (type(method) == types.MethodType):
                func = method.im_func
            else:
                func = method
            return ("%s%s" % (f.__class__.__name__, '__call__'), func)
        else:
            s = ("Instance %s of class %s does not have a __call__ method" %
                 (f, f.__class__.__name__))
            raise TypeError, s
    def class_get(f):
        if hasattr(f, '__init__'):
            return f.__name__, f.__init__.im_func
        else:
            return f.__name__, lambda: None
    codedict = { types.UnboundMethodType: method_get,
                 types.MethodType       : method_get,
                 types.FunctionType     : function_get,
                 types.InstanceType     : instance_get,
                 types.ClassType        : class_get,
                 }
    try:
        return codedict[type(f)](f)
    except KeyError:
        if callable(f): # eg, built-in functions and methods
            # raise ValueError, "type %s not supported yet." % type(f)
            return f.__name__, None
        else:
            raise TypeError, ("object %s of type %s is not callable." %
                                     (f, type(f)))

class Signature:
    def __init__(self, func):
        self.type = type(func)
        self.name, self.func = _getcode(func)
    def ordinary_args(self):
        n = self.func.func_code.co_argcount
        return _varnames(self.func)[0:n]
    def special_args(self):
        n = self.func.func_code.co_argcount
        x = {}
        #
        if _is_variadic(self.func):
            x['positional'] = _varnames(self.func)[n]
            if _has_keywordargs(self.func):
                x['keyword'] = _varnames(self.func)[n+1]
        elif _has_keywordargs(self.func):
            x['keyword'] = _varnames(self.func)[n]
        else:
            pass
        return x
    def full_arglist(self):
        base = list(self.ordinary_args())
        x = self.special_args()
        if x.has_key('positional'):
            base.append(x['positional'])
        if x.has_key('keyword'):
            base.append(x['keyword'])
        return base
    def defaults(self):
        defargs = self.func.func_defaults
        args = self.ordinary_args()
        mapping = {}
        if defargs is not None:
            for i in range(-1, -(len(defargs)+1), -1):
                mapping[args[i]] = defargs[i]
        else:
            pass
        return mapping
    def __repr__(self):
        if self.func:
            defaults = self.defaults()
            specials = self.special_args()
            l = []
            for arg in self.ordinary_args():
                if defaults.has_key(arg):
                    l.append( arg + '=' + str(defaults[arg]) )
                else:
                    l.append( arg )
            if specials.has_key('positional'):
                l.append( '*' + specials['positional'] )
            if specials.has_key('keyword'):
                l.append( '**' + specials['keyword'] )
            return "%s(%s)" % (self.name, string.join(l, ', '))
        else:
            return "%s(?)" % self.name


def aproposAll(obj):
    """
    Print out a list of all members and methods (including overloaded methods)
    of an instance or class
    """
    apropos(obj, fOverloaded = 1, fTruncate = 0)

def doc(obj):
    if (isinstance(obj, types.MethodType)) or \
       (isinstance(obj, types.FunctionType)):
        print obj.__doc__

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
    from direct.tkwidgets import Valuator
    # Set command if specified
    if command:
        kw['command'] = lambda x: apply(command, x)
        if parent is None:
            kw['title'] = command.__name__
    kw['dim'] = dim
    # Create toplevel if needed
    if not parent:
        vg = apply(Valuator.ValuatorGroupPanel, (parent,), kw)
    else:
        vg = apply(Valuator.ValuatorGroup,(parent,), kw)
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

def list2dict(L, value=None):
    """creates dict using elements of list, all assigned to same value"""
    return dict([(k,value) for k in L])

def invertDict(D):
    """creates a dictionary by 'inverting' D; keys are placed in the new
    dictionary under their corresponding value in the old dictionary.
    Data will be lost if D contains any duplicate values.

    >>> old = {'key1':1, 'key2':2}
    >>> invertDict(old)
    {1: 'key1', 2: 'key2'}
    """
    n = {}
    for key, value in D.items():
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
    used = dict([(k,None) for k in L1])
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
    fitSrcAngle2Dest(30,60) == 30
    fitSrcAngle2Dest(60,30) == 60
    fitSrcAngle2Dest(0,180) == 0
    fitSrcAngle2Dest(-1,180) == 359
    fitSrcAngle2Dest(-180,180) == 180
    """
    return dest + reduceAngle(src - dest)

def fitDestAngle2Src(src, dest):
    """
    given a src and destination angle, returns an equivalent dest angle
    that is within [-180..180) of src
    examples:
    fitDestAngle2Src(30,60) == 60
    fitDestAngle2Src(60,30) == 30
    fitDestAngle2Src(0,180) == -180
    fitDestAngle2Src(1,180) == 180
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


def binaryRepr(number, max_length = 32):
    # This will only work reliably for relatively small numbers.
    # Increase the value of max_length if you think you're going
    # to use long integers
    assert number < 2L << max_length
    shifts = map (operator.rshift, max_length * [number], \
                  range (max_length - 1, -1, -1))
    digits = map (operator.mod, shifts, max_length * [2])
    if not digits.count (1): return 0
    digits = digits [digits.index (1):]
    return string.join (map (repr, digits), '')

# constant profile defaults
PyUtilProfileDefaultFilename = 'profiledata'
PyUtilProfileDefaultLines = 80
PyUtilProfileDefaultSorts = ['cumulative', 'time', 'calls']

# call this from the prompt, and break back out to the prompt
# to stop profiling
#
# OR to do inline profiling, you must make a globally-visible
# function to be profiled, i.e. to profile 'self.load()', do
# something like this:
#
#        def func(self=self):
#            self.load()
#        import __builtin__
#        __builtin__.func = func
#        PythonUtil.startProfile(cmd='func()', filename='profileData')
#        del __builtin__.func
#
def startProfile(filename=PyUtilProfileDefaultFilename,
                 lines=PyUtilProfileDefaultLines,
                 sorts=PyUtilProfileDefaultSorts,
                 silent=0,
                 callInfo=1,
                 cmd='run()'):
    import profile
    profile.run(cmd, filename)
    if not silent:
        printProfile(filename, lines, sorts, callInfo)

# call this to see the results again
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

def getSetterName(valueName, prefix='set'):
    # getSetterName('color') -> 'setColor'
    # getSetterName('color', 'get') -> 'getColor'
    return '%s%s%s' % (prefix, string.upper(valueName[0]), valueName[1:])
def getSetter(targetObj, valueName, prefix='set'):
    # getSetter(smiley, 'pos') -> smiley.setPos
    return getattr(targetObj, getSetterName(valueName, prefix))

class Functor:
    def __init__(self, function, *args, **kargs):
        assert callable(function), "function should be a callable obj"
        self._function = function
        self._args = args
        self._kargs = kargs
        self.__name__ = 'Functor: %s' % self._function.__name__
        self.__doc__ = self._function.__doc__

    def __call__(self, *args, **kargs):
        """call function"""
        _args = list(self._args)
        _args.extend(args)
        _kargs = self._kargs.copy()
        _kargs.update(kargs)
        return apply(self._function,_args,_kargs)

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
    def __len__(self):
        return len(self.__list)

"""
ParamObj/ParamSet
=================

These two classes support you in the definition of a formal set of
parameters for an object type. The parameters may be safely queried/set on
an object instance at any time, and the object will react to newly-set
values immediately.

ParamSet & ParamObj also provide a mechanism for atomically setting
multiple parameter values before allowing the object to react to any of the
new values--useful when two or more parameters are interdependent and there
is risk of setting an illegal combination in the process of applying a new
set of values.

To make use of these classes, derive your object from ParamObj. Then define
a 'ParamSet' subclass that derives from the parent class' 'ParamSet' class,
and define the object's parameters within its ParamSet class. (see examples
below)

The ParamObj base class provides 'get' and 'set' functions for each
parameter if they are not defined. These default implementations
respectively set the parameter value directly on the object, and expect the
value to be available in that location for retrieval.

Classes that derive from ParamObj can optionally declare a 'get' and 'set'
function for each parameter. The setter should simply store the value in a
location where the getter can find it; it should not do any further
processing based on the new parameter value. Further processing should be
implemented in an 'apply' function. The applier function is optional, and
there is no default implementation.

NOTE: the previous value of a parameter is available inside an apply
function as 'self.getPriorValue()'

The ParamSet class declaration lists the parameters and defines a default
value for each. ParamSet instances represent a complete set of parameter
values. A ParamSet instance created with no constructor arguments will
contain the default values for each parameter. The defaults may be
overriden by passing keyword arguments to the ParamSet's constructor. If a
ParamObj instance is passed to the constructor, the ParamSet will extract
the object's current parameter values.

ParamSet.applyTo(obj) sets all of its parameter values on 'obj'.

SETTERS AND APPLIERS
====================
Under normal conditions, a call to a setter function, i.e.

 cam.setFov(90)

will actually result in the following calls being made:

 cam.setFov(90)
 cam.applyFov()

Calls to several setter functions, i.e.

 cam.setFov(90)
 cam.setViewType('cutscene')

will result in this call sequence:

 cam.setFov(90)
 cam.applyFov()
 cam.setViewType('cutscene')
 cam.applyViewType()

Suppose that you desire the view type to already be set to 'cutscene' at
the time when applyFov() is called. You could reverse the order of the set
calls, but suppose that you also want the fov to be set properly at the
time when applyViewType() is called.

In this case, you can 'lock' the params, i.e.

 cam.lockParams()
 cam.setFov(90)
 cam.setViewType('cutscene')
 cam.unlockParams()

This will result in the following call sequence:

 cam.setFov(90)
 cam.setViewType('cutscene')
 cam.applyFov()
 cam.applyViewType()

NOTE: Currently the order of the apply calls following an unlock is not
guaranteed.

EXAMPLE CLASSES
===============
Here is an example of a class that uses ParamSet/ParamObj to manage its
parameters:

class Camera(ParamObj):
    class ParamSet(ParamObj.ParamSet):
        Params = {
            'viewType': 'normal',
            'fov': 60,
            }
    ...

    def getViewType(self):
        return self.viewType
    def setViewType(self, viewType):
        self.viewType = viewType
    def applyViewType(self):
        if self.viewType == 'normal':
            ...

    def getFov(self):
        return self.fov
    def setFov(self, fov):
        self.fov = fov
    def applyFov(self):
        base.camera.setFov(self.fov)
    ...


EXAMPLE USAGE
=============

cam = Camera()
...

# set up for the cutscene
savedSettings = cam.ParamSet(cam)
cam.setViewType('closeup')
cam.setFov(90)
...

# cutscene is over, set the camera back
savedSettings.applyTo(cam)
del savedSettings

"""

class ParamObj:
    # abstract base for classes that want to support a formal parameter
    # set whose values may be queried, changed, 'bulk' changed, and
    # extracted/stored/applied all at once (see documentation above)

    # ParamSet subclass: container of parameter values. Derived class must
    # derive a new ParamSet class if they wish to define new params. See
    # documentation above.
    class ParamSet:
        Params = {
            # base class does not define any parameters, but they would
            # appear here as 'name': value,
            }

        def __init__(self, *args, **kwArgs):
            self.__class__._compileDefaultParams()
            if len(args) == 1 and len(kwArgs) == 0:
                # extract our params from an existing ParamObj instance
                obj = args[0]
                self.paramVals = {}
                for param in self.getParams():
                    self.paramVals[param] = getSetter(obj, param, 'get')()
            else:
                assert len(args) == 0
                if __debug__:
                    for arg in kwArgs.keys():
                        assert arg in self.getParams()
                self.paramVals = dict(kwArgs)
        def getValue(self, param):
            if param in self.paramVals:
                return self.paramVals[param]
            return self._Params[param]
        def applyTo(self, obj):
            # Apply our entire set of params to a ParamObj
            obj.lockParams()
            for param in self.getParams():
                getSetter(obj, param)(self.getValue(param))
            obj.unlockParams()
        # CLASS METHODS
        def getParams(cls):
            # returns safely-mutable list of param names
            cls._compileDefaultParams()
            return cls._Params.keys()
        getParams = classmethod(getParams)
        def getDefaultValue(cls, param):
            cls._compileDefaultParams()
            return cls._Params[param]
        getDefaultValue = classmethod(getDefaultValue)
        def _compileDefaultParams(cls):
            if cls.__dict__.has_key('_Params'):
                # we've already compiled the defaults for this class
                return
            bases = list(cls.__bases__)
            # bring less-derived classes to the front
            mostDerivedLast(bases)
            cls._Params = {}
            for c in (bases + [cls]):
                # make sure this base has its dict of param defaults
                c._compileDefaultParams()
                if c.__dict__.has_key('Params'):
                    # apply this class' default param values to our dict
                    cls._Params.update(c.Params)
        _compileDefaultParams = classmethod(_compileDefaultParams)
    # END PARAMSET SUBCLASS
    
    def __init__(self, *args, **kwArgs):
        # If you pass in a ParamSet obj, its values will be applied to this
        # object in the constructor.
        params = None
        if len(args) == 1 and len(kwArgs) == 0:
            # if there's one argument, assume that it's a ParamSet
            params = args[0]
        elif len(kwArgs) > 0:
            assert len(args) == 0
            # if we've got keyword arguments, make a ParamSet out of them
            params = self.ParamSet(**kwArgs)
            
        self._paramLockRefCount = 0
        # this holds dictionaries of parameter values prior to the set that we
        # are performing
        self._priorValuesStack = Stack()
        # this holds the name of the parameter that we are currently modifying
        # at the top of the stack
        self._curParamStack = Stack()

        def setterStub(param, setterFunc, self,
                       value):
            # should we apply the value now or should we wait?
            # if this obj's params are locked, we track which values have
            # been set, and on unlock, we'll call the applyers for those
            # values
            if self._paramLockRefCount > 0:
                setterFunc(value)
                priorValues = self._priorValuesStack.top()
                if param not in priorValues:
                    try:
                        priorValue = getSetter(self, param, 'get')()
                    except:
                        priorValue = None
                    priorValues[param] = priorValue
                self._paramsSet[param] = None
            else:
                # prepare for call to getPriorValue
                self._priorValuesStack.push({
                    param: getSetter(self, param, 'get')()
                    })
                setterFunc(value)
                # call the applier, if there is one
                applier = getattr(self, getSetterName(param, 'apply'), None)
                if applier is not None:
                    self._curParamStack.push(param)
                    applier()
                    self._curParamStack.pop()
                self._priorValuesStack.pop()

        # insert stub funcs for param setters
        for param in self.ParamSet.getParams():
            setterName = getSetterName(param)
            getterName = getSetterName(param, 'get')

            # is there a setter defined?
            if not hasattr(self, setterName):
                # no; provide the default
                def defaultSetter(self, value, param=param):
                    setattr(self, param, value)
                self.__class__.__dict__[setterName] = defaultSetter

            # is there a getter defined?
            if not hasattr(self, getterName):
                # no; provide the default. If there is no value set, return
                # the default
                def defaultGetter(self, param=param,
                                  default=self.ParamSet.getDefaultValue(param)):
                    return getattr(self, param, default)
                self.__class__.__dict__[getterName] = defaultGetter

            # grab a reference to the setter
            setterFunc = getattr(self, setterName)
            # if the setter is a direct member of this instance, move the setter
            # aside
            if setterName in self.__dict__:
                self.__dict__[setterName + '_MOVED'] = self.__dict__[setterName]
                setterFunc = self.__dict__[setterName]
            # install a setter stub that will a) call the real setter and
            # then the applier, or b) call the setter and queue the
            # applier, depending on whether our params are locked
            self.__dict__[setterName] = Functor(setterStub, param,
                                                setterFunc, self)

        if params is not None:
            params.applyTo(self)

    def setDefaultParams(self):
        # set all the default parameters on ourself
        self.ParamSet().applyTo(self)

    def lockParams(self):
        self._paramLockRefCount += 1
        if self._paramLockRefCount == 1:
            self._handleLockParams()
    def unlockParams(self):
        if self._paramLockRefCount > 0:
            self._paramLockRefCount -= 1
            if self._paramLockRefCount == 0:
                self._handleUnlockParams()
    def _handleLockParams(self):
        # this will store the names of the parameters that are modified
        self._paramsSet = {}
        # this will store the values of modified params (from prior to
        # the lock).
        self._priorValuesStack.push({})
    def _handleUnlockParams(self):
        for param in self._paramsSet:
            # call the applier, if there is one
            applier = getattr(self, getSetterName(param, 'apply'), None)
            if applier is not None:
                self._curParamStack.push(param)
                applier()
                self._curParamStack.pop()
        self._priorValuesStack.pop()
        del self._paramsSet
    def paramsLocked(self):
        return self._paramLockRefCount > 0
    def getPriorValue(self):
        # call this within an apply function to find out what the prior value
        # of the param was
        return self._priorValuesStack.top()[self._curParamStack.top()]

def bound(value, bound1, bound2):
    """
    returns value if value is between bound1 and bound2
    otherwise returns bound that is closer to value
    """
    if bound1 > bound2:
        return min(max(value, bound2), bound1)
    else:
        return min(max(value, bound1), bound2)

def lerp(v0, v1, t):
    """
    returns a value lerped between v0 and v1, according to t
    t == 0 maps to v0, t == 1 maps to v1
    """
    return v0 + (t * (v1 - v0))

def average(*args):
    """ returns simple average of list of values """
    val = 0.
    for arg in args:
        val += arg
    return val / len(args)

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
    return stackEntryInfo(1)

def callerInfo(baseFileName=1):
    """
    returns the sourcefilename, line number, and function name of the
    caller of the function that called this function
    (answers the question: 'hey callerInfo, who called me?')
    see stackEntryInfo, above, for info on 'baseFileName' and return types
    """
    return stackEntryInfo(2)

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
    fileName, lineNum, funcName = callerInfo()
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

def describeException(backTrace = 4):
    # When called in an exception handler, returns a string describing
    # the current exception.

    def byteOffsetToLineno(code, byte):
        # Returns the source line number corresponding to the given byte
        # offset into the indicated Python code module.

        import array
        lnotab = array.array('B', code.co_lnotab)

        line   = code.co_firstlineno
        for i in range(0, len(lnotab),2):
            byte -= lnotab[i]
            if byte <= 0:
                return line
            line += lnotab[i+1]

        return line

    infoArr = sys.exc_info()
    exception = infoArr[0]
    exceptionName = getattr(exception, '__name__', None)
    extraInfo = infoArr[1]
    trace = infoArr[2]

    stack = []
    while trace.tb_next:
        # We need to call byteOffsetToLineno to determine the true
        # line number at which the exception occurred, even though we
        # have both trace.tb_lineno and frame.f_lineno, which return
        # the correct line number only in non-optimized mode.
        frame = trace.tb_frame
        module = frame.f_globals.get('__name__', None)
        lineno = byteOffsetToLineno(frame.f_code, frame.f_lasti)
        stack.append("%s:%s, " % (module, lineno))
        trace = trace.tb_next

    frame = trace.tb_frame
    module = frame.f_globals.get('__name__', None)
    lineno = byteOffsetToLineno(frame.f_code, frame.f_lasti)
    stack.append("%s:%s, " % (module, lineno))

    description = ""
    for i in range(len(stack) - 1, max(len(stack) - backTrace, 0) - 1, -1):
        description += stack[i]

    description += "%s: %s" % (exceptionName, extraInfo)
    return description

def mostDerivedLast(classList):
    """pass in list of classes. sorts list in-place, with derived classes
    appearing after their bases"""
    def compare(a,b):
        if issubclass(a,b):
            result=1
        elif issubclass(b,a):
            result=-1
        else:
            result=0
        #print a,b,result
        return result
    classList.sort(compare)

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
    """given a list of (weight,item) pairs, chooses an item based on the
    weights. rng must return 0..1. if you happen to have the sum of the
    weights, pass it in 'sum'."""
    # TODO: add support for dicts
    if sum is None:
        sum = 0.
        for weight, item in choiceList:
            sum += weight

    rand = rng()
    accum = rand * sum
    for weight, item in choiceList:
        accum -= weight
        if accum <= 0.:
            return item
    # rand is ~1., and floating-point error prevented accum from hitting 0.
    # Or you passed in a 'sum' that was was too large.
    # Return the last item.
    return item

def randFloat(a, b=0., rng=random.random):
    """returns a random float in [a,b]
    call with single argument to generate random float between arg and zero
    """
    return lerp(a,b,rng())

def normalDistrib(a, b, gauss=random.gauss):
    """
    NOTE: assumes a < b

    Returns random number between a and b, using gaussian distribution, with
    mean=avg(a,b), and a standard deviation that fits ~99.7% of the curve
    between a and b. Outlying results are clipped to a and b.

    ------------------------------------------------------------------------
    http://www-stat.stanford.edu/~naras/jsm/NormalDensity/NormalDensity.html

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
    return max(a, min(b, gauss((a+b)*.5, (b-a)/6.)))

def weightedRand(valDict, rng=random.random):
    """
    pass in a dictionary with a selection -> weight mapping.  Eg.
    {"Choice 1" : 10,
     "Choice 2" : 30,
     "bear"     : 100}

    -Weights need not add up to any particular value.
    -The actual selection will be returned.
    """
    selections = valDict.keys()
    weights = valDict.values()

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

    assert(True, "Should never get here")
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
        InvalidChars = string.whitespace
        def _checkValidIdentifier(item):
            invalidChars = string.whitespace+string.punctuation
            invalidChars = invalidChars.replace('_','')
            invalidFirstChars = invalidChars+string.digits
            if item[0] in invalidFirstChars:
                raise SyntaxError, ("Enum '%s' contains invalid first char" %
                                    item)
            if not disjoint(item, invalidChars):
                for char in item:
                    if char in invalidChars:
                        raise SyntaxError, (
                            "Enum\n'%s'\ncontains illegal char '%s'" %
                            (item, char))
            return 1
        _checkValidIdentifier = staticmethod(_checkValidIdentifier)

    def __init__(self, items, start=0):
        if type(items) == types.StringType:
            items = items.split(',')

        self._stringTable = {}

        # make sure we don't overwrite an existing element of the class
        assert(self._checkExistingMembers(items))
        assert(uniqueElements(items))

        i = start
        for item in items:
            # remove leading/trailing whitespace
            item = string.strip(item)
            # is there anything left?
            if len(item) == 0:
                continue
            # make sure there are no invalid characters
            assert(Enum._checkValidIdentifier(item))
            self.__dict__[item] = i
            self._stringTable[i] = item
            i += 1

    def getString(self, value):
        return self._stringTable[value]

    def __contains__(self, value):
        return value in self._stringTable

    def __len__(self):
        return len(self._stringTable)

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
#              def __init__(self,...):
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
    def __init__(cls,name,bases,dic):
        super(Singleton,cls).__init__(name,bases,dic)
        cls.instance=None
    def __call__(cls,*args,**kw):
        if cls.instance is None:
            cls.instance=super(Singleton,cls).__call__(*args,**kw)
        return cls.instance

class SingletonError(ValueError):
    """ Used to indicate an inappropriate value for a Singleton."""

