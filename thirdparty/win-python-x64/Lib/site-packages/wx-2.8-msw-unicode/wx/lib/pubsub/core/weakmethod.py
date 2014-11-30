'''
This module provides a basic "weak method" implementation. It is necessary
because the weakref module does not support weak methods (in the sense that,
counter-intuitively, a user who creates a weakref.ref(obj.method), a reasonable
action, get a weak ref that is None. 

:copyright: Copyright 2006-2009 by Oliver Schoenborn, all rights reserved.
:license: BSD, see LICENSE.txt for details.

'''

# for function and method parameter counting:
from inspect import ismethod
# for weakly bound methods:
from new     import instancemethod as InstanceMethod
from weakref import ref as WeakRef


class WeakMethod:
    """Represent a weak bound method, i.e. a method which doesn't keep alive the 
    object that it is bound to. It uses WeakRef which, used on its own, 
    produces weak methods that are dead on creation, not very useful. 
    Typically, you will use the getWeakRef() module function instead of using
    this class directly. """
    
    def __init__(self, method, notifyDead = None):
        """The method must be bound. notifyDead will be called when 
        object that method is bound to dies. """
        assert ismethod(method)
        if method.im_self is None:
            raise ValueError('Unbound methods cannot be weak-referenced.')
            
        self.notifyDead = None
        if notifyDead is None:
            self.objRef = WeakRef(method.im_self)
        else:
            self.notifyDead = notifyDead
            self.objRef = WeakRef(method.im_self, self.__onNotifyDeadObj)
            
        self.fun = method.im_func
        self.cls = method.im_class
        
    def __onNotifyDeadObj(self, ref):
        if self.notifyDead:
            try:
                self.notifyDead(self)
            except Exception:
                import traceback
                traceback.print_exc()

    def __call__(self):
        """Returns a new.instancemethod if object for method still alive. 
        Otherwise return None. Note that instancemethod causes a 
        strong reference to object to be created, so shouldn't save 
        the return value of this call. Note also that this __call__
        is required only for compatibility with WeakRef.ref(), otherwise
        there would be more efficient ways of providing this functionality."""
        if self.objRef() is None:
            return None
        else:
            return InstanceMethod(self.fun, self.objRef(), self.cls)
        
    def __eq__(self, method2):
        """Two WeakMethod objects compare equal if they refer to the same method
        of the same instance. Thanks to Josiah Carlson for patch and clarifications
        on how dict uses eq/cmp and hashing. """
        if not isinstance(method2, WeakMethod):
            return False 
            
        return (    self.fun      is method2.fun 
                and self.objRef() is method2.objRef() 
                and self.objRef() is not None )
    
    def __hash__(self):
        """Hash is an optimization for dict searches, it need not 
        return different numbers for every different object. Some objects
        are not hashable (eg objects of classes derived from dict) so no
        hash(objRef()) in there, and hash(self.cls) would only be useful
        in the rare case where instance method was rebound. """
        return hash(self.fun)
    
    def __repr__(self):
        dead = ''
        if self.objRef() is None: 
            dead = '; DEAD'
        obj = '<%s at %s%s>' % (self.__class__, id(self), dead)
        return obj
        
    def refs(self, weakRef):
        """Return true if we are storing same object referred to by weakRef."""
        return self.objRef == weakRef


def getWeakRef(obj, notifyDead=None):
    """Get a weak reference to obj. If obj is a bound method, a WeakMethod
    object, that behaves like a WeakRef, is returned; if it is
    anything else a WeakRef is returned. If obj is an unbound method,
    a ValueError will be raised."""
    if ismethod(obj):
        createRef = WeakMethod
    else:
        createRef = WeakRef
        
    return createRef(obj, notifyDead)
    
