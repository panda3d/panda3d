import types
import string
import re
import math

def ifAbsentPut(dict, key, newValue):
    """
    If dict has key, return the value, otherwise insert the newValue and return it
    """
    if dict.has_key(key):
        return dict[key]
    else:
        dict[key] = newValue
        return newValue


def indent(stream, numIndents, str):
    """
    Write str to stream with numIndents in front it it
    """
    # To match emacs, instead of a tab character we will use 4 spaces
    stream.write('    ' * numIndents + str)

def apropos(obj, *args):
    """
    Obsolete, use pdir
    """
    print 'Use pdir instead'

def getClassLineage(obj):
    """ getClassLineage(obj): print object inheritance list """
    # Just a dictionary, return dictionary
    if type(obj) == types.DictionaryType:
        return [obj]
    # Instance, make a list with the instance and its class interitance
    elif type(obj) == types.InstanceType:
        return [obj] + getClassLineage(obj.__class__)
    # Class, see what it derives from
    elif type(obj) == types.ClassType:
        lineage = [obj]
        for c in obj.__bases__:
            lineage = lineage + getClassLineage(c)
        return lineage
    # Not what I'm looking for
    else:
        return []

def pdir(obj, str = None, fOverloaded = 0, width = None,
            fTruncate = 1, lineWidth = 75):
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
        _pdir(obj, str, fOverloaded, width, fTruncate, lineWidth)
        print

def _pdir(obj, str = None, fOverloaded = 0, width = None,
            fTruncate = 1, lineWidth = 75):
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
    overloadedKeys = []
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
            if key[:2] == '__':
                privateKeys.append(key)
                if (not width) and (keyWidth > maxWidth):
                    maxWidth = keyWidth
            elif (key[:10] == 'overloaded'):
                if fOverloaded:
                    overloadedKeys.append(key)
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
        overloadedKeys.sort()
    # Print out results
    keys = aproposKeys + privateKeys + remainingKeys + overloadedKeys
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

def adjust(command = None, parent = None, **kw):
    """
    adjust(command = None, parent = None, **kw)
    Popup and entry scale to adjust a parameter
    
    Accepts any EntryScale keyword argument.  Typical arguments include:
    command: The one argument command to execute
    min: The min value of the slider
    max: The max value of the slider
    resolution: The resolution of the slider
    text: The label on the slider
    
    These values can be accessed and/or changed after the fact
    >>> es = adjust()
    >>> es['min']
    0.0
    >>> es['min'] = 10.0
    >>> es['min']
    10.0
    """
    # Make sure we enable Tk
    base.wantDIRECT = 1
    base.wantTk = 1
    import TkGlobal
    import Tkinter
    import EntryScale
    import Pmw
    # Create toplevel if needed
    if not parent:
        parent = Toplevel()
        parent.title('Parameter Adjust')
    # Set command if specified
    if command:
        kw['command'] = command
    es = apply(EntryScale.EntryScale, (parent,), kw)
    es.pack(expand = 1, fill = X)
    es.parent = parent
    return es

def intersection(a, b):
    """
    intersection(list, list):
    """
    c = a + b
    d = []
    for i in c:
        if (i in a) and (i in b):
            # make it unique, like a set
            if (i not in d):
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

def reduceAngle(deg):
    """
    Reduces an angle (in degrees) to a value between -180. and 180.
    """
    return (math.fmod((deg + 180.0), 360.0) - 180.0)

def shortestDestAngle(src, dest):
    """
    Returns a version of dest that is numerically closest to src. It is
    assumed that src is between -180. and 180.
    Example: (shortest-dest-angle 50. -170.) --> 190.
    """
    return (src + (reduceAngle(reduceAngle(dest) - reduceAngle(src))))
