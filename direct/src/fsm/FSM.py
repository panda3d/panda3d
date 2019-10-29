"""The new Finite State Machine module. This replaces the module
previously called FSM (now called :mod:`.ClassicFSM`).

For more information on FSMs, consult the :ref:`finite-state-machines` section
of the programming manual.
"""

__all__ = ['FSMException', 'FSM']


from direct.showbase.DirectObject import DirectObject
from direct.directnotify import DirectNotifyGlobal
from direct.showbase import PythonUtil
from direct.stdpy.threading import RLock


class FSMException(Exception):
    pass


class AlreadyInTransition(FSMException):
    pass


class RequestDenied(FSMException):
    pass


class FSM(DirectObject):
    """
    A Finite State Machine.  This is intended to be the base class
    of any number of specific machines, which consist of a collection
    of states and transitions, and rules to switch between states
    according to arbitrary input data.

    The states of an FSM are defined implicitly.  Each state is
    identified by a string, which by convention begins with a capital
    letter.  (Also by convention, strings passed to request that are
    not state names should begin with a lowercase letter.)

    To define specialized behavior when entering or exiting a
    particular state, define a method named enterState() and/or
    exitState(), where "State" is the name of the state, e.g.::

        def enterRed(self):
            ... do stuff ...

        def exitRed(self):
            ... cleanup stuff ...

        def enterYellow(self):
            ... do stuff ...

        def exitYellow(self):
            ... cleanup stuff ...

        def enterGreen(self):
            ... do stuff ...

        def exitGreen(self):
            ... cleanup stuff ...

    Both functions can access the previous state name as
    self.oldState, and the new state name we are transitioning to as
    self.newState.  (Of course, in enterRed(), self.newState will
    always be "Red", and the in exitRed(), self.oldState will always
    be "Red".)

    Both functions are optional.  If either function is omitted, the
    state is still defined, but nothing is done when transitioning
    into (or out of) the state.

    Additionally, you may define a filterState() function for each
    state.  The purpose of this function is to decide what state to
    transition to next, if any, on receipt of a particular input.  The
    input is always a string and a tuple of optional parameters (which
    is often empty), and the return value should either be None to do
    nothing, or the name of the state to transition into.  For
    example::

        def filterRed(self, request, args):
            if request in ['Green']:
                return (request,) + args
            return None

        def filterYellow(self, request, args):
            if request in ['Red']:
                return (request,) + args
            return None

        def filterGreen(self, request, args):
            if request in ['Yellow']:
                return (request,) + args
            return None

    As above, the filterState() functions are optional.  If any is
    omitted, the defaultFilter() method is called instead.  A standard
    implementation of defaultFilter() is provided, which may be
    overridden in a derived class to change the behavior on an
    unexpected transition.

    If self.defaultTransitions is left unassigned, then the standard
    implementation of defaultFilter() will return None for any
    lowercase transition name and allow any uppercase transition name
    (this assumes that an uppercase name is a request to go directly
    to a particular state by name).

    self.state may be queried at any time other than during the
    handling of the enter() and exit() functions.  During these
    functions, self.state contains the value None (you are not really
    in any state during the transition).  However, during a transition
    you *can* query the outgoing and incoming states, respectively,
    via self.oldState and self.newState.  At other times, self.state
    contains the name of the current state.

    Initially, the FSM is in state 'Off'.  It does not call enterOff()
    at construction time; it is simply in Off already by convention.
    If you need to call code in enterOff() to initialize your FSM
    properly, call it explicitly in the constructor.  Similarly, when
    cleanup() is called or the FSM is destructed, the FSM transitions
    back to 'Off' by convention.  (It does call enterOff() at this
    point, but does not call exitOff().)

    To implement nested hierarchical FSM's, simply create a nested FSM
    and store it on the class within the appropriate enterState()
    function, and clean it up within the corresponding exitState()
    function.

    There is a way to define specialized transition behavior between
    two particular states.  This is done by defining a from<X>To<Y>()
    function, where X is the old state and Y is the new state.  If this
    is defined, it will be run in place of the exit<X> and enter<Y>
    functions, so if you want that behavior, you'll have to call them
    specifically.  Otherwise, you can completely replace that transition's
    behavior.

    See the code in SampleFSM.py for further examples.
    """

    notify = DirectNotifyGlobal.directNotify.newCategory("FSM")

    SerialNum = 0

    # This member lists the default transitions that are accepted
    # without question by the defaultFilter.  It's a map of state
    # names to a list of legal target state names from that state.
    # Define it only if you want to use the classic FSM model of
    # defining all (or most) of your transitions up front.  If
    # this is set to None (the default), all named-state
    # transitions (that is, those requests whose name begins with
    # a capital letter) are allowed.  If it is set to an empty
    # map, no transitions are implicitly allowed--all transitions
    # must be approved by some filter function.
    defaultTransitions = None

    def __init__(self, name):
        self.fsmLock = RLock()
        self._name = name
        self.stateArray = []
        self._serialNum = FSM.SerialNum
        FSM.SerialNum += 1
        self._broadcastStateChanges = False
        # Initially, we are in the Off state by convention.
        self.state = 'Off'

        # This member records transition requests made by demand() or
        # forceTransition() while the FSM is in transition between
        # states.
        self.__requestQueue = []

        if __debug__:
            from direct.fsm.ClassicFSM import _debugFsms
            import weakref
            _debugFsms[name]=weakref.ref(self)

    def cleanup(self):
        # A convenience function to force the FSM to clean itself up
        # by transitioning to the "Off" state.
        self.fsmLock.acquire()
        try:
            assert self.state
            if self.state != 'Off':
                self.__setState('Off')
        finally:
            self.fsmLock.release()

    def setBroadcastStateChanges(self, doBroadcast):
        self._broadcastStateChanges = doBroadcast
    def getStateChangeEvent(self):
        # if setBroadcastStateChanges(True), this event will be sent through
        # the messenger on every state change. The new and old states are
        # accessible as self.oldState and self.newState, and the transition
        # functions will already have been called.
        return 'FSM-%s-%s-stateChange' % (self._serialNum, self._name)

    def getCurrentFilter(self):
        if not self.state:
            error = "FSM cannot determine current filter while in transition (%s -> %s)." % (self.oldState, self.newState)
            raise AlreadyInTransition(error)

        filter = getattr(self, "filter" + self.state, None)
        if not filter:
            # If there's no matching filterState() function, call
            # defaultFilter() instead.
            filter = self.defaultFilter

        return filter

    def getCurrentOrNextState(self):
        # Returns the current state if we are in a state now, or the
        # state we are transitioning into if we are currently within
        # the enter or exit function for a state.
        self.fsmLock.acquire()
        try:
            if self.state:
                return self.state
            return self.newState
        finally:
            self.fsmLock.release()

    def getCurrentStateOrTransition(self):
        # Returns the current state if we are in a state now, or the
        # transition we are performing if we are currently within
        # the enter or exit function for a state.
        self.fsmLock.acquire()
        try:
            if self.state:
                return self.state
            return '%s -> %s' % (self.oldState, self.newState)
        finally:
            self.fsmLock.release()

    def isInTransition(self):
        self.fsmLock.acquire()
        try:
            return self.state == None
        finally:
            self.fsmLock.release()

    def forceTransition(self, request, *args):
        """Changes unconditionally to the indicated state.  This
        bypasses the filterState() function, and just calls
        exitState() followed by enterState()."""

        self.fsmLock.acquire()
        try:
            assert isinstance(request, str)
            self.notify.debug("%s.forceTransition(%s, %s" % (
                self._name, request, str(args)[1:]))

            if not self.state:
                # Queue up the request.
                self.__requestQueue.append(PythonUtil.Functor(
                    self.forceTransition, request, *args))
                return

            self.__setState(request, *args)
        finally:
            self.fsmLock.release()

    def demand(self, request, *args):
        """Requests a state transition, by code that does not expect
        the request to be denied.  If the request is denied, raises a
        RequestDenied exception.

        Unlike request(), this method allows a new request to be made
        while the FSM is currently in transition.  In this case, the
        request is queued up and will be executed when the current
        transition finishes.  Multiple requests will queue up in
        sequence.
        """

        self.fsmLock.acquire()
        try:
            assert isinstance(request, str)
            self.notify.debug("%s.demand(%s, %s" % (
                self._name, request, str(args)[1:]))
            if not self.state:
                # Queue up the request.
                self.__requestQueue.append(PythonUtil.Functor(
                    self.demand, request, *args))
                return

            if not self.request(request, *args):
                raise RequestDenied("%s (from state: %s)" % (request, self.state))
        finally:
            self.fsmLock.release()

    def request(self, request, *args):
        """Requests a state transition (or other behavior).  The
        request may be denied by the FSM's filter function.  If it is
        denied, the filter function may either raise an exception
        (RequestDenied), or it may simply return None, without
        changing the FSM's state.

        The request parameter should be a string.  The request, along
        with any additional arguments, is passed to the current
        filterState() function.  If filterState() returns a string,
        the FSM transitions to that state.

        The return value is the same as the return value of
        filterState() (that is, None if the request does not provoke a
        state transition, otherwise it is a tuple containing the name
        of the state followed by any optional args.)

        If the FSM is currently in transition (i.e. in the middle of
        executing an enterState or exitState function), an
        AlreadyInTransition exception is raised (but see demand(),
        which will queue these requests up and apply when the
        transition is complete)."""

        self.fsmLock.acquire()
        try:
            assert isinstance(request, str)
            self.notify.debug("%s.request(%s, %s" % (
                self._name, request, str(args)[1:]))

            filter = self.getCurrentFilter()
            result = filter(request, args)
            if result:
                if isinstance(result, str):
                    # If the return value is a string, it's just the name
                    # of the state.  Wrap it in a tuple for consistency.
                    result = (result,) + args

                # Otherwise, assume it's a (name, *args) tuple
                self.__setState(*result)

            return result
        finally:
            self.fsmLock.release()

    def defaultEnter(self, *args):
        """ This is the default function that is called if there is no
        enterState() method for a particular state name. """
        pass

    def defaultExit(self):
        """ This is the default function that is called if there is no
        exitState() method for a particular state name. """
        pass

    def defaultFilter(self, request, args):
        """This is the function that is called if there is no
        filterState() method for a particular state name.

        This default filter function behaves in one of two modes:

        (1) if self.defaultTransitions is None, allow any request
        whose name begins with a capital letter, which is assumed to
        be a direct request to a particular state.  This is similar to
        the old ClassicFSM onUndefTransition=ALLOW, with no explicit
        state transitions listed.

        (2) if self.defaultTransitions is not None, allow only those
        requests explicitly identified in this map.  This is similar
        to the old ClassicFSM onUndefTransition=DISALLOW, with an
        explicit list of allowed state transitions.

        Specialized FSM's may wish to redefine this default filter
        (for instance, to always return the request itself, thus
        allowing any transition.)."""

        if request == 'Off':
            # We can always go to the "Off" state.
            return (request,) + args

        if self.defaultTransitions is None:
            # If self.defaultTransitions is None, it means to accept
            # all requests whose name begins with a capital letter.
            # These are direct requests to a particular state.
            if request[0].isupper():
                return (request,) + args
        else:
            # If self.defaultTransitions is not None, it is a map of
            # allowed transitions from each state.  That is, each key
            # of the map is the current state name; for that key, the
            # value is a list of allowed transitions from the
            # indicated state.
            if request in self.defaultTransitions.get(self.state, []):
                # This transition is listed in the defaultTransitions map;
                # accept it.
                return (request,) + args

            # If self.defaultTransitions is not None, it is an error
            # to request a direct state transition (capital letter
            # request) not listed in defaultTransitions and not
            # handled by an earlier filter.
            if request[0].isupper():
                raise RequestDenied("%s (from state: %s)" % (request, self.state))

        # In either case, we quietly ignore unhandled command
        # (lowercase) requests.
        assert self.notify.debug("%s ignoring request %s from state %s." % (self._name, request, self.state))
        return None

    def filterOff(self, request, args):
        """From the off state, we can always go directly to any other
        state."""
        if request[0].isupper():
            return (request,) + args
        return self.defaultFilter(request, args)


    def setStateArray(self, stateArray):
        """array of unique states to iterate through"""
        self.fsmLock.acquire()
        try:
            self.stateArray = stateArray
        finally:
            self.fsmLock.release()


    def requestNext(self, *args):
        """Request the 'next' state in the predefined state array."""
        self.fsmLock.acquire()
        try:
            if self.stateArray:
                if not self.state in self.stateArray:
                    self.request(self.stateArray[0])
                else:
                    cur_index = self.stateArray.index(self.state)
                    new_index = (cur_index + 1) % len(self.stateArray)
                    self.request(self.stateArray[new_index], args)
            else:
                assert self.notifier.debug(
                                    "stateArray empty. Can't switch to next.")

        finally:
            self.fsmLock.release()

    def requestPrev(self, *args):
        """Request the 'previous' state in the predefined state array."""
        self.fsmLock.acquire()
        try:
            if self.stateArray:
                if not self.state in self.stateArray:
                    self.request(self.stateArray[0])
                else:
                    cur_index = self.stateArray.index(self.state)
                    new_index = (cur_index - 1) % len(self.stateArray)
                    self.request(self.stateArray[new_index], args)
            else:
                assert self.notifier.debug(
                                    "stateArray empty. Can't switch to next.")
        finally:
            self.fsmLock.release()

    def __setState(self, newState, *args):
        # Internal function to change unconditionally to the indicated
        # state.
        assert self.state
        assert self.notify.debug("%s to state %s." % (self._name, newState))

        self.oldState = self.state
        self.newState = newState
        self.state = None

        try:
            if not self.__callFromToFunc(self.oldState, self.newState, *args):
                self.__callExitFunc(self.oldState)
                self.__callEnterFunc(self.newState, *args)
                pass
            pass
        except:
            # If we got an exception during the enter or exit methods,
            # go directly to state "InternalError" and raise up the
            # exception.  This might leave things a little unclean
            # since we've partially transitioned, but what can you do?

            self.state = 'InternalError'
            del self.oldState
            del self.newState
            raise

        if self._broadcastStateChanges:
            messenger.send(self.getStateChangeEvent())

        self.state = newState
        del self.oldState
        del self.newState

        if self.__requestQueue:
            request = self.__requestQueue.pop(0)
            assert self.notify.debug("%s continued queued request." % (self._name))
            request()

    def __callEnterFunc(self, name, *args):
        # Calls the appropriate enter function when transitioning into
        # a new state, if it exists.
        assert self.state == None and self.newState == name

        func = getattr(self, "enter" + name, None)
        if not func:
            # If there's no matching enterFoo() function, call
            # defaultEnter() instead.
            func = self.defaultEnter
        func(*args)

    def __callFromToFunc(self, oldState, newState, *args):
        # Calls the appropriate fromTo function when transitioning into
        # a new state, if it exists.
        assert self.state == None and self.oldState == oldState and self.newState == newState

        func = getattr(self, "from%sTo%s" % (oldState,newState), None)
        if func:
            func(*args)
            return True
        return False

    def __callExitFunc(self, name):
        # Calls the appropriate exit function when leaving a
        # state, if it exists.
        assert self.state == None and self.oldState == name

        func = getattr(self, "exit" + name, None)
        if not func:
            # If there's no matching exitFoo() function, call
            # defaultExit() instead.
            func = self.defaultExit
        func()

    def __repr__(self):
        return self.__str__()

    def __str__(self):
        """
        Print out something useful about the fsm
        """
        self.fsmLock.acquire()
        try:
            className = self.__class__.__name__
            if self.state:
                str = ('%s FSM:%s in state "%s"' % (className, self._name, self.state))
            else:
                str = ('%s FSM:%s in transition from \'%s\' to \'%s\'' % (className, self._name, self.oldState, self.newState))
            return str
        finally:
            self.fsmLock.release()
