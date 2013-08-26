#!/bin/true
import os, sys, imp

panda3d_modules = {
    "core"        :("libpandaexpress", "libpanda"),
    "dtoolconfig" : "libp3dtoolconfig",
    "physics"     : "libpandaphysics",
    "fx"          : "libpandafx",
    "direct"      : "libp3direct",
    "egg"         : "libpandaegg",
    "ode"         : "libpandaode",
    "bullet"      : "libpandabullet",
    "vision"      : "libp3vision",
    "physx"       : "libpandaphysx",
    "ai"          : "libpandaai",
    "awesomium"   : "libp3awesomium",
    "speedtree"   : "libpandaspeedtree",
    "rocket"      :("_rocketcore", "_rocketcontrols", "libp3rocket"),
    "vrpn"        : "libp3vrpn",
}

class panda3d_import_manager:
    # Important: store a reference to the sys and os modules, as
    # all references in the global namespace will be reset.
    os = os
    sys = sys
    imp = imp

    __libraries__ = {}

    # Figure out the dll suffix (commonly, _d for windows debug builds),
    # and the dll extension.
    dll_suffix = ''
    dll_exts = ('.pyd', '.so')
    if sys.platform == "win32":
        dll_exts = ('.pyd', '.dll')

        # We allow the caller to preload dll_suffix into the sys module.
        dll_suffix = getattr(sys, 'dll_suffix', None)

        if dll_suffix is None:
            # Otherwise, we try to determine it from the executable name:
            # python_d.exe implies _d across the board.
            dll_suffix = ''
            if sys.executable.endswith('_d.exe'):
                dll_suffix = '_d'

    # On OSX, extension modules can be loaded from either .so or .dylib.
    if sys.platform == "darwin":
        dll_exts = ('.pyd', '.so', '.dylib')

    prepared = False

    @classmethod
    def __prepare(cls):
        # This method only needs to be called once.
        if cls.prepared:
            return
        cls.prepared = True

        # First, we must ensure that the library path is
        # modified to locate all of the dynamic libraries.
        target = None
        filename = "libpandaexpress" + cls.dll_suffix
        for dir in cls.sys.path + [cls.sys.prefix]:
            lib = cls.os.path.join(dir, filename)
            for dll_ext in cls.dll_exts:
                if (cls.os.path.exists(lib + dll_ext)):
                    target = dir
                    break
        if target == None:
            raise ImportError("Cannot find %s" % (filename))
        target = cls.os.path.abspath(target)

        # And add that directory to the system library path.
        if cls.sys.platform == "win32":
            cls.__prepend_to_path("PATH", target)
        else:
            cls.__prepend_to_path("LD_LIBRARY_PATH", target)

        if cls.sys.platform == "darwin":
            cls.__prepend_to_path("DYLD_LIBRARY_PATH", target)

    @classmethod
    def __prepend_to_path(cls, varname, target):
        """ Prepends the given directory to the
        specified search path environment variable. """

        # Get the current value
        if varname in cls.os.environ:
            path = cls.os.environ[varname].strip(cls.os.pathsep)
        else:
            path = ""

        # Prepend our value, if it's not already the first thing
        if len(path) == 0:
            cls.os.environ[varname] = target
        elif not path.startswith(target):
            cls.os.environ[varname] = target + cls.os.pathsep + path

    @classmethod
    def libimport(cls, name):
        """ Imports and returns the specified library name. The
        provided library name has to be without dll extension. """

        if name in cls.__libraries__:
            return cls.__libraries__[name]

        if not cls.prepared: cls.__prepare()

        # Try to import it normally first.
        try:
            return __import__(name)
        except ImportError:
            _, err, _ = cls.sys.exc_info()
            if str(err) != "No module named " + name and \
               str(err) != "No module named '%s'" % name:
                raise

        # Hm, importing normally didn't work. Let's try imp.load_dynamic.
        # But first, locate the desired library.
        target = None
        filename = name + cls.dll_suffix
        for dir in cls.sys.path + [cls.sys.prefix]:
            lib = cls.os.path.join(dir, filename)
            for dll_ext in cls.dll_exts:
                if (cls.os.path.exists(lib + dll_ext)):
                    target = lib + dll_ext
                    break
            if target:
                # Once we find the first match, break all the way
                # out--don't keep looking for a second match.
                break
        if target == None:
            message = "DLL loader cannot find %s." % name
            raise ImportError(message)
        target = cls.os.path.abspath(target)

        # Now import the file explicitly.
        lib = cls.imp.load_dynamic(name, target)
        cls.__libraries__[name] = lib
        return lib

class panda3d_submodule(type(sys)):
    """ Represents a submodule of 'panda3d' that represents a dynamic
    library. This dynamic library is loaded when something is accessed
    from the module. """

    __manager__ = panda3d_import_manager

    def __init__(self, name, library):
        type(sys).__init__(self, "panda3d." + name)
        self.__library__ = library
        self.__libraries__ = [self.__library__]

    def __load__(self):
        """ Forces the library to be loaded right now. """
        self.__manager__.libimport(self.__library__)

    def __getattr__(self, name):
        mod = self.__manager__.libimport(self.__library__)
        if name == "__all__":
            everything = []
            for obj in mod.__dict__.keys():
                if not obj.startswith("__"):
                    everything.append(obj)
            self.__all__ = everything
            return everything
        elif name == "__library__":
            return self.__library__
        elif name == "__libraries__":
            return self.__libraries__
        elif name in mod.__dict__.keys():
            value = mod.__dict__[name]
            setattr(self, name, value)
            return value

        # Not found? Raise the error that Python would normally raise.
        raise AttributeError("'module' object has no attribute '%s'" % name)

class panda3d_multisubmodule(type(sys)):
    """ Represents a submodule of 'panda3d' that represents multiple
    dynamic libraries. These are loaded when something is accessed
    from the module. """

    __manager__ = panda3d_import_manager

    def __init__(self, name, libraries):
        type(sys).__init__(self, "panda3d." + name)
        self.__libraries__ = libraries

    def __load__(self):
        """ Forces the libraries to be loaded right now. """
        err = []
        for lib in self.__libraries__:
            try:
                self.__manager__.libimport(lib)
            except ImportError:
                _, msg, _ = self.__manager__.sys.exc_info()
                err.append(str(msg).rstrip('.'))
        if len(err) > 0:
            raise ImportError(', '.join(err))

    def __getattr__(self, name):
        if name == "__all__":
            everything = []
            for lib in self.__libraries__:
                for obj in self.__manager__.libimport(lib).__dict__:
                    if not obj.startswith("__"):
                        everything.append(obj)
            self.__all__ = everything
            return everything
        elif name == "__libraries__":
            return self.__libraries__

        for lib in self.__libraries__:
            mod = self.__manager__.libimport(lib)
            if name in mod.__dict__:
                value = mod.__dict__[name]
                setattr(self, name, value)
                return value

        # Not found? Raise the error that Python would normally raise.
        raise AttributeError("'module' object has no attribute '%s'" % name)

class panda3d_module(type(sys)):
    """ Represents the main 'panda3d' module. """

    __file__ = __file__
    modules = panda3d_modules
    __manager__ = panda3d_import_manager

    def __load__(self):
        """ Force all the libraries to be loaded right now. """
        err = []
        for module in self.modules:
            try:
                self.__manager__.sys.modules["panda3d.%s" % module].__load__()
            except ImportError:
                _, msg, _ = self.__manager__.sys.exc_info()
                err.append(str(msg).rstrip('.'))
        if len(err) > 0:
            raise ImportError(', '.join(err))


    def __getattr__(self, name):
        if name == "__all__":
            self.__all__ = name
            return self.modules.keys()
        elif name == "__file__":
            return self.__file__
        elif name in self.modules:
            value = self.__manager__.sys.modules["panda3d.%s" % name]
            setattr(self, name, value)
            return value

        # Not found? Raise the error that Python would normally raise.
        raise AttributeError("'module' object has no attribute '%s'" % name)

# Create the fake module objects and insert them into sys.modules.
this = panda3d_module("panda3d")

# Loop through the module dictionary, create a fake
# module for each of them, and insert them into
# sys.modules and into the 'panda3d' fake module.
for mod, lib in panda3d_modules.items():
    if isinstance(lib, tuple):
        module = panda3d_multisubmodule(mod, lib)
    else:
        module = panda3d_submodule(mod, lib)
    sys.modules["panda3d." + mod] = module
    this.__dict__[mod] = module

# Important: this must be the last thing in this file
sys.modules["panda3d"] = this
