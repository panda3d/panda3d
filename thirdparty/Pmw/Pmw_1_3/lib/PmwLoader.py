# This module is used by the Pmw package system.
# The PmwLoader class can be used to simulate a python module,
# but also supports importing of submodules on demand.  This technique
# reduces startup time because Pmw submodules which are not used are
# not loaded.
#
# The PmwLoader class also supports runtime selection of the Pmw
# version(s) to use.

import sys
import os
import string
import types

_PMW_DEF = 'Pmw.def'           # Pmw definition file
_BASEMODULE = 'Base'           # Name of Base module

class PmwLoader:

    def __init__(self, dirpath, instdirs, dirs):
	self._dirpath = dirpath
	self._instdirs = instdirs
	self._dirs = dirs
	self._initialised = 0
	self._version = string.replace(instdirs[0][4:], '_', '.')
	self._alpha_versions = ()
	
    #======================================================================

    # Public methods.  These methods will be seen as "module methods".

    def setversion(self, version):
	if self._version == version:
	    return
	if self._initialised:
	    raise ValueError, 'Cannot change Pmw version after initialisation'
	self._version = version

    def setalphaversions(self, *alpha_versions):
	if self._alpha_versions == alpha_versions:
	    return
	if self._initialised:
	    raise ValueError, \
		    'Cannot change Pmw alpha versions after initialisation'
	self._alpha_versions = alpha_versions

    def version(self, alpha = 0):
	if alpha:
	    return self._alpha_versions
	else:
	    return self._version

    def installedversions(self, alpha = 0):
	rtn = []
	if alpha:
	    dirs = filter(lambda x: x[:5] == 'Alpha', self._dirs)
	    dirs.sort()
	    dirs.reverse()
	    for dir in dirs:
		rtn.append(string.replace(dir[6:], '_', '.'))
	else:
	    for dir in self._instdirs:
		rtn.append(string.replace(dir[4:], '_', '.'))
	return rtn

    #======================================================================

    # Private methods

    def _getmodule(self,modpath):
	__import__(modpath)
	mod = sys.modules[modpath]
	return mod

    def _initialise(self):
	searchpath = []

	for version in self._alpha_versions:
	    alphadir = '_Pmw.Alpha_%s.lib' % string.replace(version, '.', '_')
	    searchpath.append(alphadir)

	libdir = '_Pmw.Pmw_%s.lib' % string.replace(self._version, '.', '_')
	searchpath.append(libdir)

	# Create attributes for the PmwBase classes and functions.
	for path in searchpath:
	    try:
		basemodule = self._getmodule(path + '.Pmw' + _BASEMODULE)
		break
	    except ImportError, msg:
		if path == searchpath[-1]:
		    # No PmwBase module found.
		    raise ImportError, msg

	for k,v in basemodule.__dict__.items():
	    if k[0] is not '_' and type(v) != types.ModuleType:
		self.__dict__[k] = v

	# Set the Pmw definitions from the Pmw.def file.
	dict = {
	    '_widgets'      : {},
	    '_extraWidgets' : {},
	    '_functions'    : {},
	    '_modules'      : {},
	}
	for name in dict.keys():
	    self.__dict__[name] = {}
	searchpath.reverse()
	for path in searchpath:
	    pathbit = apply(os.path.join, tuple(string.split(path[5:], '.')))
	    lpath = os.path.join(self._dirpath, pathbit)
	    d = {}
	    execfile(os.path.join(lpath,_PMW_DEF), d)
	    for k,v in d.items():
		if dict.has_key(k):
		    if type(v) == types.TupleType:
			for item in v:
			    modpath = path + '.Pmw' + item
			    dict[k][item] = modpath
		    elif type(v) == types.DictionaryType:
			for k1, v1 in v.items():
			    modpath = path + '.Pmw' + v1
			    dict[k][k1] = modpath
	self.__dict__.update(dict)
	self._widgets_keys = self._widgets.keys()
	self._extraWidgets_keys = self._extraWidgets.keys()
	self._functions_keys = self._functions.keys()
	self._modules_keys = self._modules.keys()

	self._initialised = 1

    def __getattr__(self, name):
	if not self._initialised:
	    self._initialise()
	    # Beware: _initialise may have defined 'name'
	    if name in self.__dict__.keys():
		return self.__dict__[name]

	# The requested attribute is not yet set. Look it up in the
	# tables set by Pmw.def, import the appropriate module and
	# set the attribute so that it will be found next time.

	if name in self._widgets_keys:
	    # The attribute is a widget name.
	    mod = self._getmodule(self._widgets[name])
	    cls = getattr(mod,name)
	    self.__dict__[name] = cls
	    return cls

	if name in self._functions_keys:
	    # The attribute is a function from one of the modules.
	    modname = self._functions[name]
	    mod  = self._getmodule(modname)
	    func = getattr(mod, name)
	    self.__dict__[name] = func
	    return func

	if name in self._modules_keys:
	    # The attribute is a module
	    mod = self._getmodule(self._modules[name])
	    self.__dict__[name] = mod
	    return mod

	if name in self._extraWidgets_keys:
	    # XXX I should import them all, once I've started.
            # The attribute is a widget name in a module of another name
	    modname = self._extraWidgets[name]
	    mod = self._getmodule(modname)
	    cls = getattr(mod, name)
            self.__dict__[name] = cls
            return cls

	# The attribute is not known by Pmw, report an error.
	raise AttributeError, name
