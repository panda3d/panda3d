# classes for event-driven programming
# http://en.wikipedia.org/wiki/Event-driven_programming

__all__ = ['StateVar', 'FunctionCall', 'EnterExit', 'Pulse', 'EventPulse',
           'EventArgument', ]

from direct.showbase.DirectObject import DirectObject

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
        subscriber._recvStatePush(self)

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
    def __init__(self, source):
        self._source = None
        self._initSource = source

    def _finishInit(self):
        # initialization is split across two functions to allow objects that derive from this
        # class to set everything up so that they can respond appropriately to the initial
        # state push from the state source
        self._subscribeTo(self._initSource)
        del self._initSource

    def destroy(self):
        self._unsubscribe()
        del self._source

    def _subscribeTo(self, source):
        self._unsubscribe()
        self._source = source
        if self._source:
            self._source._addSubscription(self)

    def _unsubscribe(self):
        if self._source:
            self._source._removeSubscription(self)
            self._source = None

    def _recvStatePush(self, source):
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
    def __init__(self, source):
        ReceivesStateChanges.__init__(self, source)
        PushesStateChanges.__init__(self, source.getState())
        ReceivesStateChanges._finishInit(self)

    def destroy(self):
        PushesStateChanges.destroy(self)
        ReceivesStateChanges.destroy(self)

    def _recvStatePush(self, source):
        # got a state push, apply new state to self
        self._handlePotentialStateChange(source._value)

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
    def __init__(self, source, func):
        self._func = func
        StateChangeNode.__init__(self, source)

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
    del handler
    del l

class EnterExit(StateChangeNode):
    # call enterFunc when our state becomes true, exitFunc when it becomes false
    def __init__(self, source, enterFunc, exitFunc):
        self._enterFunc = enterFunc
        self._exitFunc = exitFunc
        StateChangeNode.__init__(self, source)

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
    ee.destroy()
    sv.destroy()
    del ee
    del sv
    del enter
    del exit
    del l

class Pulse(PushesStateChanges):
    # changes state to True then immediately to False whenever sendPulse is called
    def __init__(self):
        PushesStateChanges.__init__(self, False)

    def sendPulse(self):
        self._handlePotentialStateChange(True)
        self._handlePotentialStateChange(False)
    
if __debug__:
    l = []
    def handler(value, l=l):
        l.append(value)
    p = Pulse()
    fc = FunctionCall(p, handler)
    assert l == []
    p.sendPulse()
    assert l == [True, False, ]
    p.sendPulse()
    assert l == [True, False, True, False, ]
    fc.destroy()
    p.destroy()
    del fc
    del p
    del l
    del handler

class EventPulse(Pulse, DirectObject):
    # sends a True-False "pulse" whenever a specific messenger message is sent
    def __init__(self, event):
        Pulse.__init__(self)
        self.accept(event, self.sendPulse)

    def destroy(self):
        self.ignoreAll()
        Pulse.destroy(self)

if __debug__:
    l = []
    def handler(value, l=l):
        l.append(value)
    ep = EventPulse('testEvent')
    fc = FunctionCall(ep, handler)
    assert l == []
    messenger.send('testEvent')
    assert l == [True, False, ]
    messenger.send('testEvent')
    assert l == [True, False, True, False, ]
    fc.destroy()
    ep.destroy()
    del fc
    del ep
    del l
    del handler
    
class EventArgument(PushesStateChanges, DirectObject):
    # tracks a particular argument to a particular messenger event
    def __init__(self, event, index=0):
        PushesStateChanges.__init__(self, None)
        self._index = index
        self.accept(event, self._handleEvent)

    def destroy(self):
        self.ignoreAll()
        del self._index
        PushesStateChanges.destroy(self)
        
    def _handleEvent(self, *args):
        self._handlePotentialStateChange(args[self._index])

if __debug__:
    l = []
    def handler(value, l=l):
        l.append(value)
    ea = EventArgument('testEvent', index=1)
    fc = FunctionCall(ea, handler)
    messenger.send('testEvent', ['a', 'b'])
    assert l == ['b',]
    messenger.send('testEvent', [1, 2, 3, ])
    assert l == ['b', 2, ]
    fc.destroy()
    ea.destroy()
    del fc
    del ea
    del l

