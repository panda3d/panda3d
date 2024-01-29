# Python interface to some of the commands of the 2.4 version of the
# BLT extension to tcl.

import string
import types
import tkinter

# Supported commands:
_busyCommand = '::blt::busy'
_vectorCommand = '::blt::vector'
_graphCommand = '::blt::graph'
_testCommand = '::blt::*'
_chartCommand = '::blt::stripchart'
_tabsetCommand = '::blt::tabset'

_haveBlt = None
_haveBltBusy = None

_forceBltDisable = True

def setBltDisable(window, value):
    global _forceBltDisable
    _forceBltDisable = value
    _checkForBlt(window)

def _checkForBlt(window):
    global _haveBlt
    global _haveBltBusy
    global _forceBltDisable

    # Blt may be a package which has not yet been loaded. Try to load it.
    try:
        window.tk.call('package', 'require', 'BLT')
    except tkinter.TclError:
        # Another way to try to dynamically load blt:
        try:
            window.tk.call('load', '', 'Blt')
        except tkinter.TclError:
            pass

    _haveBlt= (window.tk.call('info', 'commands', _testCommand) != '')
    _haveBltBusy = (window.tk.call('info', 'commands', _busyCommand) != '')

    #only force haveBlt, not Busy since Busy might work...
    if (_forceBltDisable):
        _haveBlt = False

def haveblt(window):
    if _haveBlt is None:
        _checkForBlt(window)
    return _haveBlt

def havebltbusy(window):
    if _haveBlt is None:
        _checkForBlt(window)
    return _haveBltBusy

def _loadBlt(window):
    if _haveBlt is None:
        if window is None:
            window = tkinter._default_root
            if window is None:
                window = tkinter.Tk()
        _checkForBlt(window)

def busy_hold(window, cursor = None):
    _loadBlt(window)
    if cursor is None:
        window.tk.call(_busyCommand, 'hold', window._w)
    else:
        window.tk.call(_busyCommand, 'hold', window._w, '-cursor', cursor)

def busy_release(window):
    _loadBlt(window)
    window.tk.call(_busyCommand, 'release', window._w)

def busy_forget(window):
    _loadBlt(window)
    window.tk.call(_busyCommand, 'forget', window._w)

#=============================================================================
# Interface to the blt vector command which makes it look like the
# builtin python list type.
# The -variable, -command, -watchunset creation options are not supported.
# The dup, merge, notify, offset, populate, seq and variable methods
# and the +, -, * and / operations are not supported.

# Blt vector functions:
def vector_expr(expression):
    tk = tkinter._default_root.tk
    strList = tk.splitlist(tk.call(_vectorCommand, 'expr', expression))
    return tuple(map(float, strList))

def vector_names(pattern = None):
    tk = tkinter._default_root.tk
    return tk.splitlist(tk.call(_vectorCommand, 'names', pattern))

class Vector:
    _varnum = 0
    def __init__(self, size=None, master=None):
        # <size> can be either an integer size, or a string "first:last".
        _loadBlt(master)
        if master:
            self._master = master
        else:
            self._master = tkinter._default_root
        self.tk = self._master.tk
        self._name = 'PY_VEC' + str(Vector._varnum)
        Vector._varnum = Vector._varnum + 1
        if size is None:
            self.tk.call(_vectorCommand, 'create', self._name)
        else:
            self.tk.call(_vectorCommand, 'create', '%s(%s)' % (self._name, size))
    def __del__(self):
        self.tk.call(_vectorCommand, 'destroy', self._name)
    def __str__(self):
        return self._name

    def __repr__(self):
        #return '[' + string.join(list(map(str, self)), ', ') + ']'
        return '[' + ', '.join(list(map(str, self))) + ']'
    def __cmp__(self, list):
        return cmp(self[:], list)

    def __len__(self):
        return self.tk.getint(self.tk.call(self._name, 'length'))
    def __getitem__(self, key):
        if isinstance(key, slice):
            return self.__getslice__(key.start, key.stop)
        oldkey = key
        if key < 0:
            key = key + len(self)
        try:
            return self.tk.getdouble(self.tk.globalgetvar(self._name, str(key)))
        except tkinter.TclError:
            raise IndexError(oldkey)
    def __setitem__(self, key, value):
        if key < 0:
            key = key + len(self)
        return self.tk.globalsetvar(self._name, str(key), float(value))

    def __delitem__(self, key):
        if key < 0:
            key = key + len(self)
        return self.tk.globalunsetvar(self._name, str(key))

    def __getslice__(self, start, end):
        length = len(self)
        if start is None or start < 0:
            start = 0
        if end is None or end > length:
            end = length
        if start >= end:
            return []
        end = end - 1  # Blt vector slices include end point.
        text = self.tk.globalgetvar(self._name, str(start) + ':' + str(end))
        return list(map(self.tk.getdouble, self.tk.splitlist(text)))

    def __setslice__(self, start, end, list):
        if start > end:
            end = start
        self.set(self[:start] + list + self[end:])

    def __delslice__(self, start, end):
        if start < end:
            self.set(self[:start] + self[end:])

    def __add__(self, list):
        return self[:] + list
    def __radd__(self, list):
        return list + self[:]
    def __mul__(self, n):
        return self[:] * n
    __rmul__ = __mul__

    # Python builtin list methods:
    def append(self, *args):
        self.tk.call(self._name, 'append', args)
    def count(self, obj):
        return self[:].count(obj)
    def index(self, value):
        return self[:].index(value)
    def insert(self, index, value):
        self[index:index] = [value]
    def remove(self, value):
        del self[self.index(value)]
    def reverse(self):
        s = self[:]
        s.reverse()
        self.set(s)
    def sort(self, *args):
        s = self[:]
        s.sort()
        self.set(s)

    # Blt vector instance methods:
    # append - same as list method above
    def clear(self):
        self.tk.call(self._name, 'clear')
    def delete(self, *args):
        self.tk.call((self._name, 'delete') + args)
    def expr(self, expression):
        self.tk.call(self._name, 'expr', expression)
    def length(self, newSize=None):
        return self.tk.getint(self.tk.call(self._name, 'length', newSize))
    def range(self, first, last=None):
        # Note that, unlike self[first:last], this includes the last
        # item in the returned range.
        text = self.tk.call(self._name, 'range', first, last)
        return list(map(self.tk.getdouble, self.tk.splitlist(text)))
    def search(self, start, end=None):
        return self._master._getints(self.tk.call(
                self._name, 'search', start, end))
    def set(self, list):
        if type(list) != tuple:
            list = tuple(list)
        self.tk.call(self._name, 'set', list)

    # The blt vector sort method has different semantics to the python
    # list sort method.  Call these blt_sort:
    def blt_sort(self, *args):
        self.tk.call((self._name, 'sort') + args)
    def blt_sort_reverse(self, *args):
        self.tk.call((self._name, 'sort', '-reverse') + args)

    # Special blt vector indexes:
    def min(self):
        return self.tk.getdouble(self.tk.globalgetvar(self._name, 'min'))
    def max(self):
        return self.tk.getdouble(self.tk.globalgetvar(self._name, 'max'))

    # Method borrowed from Tkinter.Var class:
    def get(self):
        return self[:]

#=============================================================================

# This is a general purpose configure routine which can handle the
# configuration of widgets, items within widgets, etc.  Supports the
# forms configure() and configure('font') for querying and
# configure(font = 'fixed', text = 'hello') for setting.

def _doConfigure(widget, subcommand, option, kw):

    if not option and not kw:
        # Return a description of all options.
        ret = {}
        options = widget.tk.splitlist(widget.tk.call(subcommand))
        for optionString in options:
            optionInfo = widget.tk.splitlist(optionString)
            option = optionInfo[0][1:]
            ret[option] = (option,) + optionInfo[1:]
        return ret

    if option:
        # Return a description of the option given by <option>.
        if kw:
            # Having keywords implies setting configuration options.
            # Can't set and get in one command!
            raise ValueError('cannot have option argument with keywords')
        option = '-' + option
        optionInfo = widget.tk.splitlist(widget.tk.call(subcommand + (option,)))
        return (optionInfo[0][1:],) + optionInfo[1:]

    # Otherwise, set the given configuration options.
    widget.tk.call(subcommand + widget._options(kw))

#=============================================================================

class Graph(tkinter.Widget):
    # Wrapper for the blt graph widget, version 2.4.

    def __init__(self, master=None, cnf={}, **kw):
        _loadBlt(master)
        tkinter.Widget.__init__(self, master, _graphCommand, cnf, kw)

    def bar_create(self, name, **kw):
        self.tk.call((self._w, 'bar', 'create', name) + self._options(kw))

    def line_create(self, name, **kw):
        self.tk.call((self._w, 'line', 'create', name) + self._options(kw))

    def extents(self, item):
        return self.tk.getint(self.tk.call(self._w, 'extents', item))

    def invtransform(self, winX, winY):
        return self._getdoubles(
                self.tk.call(self._w, 'invtransform', winX, winY))

    def inside(self, x, y):
        return self.tk.getint(self.tk.call(self._w, 'inside', x, y))

    def snap(self, photoName):
        self.tk.call(self._w, 'snap', photoName)

    def transform(self, x, y):
        return self._getdoubles(self.tk.call(self._w, 'transform', x, y))

    def axis_cget(self, axisName, key):
        return self.tk.call(self._w, 'axis', 'cget', axisName, '-' + key)
    def axis_configure(self, axes, option=None, **kw):
        # <axes> may be a list of axisNames.
        if type(axes) is str:
            axes = [axes]
        subcommand = (self._w, 'axis', 'configure') + tuple(axes)
        return _doConfigure(self, subcommand, option, kw)
    def axis_create(self, axisName, **kw):
        self.tk.call((self._w, 'axis', 'create', axisName) + self._options(kw))
    def axis_delete(self, *args):
        self.tk.call((self._w, 'axis', 'delete') + args)
    def axis_invtransform(self, axisName, value):
        return self.tk.getdouble(self.tk.call(
                self._w, 'axis', 'invtransform', axisName, value))
    def axis_limits(self, axisName):
        return self._getdoubles(self.tk.call(
                self._w, 'axis', 'limits', axisName))
    def axis_names(self, *args):
        return self.tk.splitlist(
                self.tk.call((self._w, 'axis', 'names') + args))
    def axis_transform(self, axisName, value):
        return self.tk.getint(self.tk.call(
                self._w, 'axis', 'transform', axisName, value))

    def xaxis_cget(self, key):
        return self.tk.call(self._w, 'xaxis', 'cget', '-' + key)
    def xaxis_configure(self, option=None, **kw):
        subcommand = (self._w, 'xaxis', 'configure')
        return _doConfigure(self, subcommand, option, kw)
    def xaxis_invtransform(self, value):
        return self.tk.getdouble(self.tk.call(
                self._w, 'xaxis', 'invtransform', value))
    def xaxis_limits(self):
        return self._getdoubles(self.tk.call(self._w, 'xaxis', 'limits'))
    def xaxis_transform(self, value):
        return self.tk.getint(self.tk.call(
                self._w, 'xaxis', 'transform', value))
    def xaxis_use(self, axisName = None):
        return self.tk.call(self._w, 'xaxis', 'use', axisName)

    def x2axis_cget(self, key):
        return self.tk.call(self._w, 'x2axis', 'cget', '-' + key)
    def x2axis_configure(self, option=None, **kw):
        subcommand = (self._w, 'x2axis', 'configure')
        return _doConfigure(self, subcommand, option, kw)
    def x2axis_invtransform(self, value):
        return self.tk.getdouble(self.tk.call(
                self._w, 'x2axis', 'invtransform', value))
    def x2axis_limits(self):
        return self._getdoubles(self.tk.call(self._w, 'x2axis', 'limits'))
    def x2axis_transform(self, value):
        return self.tk.getint(self.tk.call(
                self._w, 'x2axis', 'transform', value))
    def x2axis_use(self, axisName = None):
        return self.tk.call(self._w, 'x2axis', 'use', axisName)

    def yaxis_cget(self, key):
        return self.tk.call(self._w, 'yaxis', 'cget', '-' + key)
    def yaxis_configure(self, option=None, **kw):
        subcommand = (self._w, 'yaxis', 'configure')
        return _doConfigure(self, subcommand, option, kw)
    def yaxis_invtransform(self, value):
        return self.tk.getdouble(self.tk.call(
                self._w, 'yaxis', 'invtransform', value))
    def yaxis_limits(self):
        return self._getdoubles(self.tk.call(self._w, 'yaxis', 'limits'))
    def yaxis_transform(self, value):
        return self.tk.getint(self.tk.call(
                self._w, 'yaxis', 'transform', value))
    def yaxis_use(self, axisName = None):
        return self.tk.call(self._w, 'yaxis', 'use', axisName)

    def y2axis_cget(self, key):
        return self.tk.call(self._w, 'y2axis', 'cget', '-' + key)
    def y2axis_configure(self, option=None, **kw):
        subcommand = (self._w, 'y2axis', 'configure')
        return _doConfigure(self, subcommand, option, kw)
    def y2axis_invtransform(self, value):
        return self.tk.getdouble(self.tk.call(
                self._w, 'y2axis', 'invtransform', value))
    def y2axis_limits(self):
        return self._getdoubles(self.tk.call(self._w, 'y2axis', 'limits'))
    def y2axis_transform(self, value):
        return self.tk.getint(self.tk.call(
                self._w, 'y2axis', 'transform', value))
    def y2axis_use(self, axisName = None):
        return self.tk.call(self._w, 'y2axis', 'use', axisName)

    def crosshairs_cget(self, key):
        return self.tk.call(self._w, 'crosshairs', 'cget', '-' + key)
    def crosshairs_configure(self, option=None, **kw):
        subcommand = (self._w, 'crosshairs', 'configure')
        return _doConfigure(self, subcommand, option, kw)
    def crosshairs_off(self):
        self.tk.call(self._w, 'crosshairs', 'off')
    def crosshairs_on(self):
        self.tk.call(self._w, 'crosshairs', 'on')
    def crosshairs_toggle(self):
        self.tk.call(self._w, 'crosshairs', 'toggle')

    def element_activate(self, name, *args):
        self.tk.call((self._w, 'element', 'activate', name) + args)
    def element_bind(self, tagName, sequence=None, func=None, add=None):
        return self._bind((self._w, 'element', 'bind', tagName),
                sequence, func, add)
    def element_unbind(self, tagName, sequence, funcid=None):
        self.tk.call(self._w, 'element', 'bind', tagName, sequence, '')
        if funcid:
            self.deletecommand(funcid)

    def element_cget(self, name, key):
        return self.tk.call(self._w, 'element', 'cget', name, '-' + key)

    def element_closest(self, x, y, *args, **kw):
        var = 'python_private_1'
        success = self.tk.getint(self.tk.call(
                (self._w, 'element', 'closest', x, y, var) +
                        self._options(kw) + args))
        if success:
            rtn = {}
            rtn['dist'] = self.tk.getdouble(self.tk.globalgetvar(var, 'dist'))
            rtn['x'] = self.tk.getdouble(self.tk.globalgetvar(var, 'x'))
            rtn['y'] = self.tk.getdouble(self.tk.globalgetvar(var, 'y'))
            rtn['index'] = self.tk.getint(self.tk.globalgetvar(var, 'index'))
            rtn['name'] = self.tk.globalgetvar(var, 'name')
            return rtn
        else:
            return None

    def element_configure(self, names, option=None, **kw):
        # <names> may be a list of elemNames.
        if type(names) is str:
            names = [names]
        subcommand = (self._w, 'element', 'configure') + tuple(names)
        return _doConfigure(self, subcommand, option, kw)

    def element_deactivate(self, *args):
        self.tk.call((self._w, 'element', 'deactivate') + args)

    def element_delete(self, *args):
        self.tk.call((self._w, 'element', 'delete') + args)
    def element_exists(self, name):
        return self.tk.getboolean(
                self.tk.call(self._w, 'element', 'exists', name))

    def element_names(self, *args):
        return self.tk.splitlist(
                self.tk.call((self._w, 'element', 'names') + args))
    def element_show(self, nameList=None):
        if nameList is not None:
            nameList = tuple(nameList)
        return self.tk.splitlist(
                self.tk.call(self._w, 'element', 'show', nameList))
    def element_type(self, name):
        return self.tk.call(self._w, 'element', 'type', name)

    def grid_cget(self, key):
        return self.tk.call(self._w, 'grid', 'cget', '-' + key)
    def grid_configure(self, option=None, **kw):
        subcommand = (self._w, 'grid', 'configure')
        return _doConfigure(self, subcommand, option, kw)

    def grid_off(self):
        self.tk.call(self._w, 'grid', 'off')
    def grid_on(self):
        self.tk.call(self._w, 'grid', 'on')
    def grid_toggle(self):
        self.tk.call(self._w, 'grid', 'toggle')

    def legend_activate(self, *args):
        self.tk.call((self._w, 'legend', 'activate') + args)
    def legend_bind(self, tagName, sequence=None, func=None, add=None):
        return self._bind((self._w, 'legend', 'bind', tagName),
                sequence, func, add)
    def legend_unbind(self, tagName, sequence, funcid=None):
        self.tk.call(self._w, 'legend', 'bind', tagName, sequence, '')
        if funcid:
            self.deletecommand(funcid)

    def legend_cget(self, key):
        return self.tk.call(self._w, 'legend', 'cget', '-' + key)
    def legend_configure(self, option=None, **kw):
        subcommand = (self._w, 'legend', 'configure')
        return _doConfigure(self, subcommand, option, kw)
    def legend_deactivate(self, *args):
        self.tk.call((self._w, 'legend', 'deactivate') + args)
    def legend_get(self, pos):
        return self.tk.call(self._w, 'legend', 'get', pos)

    def pen_cget(self, name, key):
        return self.tk.call(self._w, 'pen', 'cget', name, '-' + key)
    def pen_configure(self, names, option=None, **kw):
        # <names> may be a list of penNames.
        if type(names) is str:
            names = [names]
        subcommand = (self._w, 'pen', 'configure') + tuple(names)
        return _doConfigure(self, subcommand, option, kw)
    def pen_create(self, name, **kw):
        self.tk.call((self._w, 'pen', 'create', name) + self._options(kw))
    def pen_delete(self, *args):
        self.tk.call((self._w, 'pen', 'delete') + args)
    def pen_names(self, *args):
        return self.tk.splitlist(self.tk.call((self._w, 'pen', 'names') + args))

    def postscript_cget(self, key):
        return self.tk.call(self._w, 'postscript', 'cget', '-' + key)
    def postscript_configure(self, option=None, **kw):
        subcommand = (self._w, 'postscript', 'configure')
        return _doConfigure(self, subcommand, option, kw)
    def postscript_output(self, fileName=None, **kw):
        prefix = (self._w, 'postscript', 'output')
        if fileName is None:
            return self.tk.call(prefix + self._options(kw))
        else:
            self.tk.call(prefix + (fileName,) + self._options(kw))

    def marker_after(self, first, second=None):
        self.tk.call(self._w, 'marker', 'after', first, second)
    def marker_before(self, first, second=None):
        self.tk.call(self._w, 'marker', 'before', first, second)
    def marker_bind(self, tagName, sequence=None, func=None, add=None):
        return self._bind((self._w, 'marker', 'bind', tagName),
                sequence, func, add)
    def marker_unbind(self, tagName, sequence, funcid=None):
        self.tk.call(self._w, 'marker', 'bind', tagName, sequence, '')
        if funcid:
            self.deletecommand(funcid)

    def marker_cget(self, name, key):
        return self.tk.call(self._w, 'marker', 'cget', name, '-' + key)
    def marker_configure(self, names, option=None, **kw):
        # <names> may be a list of markerIds.
        if type(names) is str:
            names = [names]
        subcommand = (self._w, 'marker', 'configure') + tuple(names)
        return _doConfigure(self, subcommand, option, kw)
    def marker_create(self, type, **kw):
        return self.tk.call(
                (self._w, 'marker', 'create', type) + self._options(kw))

    def marker_delete(self, *args):
        self.tk.call((self._w, 'marker', 'delete') + args)
    def marker_exists(self, name):
        return self.tk.getboolean(
                self.tk.call(self._w, 'marker', 'exists', name))
    def marker_names(self, *args):
        return self.tk.splitlist(
                self.tk.call((self._w, 'marker', 'names') + args))
    def marker_type(self, name):
        type = self.tk.call(self._w, 'marker', 'type', name)
        if type == '':
            type = None
        return type

#=============================================================================
class Stripchart(Graph):
    # Wrapper for the blt stripchart widget, version 2.4.

    def __init__(self, master=None, cnf={}, **kw):
        _loadBlt(master)
        tkinter.Widget.__init__(self, master, _chartCommand, cnf, kw)

#=============================================================================
class Tabset(tkinter.Widget):

    # Wrapper for the blt TabSet widget, version 2.4.

    def __init__(self, master=None, cnf={}, **kw):
        _loadBlt(master)
        tkinter.Widget.__init__(self, master, _tabsetCommand, cnf, kw)

    def activate(self, tabIndex):
        self.tk.call(self._w, 'activate', tabIndex)

    # This is the 'bind' sub-command:
    def tag_bind(self, tagName, sequence=None, func=None, add=None):
        return self._bind((self._w, 'bind', tagName), sequence, func, add)

    def tag_unbind(self, tagName, sequence, funcid=None):
        self.tk.call(self._w, 'bind', tagName, sequence, '')
        if funcid:
            self.deletecommand(funcid)

    def delete(self, first, last = None):
        self.tk.call(self._w, 'delete', first, last)

    # This is the 'focus' sub-command:
    def tab_focus(self, tabIndex):
        self.tk.call(self._w, 'focus', tabIndex)

    def get(self, tabIndex):
        return self.tk.call(self._w, 'get', tabIndex)

    def index(self, tabIndex):
        index = self.tk.call(self._w, 'index', tabIndex)
        if index == '':
            return None
        else:
            return self.tk.getint(self.tk.call(self._w, 'index', tabIndex))

    def insert(self, position, name1, *names, **kw):
        self.tk.call(
            (self._w, 'insert', position, name1) + names + self._options(kw))

    def invoke(self, tabIndex):
        return self.tk.call(self._w, 'invoke', tabIndex)

    def move(self, tabIndex1, beforeOrAfter, tabIndex2):
        self.tk.call(self._w, 'move', tabIndex1, beforeOrAfter, tabIndex2)

    def nearest(self, x, y):
        return self.tk.call(self._w, 'nearest', x, y)

    def scan_mark(self, x, y):
        self.tk.call(self._w, 'scan', 'mark', x, y)

    def scan_dragto(self, x, y):
        self.tk.call(self._w, 'scan', 'dragto', x, y)

    def see(self, index):
        self.tk.call(self._w, 'see', index)

    def see(self, tabIndex):
        self.tk.call(self._w,'see',tabIndex)

    def size(self):
        return self.tk.getint(self.tk.call(self._w, 'size'))

    def tab_cget(self, tabIndex, option):
        if option[:1] != '-':
            option = '-' + option
        if option[-1:] == '_':
            option = option[:-1]
        return self.tk.call(self._w, 'tab', 'cget', tabIndex, option)

    def tab_configure(self, tabIndexes, option=None, **kw):
        # <tabIndexes> may be a list of tabs.
        if type(tabIndexes) in (str, int):
            tabIndexes = [tabIndexes]
        subcommand = (self._w, 'tab', 'configure') + tuple(tabIndexes)
        return _doConfigure(self, subcommand, option, kw)

    def tab_names(self, *args):
        return self.tk.splitlist(self.tk.call((self._w, 'tab', 'names') + args))

    def tab_tearoff(self, tabIndex, newName = None):
        if newName is None:
            name = self.tk.call(self._w, 'tab', 'tearoff', tabIndex)
            return self.nametowidget(name)
        else:
            self.tk.call(self._w, 'tab', 'tearoff', tabIndex, newName)

    def view(self):
        s = self.tk.call(self._w, 'view')
        return tuple(map(self.tk.getint, self.tk.splitlist(s)))
    def view_moveto(self, fraction):
        self.tk.call(self._w, 'view', 'moveto', fraction)
    def view_scroll(self, number, what):
        self.tk.call(self._w, 'view', 'scroll', number, what)
