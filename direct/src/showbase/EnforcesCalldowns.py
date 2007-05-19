__all__ = ['EnforcesCalldowns', 'calldownEnforced', 'EnforcedCalldownException',
           ]

from direct.showbase.PythonUtil import ClassTree, getBase
import new, __builtin__

class EnforcedCalldownException(Exception):
    def __init__(self, what):
        Exception.__init__(self, what)

class EnforcesCalldowns:
    """Derive from this class if you want to ensure that specific methods
    get called.  See calldownEnforced decorator below"""

    # class-level data for enforcement of base class method call-down
    #
    # The problem is that we don't have access to the class in the
    # decorator, so we need to put the decorated methods in a global
    # dict. We can then insert a stub method on each class instance for
    # every method that has enforced base-class methods, and the stub can
    # watch for each base-class method checkpoint to be passed.

    # since the decorator can't know its own id until after it has been
    # defined, we map from decorator ID to original func ID
    _decoId2funcId = {}
    # as calldownEnforced decorators are created, they add themselves to
    # this dict. At this point we don't know what class they belong to.
    _funcId2func = {}
    # this is here so that we can print nice error messages
    _funcId2class = {}
    # as class instances are created, we populate this dictionary of
    # class to func name to list of func ids. The lists of func ids
    # include base-class funcs.
    _class2funcName2funcIds = {}

    # this method will be inserted into instances of classes that need
    # to enforce base-class method calls, as the most-derived implementation
    # of the method
    @staticmethod
    def _enforceCalldowns(oldMethod, name, obj, *args, **kArgs):
        name2funcIds = EnforcesCalldowns._class2funcName2funcIds[obj.__class__]
        funcIds = name2funcIds.get(name)

        # prepare for the method call
        for funcId in funcIds:
            obj._EClatch(funcId)

        # call the actual method that we're stubbing
        result = oldMethod(*args, **kArgs)

        # check on the results
        for funcId in funcIds:
            obj._ECcheck(funcId)

        return result

    @staticmethod
    def notActive():
        return not (__dev__ and getBase().config.GetBool('enforce-calldowns', 1))

    def __init__(self):
        if EnforcesCalldowns.notActive():
            return

        # protect against multiple calldowns via multiple inheritance
        if hasattr(self, '_numInits'):
            self._numInits += 1
        else:
            self._numInits = 1

            # this map tracks how many times each func has been called
            self._funcId2calls = {}
            # this map tracks the 'latch' values for each func; if the call count
            # for a func is greater than the latch, then the func has been called.
            self._funcId2latch = {}

            if self.__class__ not in EnforcesCalldowns._class2funcName2funcIds:
                # prepare stubs to enforce method call-downs
                EnforcesCalldowns._class2funcName2funcIds.setdefault(self.__class__, {})
                # look through all of our base classes and find matches
                classes = ClassTree(self).getAllClasses()
                # collect IDs of all the enforced methods
                funcId2func = {}
                for cls in classes:
                    for name, item in cls.__dict__.items():
                        if id(item) in EnforcesCalldowns._decoId2funcId:
                            funcId = EnforcesCalldowns._decoId2funcId[id(item)]
                            funcId2func[funcId] = item
                            EnforcesCalldowns._funcId2class[funcId] = cls
                # add these funcs to the list for our class
                funcName2funcIds = EnforcesCalldowns._class2funcName2funcIds[self.__class__]
                for funcId, func in funcId2func.items():
                    funcName2funcIds.setdefault(func.__name__, [])
                    funcName2funcIds[func.__name__].append(funcId)

            # now run through all the enforced funcs for this class and insert
            # stub methods to do the enforcement
            funcName2funcIds = EnforcesCalldowns._class2funcName2funcIds[self.__class__]
            self._obscuredMethodNames = set()
            for name in funcName2funcIds:
                oldMethod = getattr(self, name)
                self._obscuredMethodNames.add(name)
                setattr(self, name, new.instancemethod(
                    Functor(EnforcesCalldowns._enforceCalldowns, oldMethod, name),
                    self, self.__class__))
            
    def EC_destroy(self):
        """this used to be called destroy() but it was masking destroy() functions
        on other classes that were multiply-inherited after ('to the right of')
        this class"""
        if EnforcesCalldowns.notActive():
            return
        # this must be called on destruction to prevent memory leaks
        # protect against multiple calldowns via multiple inheritance
        assert hasattr(self, '_numInits'), (
            'too many calls to EnforcesCalldowns.EC_destroy, class=%s' % self.__class__.__name__)
        if self._numInits == 1:
            for name in self._obscuredMethodNames:
                # Functors need to be destroyed to prevent garbage leaks
                getattr(self, name).destroy()
                delattr(self, name)
            del self._obscuredMethodNames
            # this opens up more cans of worms. Let's keep it closed for the moment
            #del self._funcId2calls
            #del self._funcId2latch
            del self._numInits
        else:
            self._numInits -= 1

    def skipCalldown(self, method):
        if EnforcesCalldowns.notActive():
            return
        # Call this function if you really don't want to call down to an
        # enforced base-class method. This should hardly ever be used.
        funcName2funcIds = EnforcesCalldowns._class2funcName2funcIds[self.__class__]
        funcIds = funcName2funcIds[method.__name__]
        for funcId in funcIds:
            self._ECvisit(funcId)

    def _EClatch(self, funcId):
        self._funcId2calls.setdefault(funcId, 0)
        self._funcId2latch[funcId] = self._funcId2calls[funcId]
    def _ECvisit(self, funcId):
        self._funcId2calls.setdefault(funcId, 0)
        self._funcId2calls[funcId] += 1
    def _ECcheck(self, funcId):
        if self._funcId2latch[funcId] == self._funcId2calls[funcId]:
            func = EnforcesCalldowns._funcId2func[funcId]
            __builtin__.classTree = ClassTree(self)
            raise EnforcedCalldownException(
                '%s.%s did not call down to %s.%s; type \'classTree\' to see hierarchy' % (
                self.__class__.__module__, self.__class__.__name__,
                EnforcesCalldowns._funcId2class[funcId].__name__,
                func.__name__))

def calldownEnforced(f):
    """
    Use this decorator to ensure that derived classes that override this method
    call down to the base class method.
    """
    if EnforcesCalldowns.notActive():
        return f
    def calldownEnforcedDecorator(obj, *args, **kArgs):
        # track the fact that this func has been called
        obj._ECvisit(id(f))
        f(obj, *args, **kArgs)
    calldownEnforcedDecorator.__doc__ = f.__doc__
    calldownEnforcedDecorator.__name__ = f.__name__
    calldownEnforcedDecorator.__module__ = f.__module__
    EnforcesCalldowns._decoId2funcId[id(calldownEnforcedDecorator)] = id(f)
    EnforcesCalldowns._funcId2func[id(f)] = calldownEnforcedDecorator
    return calldownEnforcedDecorator

if not EnforcesCalldowns.notActive():
    class CalldownEnforceTest(EnforcesCalldowns):
        @calldownEnforced
        def testFunc(self):
            pass
    class CalldownEnforceTestSubclass(CalldownEnforceTest):
        def testFunc(self):
            CalldownEnforceTest.testFunc(self)
        def destroy(self):
            CalldownEnforceTest.EC_destroy(self)
    class CalldownEnforceTestSubclassFail(CalldownEnforceTest):
        def testFunc(self):
            pass
    class CalldownEnforceTestSubclassSkip(CalldownEnforceTest):
        def testFunc(self):
            self.skipCalldown(CalldownEnforceTest.testFunc)
    class CalldownEnforceTestSubclass2(CalldownEnforceTest):
        def testFunc(self):
            CalldownEnforceTest.testFunc(self)
        def destroy(self):
            CalldownEnforceTest.EC_destroy(self)
    class CalldownEnforceTestDiamond(CalldownEnforceTestSubclass,
                                     CalldownEnforceTestSubclass2):
        def __init__(self):
            CalldownEnforceTestSubclass.__init__(self)
            CalldownEnforceTestSubclass2.__init__(self)
        def testFunc(self):
            CalldownEnforceTestSubclass.testFunc(self)
            CalldownEnforceTestSubclass2.testFunc(self)
        def destroy(self):
            CalldownEnforceTestSubclass.destroy(self)
            CalldownEnforceTestSubclass2.destroy(self)
            
    cets = CalldownEnforceTestSubclass()
    cetsf = CalldownEnforceTestSubclassFail()
    cetss = CalldownEnforceTestSubclassSkip()
    cetd = CalldownEnforceTestDiamond()
    raised = False
    try:
        cets.testFunc()
    except EnforcedCalldownException, e:
        raised = True
    if raised:
        raise "calldownEnforced raised when it shouldn't"
    raised = False
    try:
        cetsf.testFunc()
    except EnforcedCalldownException, e:
        raised = True
    if not raised:
        raise 'calldownEnforced failed to raise'
    raised = False
    try:
        cetss.testFunc()
    except EnforcedCalldownException, e:
        raised = True
    if raised:
        raise "calldownEnforced.skipCalldown raised when it shouldn't"
    raised = False
    cetd.testFunc()
    msg = ''
    try:
        # make sure we're OK to call down to destroy multiple times
        cetd.destroy()
    except Exception, e:
        msg = str(e)
        raised = True
    if raised:
        raise "calldownEnforcedDiamond.destroy raised when it shouldn't\n%s" % msg
    cetss.EC_destroy()
    cetsf.EC_destroy()
    cets.EC_destroy()
    del cetd
    del cetss
    del cetsf
    del cets
    del CalldownEnforceTestDiamond
    del CalldownEnforceTestSubclass2
    del CalldownEnforceTestSubclassSkip
    del CalldownEnforceTestSubclassFail
    del CalldownEnforceTestSubclass
    del CalldownEnforceTest
