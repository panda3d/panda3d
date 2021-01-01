""" This module extends standard Python's pickle module so that it is
capable of writing more efficient pickle files that contain Panda
objects with shared pointers.  In particular, a single Python
structure that contains many NodePaths into the same scene graph will
write the NodePaths correctly when used with this pickle module, so
that when it is unpickled later, the NodePaths will still reference
into the same scene graph together.

If you use the standard pickle module instead, the NodePaths will each
duplicate its own copy of its scene graph.

This is necessary because the standard pickle module doesn't provide a
mechanism for sharing context between different objects written to the
same pickle stream, so each NodePath has to write itself without
knowing about the other NodePaths that will also be writing to the
same stream.  This replacement module solves this problem by defining
a ``__reduce_persist__()`` replacement method for ``__reduce__()``,
which accepts a pointer to the Pickler object itself, allowing for
shared context between all objects written by that Pickler.

Unfortunately, cPickle cannot be supported, because it does not
support extensions of this nature. """

__all__ = ["PickleError", "PicklingError", "UnpicklingError", "Pickler",
           "Unpickler", "dump", "dumps", "load", "loads", "HIGHEST_PROTOCOL"]

import sys
from panda3d.core import BamWriter, BamReader, TypedObject

if sys.version_info >= (3, 0):
    from copyreg import dispatch_table
else:
    from copy_reg import dispatch_table

# A funny replacement for "import pickle" so we don't get confused
# with the local pickle.py.
pickle = __import__('pickle')

HIGHEST_PROTOCOL = pickle.HIGHEST_PROTOCOL

PickleError = pickle.PickleError
PicklingError = pickle.PicklingError
UnpicklingError = pickle.UnpicklingError

if sys.version_info >= (3, 0):
    DEFAULT_PROTOCOL = pickle.DEFAULT_PROTOCOL
    __all__.append("DEFAULT_PROTOCOL")

    BasePickler = pickle._Pickler
    BaseUnpickler = pickle._Unpickler
else:
    BasePickler = pickle.Pickler
    BaseUnpickler = pickle.Unpickler


class Pickler(BasePickler):

    def __init__(self, *args, **kw):
        self.bamWriter = BamWriter()
        self._canonical = {}
        BasePickler.__init__(self, *args, **kw)

    def clear_memo(self):
        BasePickler.clear_memo(self)
        self._canonical.clear()
        self.bamWriter = BamWriter()

    # We have to duplicate most of the save() method, so we can add
    # support for __reduce_persist__().

    def save(self, obj, save_persistent_id=True):
        if self.proto >= 4:
            self.framer.commit_frame()

        # Check for persistent id (defined by a subclass)
        pid = self.persistent_id(obj)
        if pid is not None and save_persistent_id:
            self.save_pers(pid)
            return

        # Check if this is a Panda type that we've already saved; if so, store
        # a mapping to the canonical copy, so that Python's memoization system
        # works properly.  This is needed because Python uses id(obj) for
        # memoization, but there may be multiple Python wrappers for the same
        # C++ pointer, and we don't want that to result in duplication.
        t = type(obj)
        if issubclass(t, TypedObject.__base__):
            canonical = self._canonical.get(obj.this)
            if canonical is not None:
                obj = canonical
            else:
                # First time we're seeing this C++ pointer; save it as the
                # "canonical" version.
                self._canonical[obj.this] = obj

        # Check the memo
        x = self.memo.get(id(obj))
        if x:
            self.write(self.get(x[0]))
            return

        # Check the type dispatch table
        f = self.dispatch.get(t)
        if f:
            f(self, obj) # Call unbound method with explicit self
            return

        # Check for a class with a custom metaclass; treat as regular class
        try:
            issc = issubclass(t, type)
        except TypeError: # t is not a class (old Boost; see SF #502085)
            issc = 0
        if issc:
            self.save_global(obj)
            return

        # Check copy_reg.dispatch_table
        reduce = dispatch_table.get(t)
        if reduce:
            rv = reduce(obj)
        else:
            # New code: check for a __reduce_persist__ method, then
            # fall back to standard methods.
            reduce = getattr(obj, "__reduce_persist__", None)
            if reduce:
                rv = reduce(self)
            else:
                # Check for a __reduce_ex__ method, fall back to __reduce__
                reduce = getattr(obj, "__reduce_ex__", None)
                if reduce:
                    rv = reduce(self.proto)
                else:
                    reduce = getattr(obj, "__reduce__", None)
                    if reduce:
                        rv = reduce()
                    else:
                        raise PicklingError("Can't pickle %r object: %r" %
                                            (t.__name__, obj))

        # Check for string returned by reduce(), meaning "save as global"
        if type(rv) is str:
            self.save_global(obj, rv)
            return

        # Assert that reduce() returned a tuple
        if type(rv) is not tuple:
            raise PicklingError("%s must return string or tuple" % reduce)

        # Assert that it returned an appropriately sized tuple
        l = len(rv)
        if not (2 <= l <= 5):
            raise PicklingError("Tuple returned by %s must have "
                                "two to five elements" % reduce)

        # Save the reduce() output and finally memoize the object
        self.save_reduce(obj=obj, *rv)


class Unpickler(BaseUnpickler):

    def __init__(self, *args, **kw):
        self.bamReader = BamReader()
        BaseUnpickler.__init__(self, *args, **kw)

    # Duplicate the load_reduce() function, to provide a special case
    # for the reduction function.

    def load_reduce(self):
        stack = self.stack
        args = stack.pop()
        func = stack[-1]

        # If the function name ends with "_persist", then assume the
        # function wants the Unpickler as the first parameter.
        func_name = func.__name__
        if func_name.endswith('_persist') or func_name.endswith('Persist'):
            value = func(self, *args)
        else:
            # Otherwise, use the existing pickle convention.
            value = func(*args)

        stack[-1] = value

    if sys.version_info >= (3, 0):
        BaseUnpickler.dispatch[pickle.REDUCE[0]] = load_reduce
    else:
        BaseUnpickler.dispatch[pickle.REDUCE] = load_reduce


# Shorthands
from io import BytesIO

def dump(obj, file, protocol=None):
    Pickler(file, protocol).dump(obj)

def dumps(obj, protocol=None):
    file = BytesIO()
    Pickler(file, protocol).dump(obj)
    return file.getvalue()

def load(file):
    return Unpickler(file).load()

def loads(str):
    file = BytesIO(str)
    return Unpickler(file).load()
