# copyright McMillan Enterprises, 1999
import os, sys
import string

SCRIPT = 1
GSCRIPT = 2
MODULE = 3
PACKAGE = 4
PBINARY = 5
BINARY = 6
ZLIB = 7
DIRECTORY = 8
DATA = 9

_bpath = None
_ppath = None
_pcache = {}

def _locate(nm, xtrapath=None, base=None):
    """Find a file / directory named NM in likely places.
    
       XTRAPATH is a list of paths to prepend to BASE.
       If BASE is None, sys.path (as extended by packages) is used."""
    ppath = base
    if base is None:
        ppath = _ppath
    if xtrapath:
        ppath = xtrapath + ppath
    for pth in ppath:
        fullnm = os.path.join(pth, nm)
        #print " _locate trying", fullnm
        if os.path.exists(fullnm):
            break
    else:
        return ''
    return fullnm

def _locatepython(name, xtrapath=None):
    """Locate a Python resource named NAME.
    
       All of the standard file extensions will be tried.
       XTRAPATH is prepended to sys.path."""
    for ext in ('.py', '.pyc', '.pyw', '.pyo', '.pyd', '.dll'):
        fullnm = _locate(name+ext, xtrapath)
        if fullnm:
            break
    else:
        for ext in ('.pyd', '.dll'):
            fullnm = _locate(name+ext, [], _bpath)
            if fullnm:
                break
    return fullnm

def ispackage(name):
    """Determine if NAME is the name of a package."""
    if os.path.exists(os.path.join(name, '__init__.py')):
        return 1
    if os.path.exists(os.path.join(name, '__init__.pyc')):
        return 1
    if os.path.exists(os.path.join(name, '__init__.pyo')):
        return 1
    return 0
        
def idtype(fullnm):
    """Figure out what type of resource FULLNM refers to."""
    if os.path.isdir(fullnm):
        if ispackage(fullnm):
            return PACKAGE
        return DIRECTORY
    ext = os.path.splitext(fullnm)[1]
    if ext:
        if ext == '.pyd':
            return PBINARY
        if ext == '.dll':
            return BINARY
        if ext in ('.pyc', '.pyo'):
            return MODULE
        if ext == '.py':
            return SCRIPT
        if ext == '.pyw':
            return GSCRIPT
        if ext == '.pyz':
            return ZLIB
    return DATA

def identify(name, xtrapath=None):
    """Find, and identify the type of NAME, using XTRAPATH as the
       first place to look.

       Return type, name and full path name.
       NAME can be a logical or physical name. However, the logical
       name of a Python module can easily conflict with the physical
       name of something else, so beware."""
    if os.path.exists(name):
        fullnm = name
    else:
        if xtrapath is None:
            xtra = []
        elif _pcache.has_key(id(xtrapath)):
            xtra = _pcache[id(xtrapath)]
        else:
            xtra = expand(xtrapath)
            _pcache[id(xtrapath)] = xtra 
        fullnm = _locate(name, xtra)
        if not fullnm:
            fullnm =  _locate(name, [], _bpath)
            if not fullnm:
                ext = os.path.splitext(name)[1]
                if not ext:
                    fullnm = _locatepython(name, xtra)
                    if not fullnm:
                        raise ValueError, "%s not found" % name
                else:
                    nm = name
                    while string.count(nm, '.'):
                        nm = string.replace(nm, '.', '/', 1)
                        fullnm = _locatepython(nm, xtra)
                        if fullnm:
                            break
                    else:
                        raise ValueError, "%s not found" % name
                    
    typ = idtype(fullnm)
    nm = name
    if typ in (GSCRIPT, SCRIPT, MODULE, PACKAGE, PBINARY):
        dir, nm = os.path.split(fullnm)
        nm = os.path.splitext(nm)[0]
    if typ == SCRIPT:
        if os.path.exists(fullnm+'c') or os.path.exists(fullnm+'o'):
            typ = MODULE
    if typ in (MODULE, PACKAGE):
        while idtype(dir) == PACKAGE:
            dir, lnode = os.path.split(dir)
            nm = lnode+'.'+nm
    elif typ == BINARY:
        nm = os.path.basename(fullnm)
    return typ, nm, fullnm
 
def expand(plist):
    """ expand a list of paths (like sys.path) to include all the 
        directories that qualify as packages """
    pkgdirs = []
    for pth in plist:
        os.path.walk(pth, pkgfinder, pkgdirs)
    return plist + pkgdirs

def pkgfinder(pkgdirs, dir, fnms):
    i = 0
    while i < len(fnms):
        fnm = os.path.join(dir, fnms[i])
        if os.path.isdir(fnm):
            if ispackage(fnm):
                pkgdirs.append(fnm)
                i = i + 1
            else:
                del fnms[i]
        else:
            i = i + 1

if _bpath is None:
    try:
        import win32api
    except ImportError:
        print "Cannot determine your Windows or System directories"
        print "Please add them to your PATH if .dlls are not found"
        _bpath = []
    else:
        sysdir = win32api.GetSystemDirectory()
        sysdir2 = os.path.join(sysdir, '../SYSTEM')
        windir = win32api.GetWindowsDirectory()
        _bpath = [sysdir, sysdir2, windir]
    _bpath.extend(string.split(os.environ.get('PATH', ''), ';'))
if _ppath is None:
    _ppath = expand(sys.path)
        
def getpath():
    """Return the path that Windows will search for dlls."""
    return _bpath
