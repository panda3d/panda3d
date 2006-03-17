import os
import string
import archivebuilder
import carchive
import tocfilter
import bindepend
import finder

_cache = {}

def makeresource(name, xtrapath=None):
    """Factory function that returns a resource subclass.

       NAME is the logical or physical name of a resource.
       XTRAPTH is a path or list of paths to search first.
       return one of the resource subclasses.
       Warning - logical names can conflict; archive might return a directory,
       when the module archive.py was desired."""
    typ, nm, fullname = finder.identify(name, xtrapath)
    fullname = os.path.normpath(fullname)
    if _cache.has_key(fullname):
        return _cache[fullname]
    elif typ in (finder.SCRIPT, finder.GSCRIPT):
        rsrc = scriptresource(nm, fullname)
    elif typ == finder.MODULE:
        rsrc = moduleresource(nm, fullname)
    elif typ == finder.PACKAGE:
        rsrc = pkgresource(nm, fullname)
    elif typ in (finder.PBINARY, finder.BINARY):
        rsrc = binaryresource(nm, fullname)
    elif typ == finder.ZLIB:
        rsrc = zlibresource(nm, fullname)
    elif typ == finder.DIRECTORY:
        rsrc = dirresource(nm, fullname)
    else:
        try:
            carchive.CArchive(fullname)
        except:
            rsrc = dataresource(nm, fullname)
        else:
            rsrc = archiveresource(nm, fullname)
    _cache[fullname] = rsrc
    return rsrc

class resource:
    """ Base class for all resources.

        contents() returns of list of what's contained (eg files in dirs)
        dependencies() for Python resources returns a list of moduleresources
         and binaryresources """
    def __init__(self, name, path, typ):
        """NAME is the logical name of the resource.
           PATH is the full path to the resource.
           TYP is the type code.
           No editting or sanity checks."""
        self.name = name
        self.path = path
        self.typ = typ
    def __repr__(self):
        return "(%(name)s, %(path)s, %(typ)s)" % self.__dict__
    def contents(self):
        """A list of resources within this resource.

           Overridable.
           Base implementation returns [self]"""
        return [self]
    def dependencies(self):
        """A list of resources this resource requires.

           Overridable.
           Base implementation returns []"""
        return []
    def __cmp__(self, other):
        if not isinstance(other, self.__class__):
            return -1
        return cmp((self.typ, self.name), (other.typ, other.name))
    def asFilter(self):
        """Create a tocfilter based on self.

           Pure virtual"""
        raise NotImplementedError
    def asSource(self):
        """Return self in source form.

           Base implementation returns self"""
        return self
    def asBinary(self):
        """Return self in binary form.

           Base implementation returns self"""
        return self

class pythonresource(resource):
    """An empty base class.

       Used to classify resources."""
    pass


class scriptresource(pythonresource):
    """ A top-level python resource.

        Has (lazily computed) attributes, modules and binaries, which together
        are the scripts dependencies() """
    def __init__(self, name, fullname):
        resource.__init__(self, name, fullname, 's')
    def __getattr__(self, name):
        if name == 'modules':
            print "Analyzing python dependencies of", self.name, self.path
            self.modules = []
            self._binaries = []
            nodes = string.split(self.name, '.')[:-1] # MEInc.Dist.archive -> ['MEInc', 'Dist']
            for i in range(len(nodes)):
                nm = string.join(nodes[:i+1], '.')
                rsrc = makeresource(nm+'.__init__')
                rsrc.name = nm
                self.modules.append(rsrc)
            for (nm, path) in archivebuilder.Dependencies(self.path):
                path = os.path.normcase(os.path.abspath(path))
                if os.path.splitext(path)[1] == '.py':
                    self.modules.append(moduleresource(nm, path))
                else:
                    self._binaries.append(binaryresource(nm, path))
            return self.modules
        elif name == 'binaries':
            x = self.modules
            tmp = {}
            for br in self._binaries:
                tmp[br.name] = br
                for br2 in br.dependencies():
                    tmp[br2.name] = br2
            self.binaries = tmp.values()
            return self.binaries
        else:
            raise AttributeError, "%s" % name
    def dependencies(self):
        """Return all dependencies (Python and binary) of self."""
        return self.modules + self.binaries
    def asFilter(self):
        """Return a ModFilter based on self."""
        return tocfilter.ModFilter([self.name])
    def asSource(self):
        """Return self as a dataresource (ie, a text file wrapper)."""
        r = dataresource(self.path)
        r.name = apply(os.path.join, string.split(self.name, '.')[:-1]+[r.name])
        return r

class moduleresource(scriptresource):
    """ A module resource (differs from script in that it will generally
        be worked with as a .pyc instead of in source form) """
    def __init__(self, name, fullname):
        resource.__init__(self, name, fullname, 'm')
    def asBinary(self):
        """Return self as a dataresource (ie, a binary file wrapper)."""
        r = dataresource(self.path)
        r.name = os.path.basename(r.name)
        r.typ = 'b'
        return r
    def asSource(self):
        """Return self as a scriptresource (ie, uncompiled form)."""
        return scriptresource(self.name, self.path[:-1]).asSource()

class binaryresource(resource):
    """A .dll or .pyd.

       dependencies() yields more binaryresources """
    def __init__(self, name, fullname):
        if string.find(name, '.') == -1:
            pth, bnm = os.path.split(fullname)
            junk, ext = os.path.splitext(bnm)
            fullname = os.path.join(pth, name + ext)
        resource.__init__(self, name, fullname, 'b')
        self._depends = None
    def dependencies(self):
        """Return a list of binary dependencies."""
        if self._depends is not None:
            return self._depends
        self._depends = []
        for (lib, path) in bindepend.Dependencies([(self.name, self.path)]):
            self._depends.append(binaryresource(lib, path))
        return self._depends
    def asFilter(self):
        """Create a FileFilter from self."""
        return tocfilter.FileFilter([self.name])

class dataresource(resource):
    """A subclass for arbitrary files. """
    def __init__(self, name, fullname=None):
        resource.__init__(self, name, fullname or name, 'x')
    def asFilter(self):
        """Create a FileFilter from self."""
        return tocfilter.FileFilter([self.name])

class archiveresource(dataresource):
    """A sublcass for CArchives. """
    def __init__(self, name, fullname=None):
        resource.__init__(self, name, fullname or name, 'a')

class zlibresource(dataresource):
    """A subclass for ZlibArchives. """
    def __init__(self, name, fullname=None):
        resource.__init__(self, name, fullname or name, 'z')

class dirresource(resource):
    """A sublcass for a directory.

       Generally transformed to a list of files through
        contents() and filtered by file extensions or resource type.
        Note that contents() is smart enough to regard a .py and .pyc
        as the same resource. """
    RECURSIVE = 0
    def __init__(self, name, fullname=None):
        resource.__init__(self, name, fullname or name, 'd')
        self._contents = None
    def contents(self, prefix=''):
        """Return the list of (typed) resources in self.name"""
        if self._contents is not None:
            return self._contents
        self._contents = []
        flist = os.listdir(self.path)
        for fnm in flist:
            try:
                bnm, ext = os.path.splitext(fnm)
                if ext == '.py' and (bnm+'.pyc' in flist or bnm+'.pyo' in flist):
                    pass
                elif ext == '.pyo' and (bnm + '.pyc' in flist):
                    pass
                else:
                    rsrc = makeresource(os.path.join(self.path, fnm))
                    if isinstance(rsrc, pkgresource):
                        rsrc = self.__class__(rsrc.path)
                    if self.RECURSIVE:
                        if isinstance(rsrc, moduleresource) or isinstance(rsrc, scriptresource):
                            rsrc = rsrc.asSource()
                            fnm = os.path.basename(rsrc.path)
                        rsrc.name = os.path.join(prefix, fnm)
                        if rsrc.typ == 'd':
                            rsrc.RECURSIVE = 1
                            self._contents.extend(rsrc.contents(rsrc.name))
                        else:
                            self._contents.append(rsrc)
                    else:
                        self._contents.append(rsrc)
            except ValueError, e:
                raise RuntimeError, "Can't make resource from %s\n ValueError: %s" \
                      % (os.path.join(self.path, fnm), `e.args`)
        return self._contents
    def asFilter(self):
        return tocfilter.DirFilter([self.path])

class treeresource(dirresource):
    """A subclass for a directory and subdirectories."""
    RECURSIVE = 1
    def __init__(self, name, fullname=None):
        dirresource.__init__(self, name, fullname)

class pkgresource(pythonresource):
    """A Python package.

        Note that contents() can be fooled by fancy __path__ statements. """
    def __init__(self, nm, fullname):
        resource.__init__(self, nm, fullname, 'p')
        self._contents = None
        self._depends = None
    def contents(self, parent=None):
        """Return a list of subpackages and modules in self."""
        if self._contents is not None:
            return self._contents
        if parent is None:
            parent = self.name
        self._contents = []
        cheat = treeresource(self.path)
        for rsrc in cheat.contents():
            if os.path.splitext(rsrc.path)[1] == '.py':
                rsrc = moduleresource(string.replace(rsrc.name[:-3], os.sep, '.'),
                                      rsrc.path)
                if rsrc.name[-8:] == '__init__':
                    rsrc.name = rsrc.name[:-9]
            elif os.path.isdir(rsrc.path):
                rsrc = makeresource(rsrc.path)
            else:
                continue
            if rsrc.name:
                rsrc.name = parent + '.' + rsrc.name
            else:
                rsrc.name = parent
            if rsrc.typ == 'm':
                self._contents.append(rsrc)
            elif rsrc.typ == 'p':
                self._contents.extend(rsrc.contents(rsrc.name))
        return self._contents
    def dependencies(self):
        """Return the list of accumulated dependencies of all modules in self."""
        if self._depends is not None:
            return self._depends
        self._depends = []
        tmp = {}
        for rsrc in self.contents():
            for r in rsrc.dependencies():
                tmp[r.name] = r
        self._depends = tmp.values()
        return self._depends
    def asFilter(self):
        """Create a PkgFilter from self."""
        return tocfilter.PkgFilter([os.path.dirname(self.path)])







if __name__ == '__main__':
    s = scriptresource('finder.py', './finder.py')
    print "s.modules:", s.modules
    print "s.binaries:", s.binaries

