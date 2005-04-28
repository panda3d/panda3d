import os
import finder
import re
import sys

def makefilter(name, xtrapath=None):
    typ, nm, fullname = finder.identify(name, xtrapath)
    if typ in (finder.SCRIPT, finder.GSCRIPT, finder.MODULE):
        return ModFilter([os.path.splitext(nm)[0]])
    if typ == finder.PACKAGE:
        return PkgFilter([fullname])
    if typ == finder.DIRECTORY:
        return DirFilter([fullname])
    if typ in (finder.BINARY, finder.PBINARY):
        return FileFilter([nm])
    return FileFilter([fullname])
  
class _Filter:
    def __repr__(self):
        return '<'+self.__class__.__name__+' '+repr(self.elements)+'>'
    
class _NameFilter(_Filter):
    """ A filter mixin that matches (exactly) on name """
    def matches(self, res):
        return self.elements.get(res.name, 0)
        
class _PathFilter(_Filter):
    """ A filter mixin that matches if the resource is below any of the paths"""
    def matches(self, res):
        p = os.path.normcase(os.path.abspath(res.path))
        while len(p) > 3:
            p = os.path.dirname(p)
            if self.elements.get(p, 0):
                return 1
        return 0
        
class _ExtFilter(_Filter):
    """ A filter mixin that matches based on file extensions (either way) """
    include = 0
    def matches(self, res):
        fnd = self.elements.get(os.path.splitext(res.path)[1], 0)
        if self.include:
            return not fnd
        return fnd
    
class _TypeFilter(_Filter):
    """ A filter mixin that matches on resource type (either way) """
    include = 0
    def matches(self, res):
        fnd = self.elements.get(res.typ, 0)
        if self.include:
            return not fnd
        return fnd

class _PatternFilter(_Filter):
    """ A filter that matches if re.search succeeds on the resource path """
    def matches(self, res):
        for regex in self.elements:
            if regex.search(res.path):
                return 1
        return 0
    
class ExtFilter(_ExtFilter):
    """ A file extension filter.
        ExtFilter(extlist, include=0)
        where extlist is a list of file extensions """
    def __init__(self, extlist, include=0):
        self.elements = {}
        for ext in extlist:
            if ext[0:1] != '.':
                ext = '.'+ext
            self.elements[ext] = 1
        self.include = include

class TypeFilter(_TypeFilter):
    """ A filter for resource types.
        TypeFilter(typlist, include=0)
        where typlist is a subset of ['a','b','d','m','p','s','x','z'] """
    def __init__(self, typlist, include=0):
        self.elements = {}
        for typ in typlist:
            self.elements[typ] = 1
        self.include = include

class FileFilter(_NameFilter):
    """ A filter for data files """
    def __init__(self, filelist):
        self.elements = {}
        for f in filelist:
            self.elements[f] = 1
              
class ModFilter(_NameFilter):
    """ A filter for Python modules.
        ModFilter(modlist) where modlist is eg ['macpath', 'dospath'] """
    def __init__(self, modlist):
        self.elements = {}
        for mod in modlist:
            self.elements[mod] = 1
            
class DirFilter(_PathFilter):
    """ A filter based on directories.
        DirFilter(dirlist)
        dirs may be relative and will be normalized.
        Subdirectories of dirs will be excluded. """
    def __init__(self, dirlist):
        self.elements = {}
        for pth in dirlist:
            pth = os.path.normcase(os.path.abspath(pth))
            self.elements[pth] = 1
            
class PkgFilter(_PathFilter):
    """At this time, identical to a DirFilter (being lazy) """
    def __init__(self, pkglist):
        #warning - pkgs are expected to be full directories
        self.elements = {}
        for pkg in pkglist:
            pth = os.path.normcase(os.path.abspath(pkg))
            self.elements[pth] = 1
            
class StdLibFilter(_PathFilter):
    """ A filter that excludes anything found in the standard library """
    def __init__(self):
        pth = os.path.normcase(os.path.join(sys.exec_prefix, 'lib'))
        self.elements = {pth:1}
     
class PatternFilter(_PatternFilter):
    """ A filter that excludes if any pattern is found in resource's path """
    def __init__(self, patterns):
        self.elements = []
        for pat in patterns:
            self.elements.append(re.compile(pat))
