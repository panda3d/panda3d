class PushesStateChanges:
    # base class for objects that broadcast state changes to a set of subscriber objects
    def __init__(self, value):
        self._value = value
        # push state changes to these objects
        self._subscribers = set()

    def destroy(self):
        if len(self._subscribers) != 0:
            raise '%s object still has subscribers in destroy(): %s' % (
                self.__class__.__name__, self._subscribers)
        del self._subscribers
        del self._value

    def getState(self):
        return self._value

    def _addSubscription(self, subscriber):
        self._subscribers.add(subscriber)

    def _removeSubscription(self, subscriber):
        self._subscribers.remove(subscriber)

    def _handlePotentialStateChange(self, value):
        oldValue = self._value
        self._value = value
        if oldValue != value:
            self._handleStateChange()

    def _handleStateChange(self):
        # push this object's state to the subscribing objects
        for subscriber in self._subscribers:
            subscriber._recvStatePush(self)

if __debug__:
    psc = PushesStateChanges(0)
    assert psc.getState() == 0
    psc.destroy()
    del psc

class ReceivesStateChanges:
    # base class for objects that subscribe to state changes from PushesStateChanges objects
    def __init__(self, other):
        self._other = None
        self._subscribeTo(other)

    def destroy(self):
        self._unsubscribe()
        del self._other

    def _subscribeTo(self, other):
        self._unsubscribe()
        self._other = other
        if self._other:
            self._other._addSubscription(self)

    def _unsubscribe(self):
        if self._other:
            self._other._removeSubscription(self)
            self._other = None

    def _recvStatePush(self, other):
        pass

if __debug__:
    rsc = ReceivesStateChanges(None)
    rsc.destroy()
    del rsc

class StateVar(PushesStateChanges):
    # coder-friendly object that allows values to be set on it and pushes those values
    # as state changes
    def set(self, value):
        PushesStateChanges._handlePotentialStateChange(self, value)

    def get(self):
        return PushesStateChanges.getState(self)

if __debug__:
    sv = StateVar(0)
    assert sv.get() == 0
    sv.set(1)
    assert sv.get() == 1
    sv.destroy()
    del sv

class StateChangeNode(PushesStateChanges, ReceivesStateChanges):
    # base class that can be used to create a state-change notification chain
    def __init__(self, other):
        ReceivesStateChanges.__init__(self, other)
        PushesStateChanges.__init__(self, other.getState())

    def destroy(self):
        PushesStateChanges.destroy(self)
        ReceivesStateChanges.destroy(self)

    def _recvStatePush(self, other):
        # got a state push, apply new state to self
        self._handlePotentialStateChange(other._value)

if __debug__:
    sv = StateVar(0)
    assert sv.get() == 0
    scn = StateChangeNode(sv)
    assert scn.getState() == 0
    sv.set(1)
    assert sv.get() == 1
    assert scn.getState() == 1
    scn2 = StateChangeNode(scn)
    assert scn2.getState() == 1
    sv.set(2)
    assert scn2.getState() == 2
    scn3 = StateChangeNode(scn)
    assert scn3.getState() == 2
    sv.set(3)
    assert scn2.getState() == 3
    assert scn3.getState() == 3
    scn3.destroy()
    scn2.destroy()
    scn.destroy()
    sv.destroy()
    del scn3
    del scn2
    del scn
    del sv

class FunctionCall(StateChangeNode):
    # calls func with new state whenever state changes
    def __init__(self, other, func):
        self._func = func
        StateChangeNode.__init__(self, other)

    def destroy(self):
        StateChangeNode.destroy(self)
        del self._func

    def _handleStateChange(self):
        self._func(self._value)
        StateChangeNode._handleStateChange(self)
        
if __debug__:
    l = []
    def handler(value, l=l):
        l.append(value)
    assert l == []
    sv = StateVar(0)
    fc = FunctionCall(sv, handler)
    assert l == []
    sv.set(1)
    assert l == [1,]
    sv.set(2)
    assert l == [1,2,]
    fc.destroy()
    sv.destroy()
    del fc
    del sv

class EnterExit(StateChangeNode):
    # call enterFunc when our state becomes true, exitFunc when it becomes false
    def __init__(self, other, enterFunc, exitFunc):
        self._enterFunc = enterFunc
        self._exitFunc = exitFunc
        StateChangeNode.__init__(self, other)

    def destroy(self):
        StateChangeNode.destroy(self)
        del self._exitFunc
        del self._enterFunc

    def _handlePotentialStateChange(self, value):
        # convert the incoming state as a bool
        StateChangeNode._handlePotentialStateChange(self, bool(value))

    def _handleStateChange(self):
        if self._value:
            self._enterFunc()
        else:
            self._exitFunc()
        StateChangeNode._handleStateChange(self)

if __debug__:
    l = []
    def enter(l=l):
        l.append(1)
    def exit(l=l):
        l.append(0)
    sv = StateVar(0)
    ee = EnterExit(sv, enter, exit)
    sv.set(0)
    assert l == []
    sv.set(1)
    assert l == [1,]
    sv.set(2)
    assert l == [1,]
    sv.set(0)
    assert l == [1,0,]
    sv.set(True)
    assert l == [1,0,1,]
    sv.set(False)
    assert l == [1,0,1,0,]
