import types
import re

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
    #indentString = '\t'
    # To match emacs, instead of a tab character we will use 4 spaces
    indentString = '    '
    stream.write(indentString * numIndents + str)

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
    print
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
                if (not width) & (keyWidth > maxWidth):
                    maxWidth = keyWidth
        else:
            if key[:2] == '__':
                privateKeys.append(key)
                if (not width) & (keyWidth > maxWidth):
                    maxWidth = keyWidth
            elif (key[:10] == 'overloaded'):
                if fOverloaded:
                    overloadedKeys.append(key)
                    if (not width) & (keyWidth > maxWidth):
                        maxWidth = keyWidth
            else:
                remainingKeys.append(key)
                if (not width) & (keyWidth > maxWidth):
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
        value = `dict[key]`
        if fTruncate:
            # Cut off line (keeping at least 1 char)
            value = value[:max(1,lineWidth - maxWidth)]
        print (format % key)[:maxWidth] + '\t' + value

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
