"""
The new Finite State Machine module.  This replaces the modules
previously called FSM.py (now called ClassicFSM.py).
"""

from direct.showbase import DirectObject
from direct.directnotify import DirectNotifyGlobal
import types
import string

class FSM(DirectObject.DirectObject):
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
    exitState(), where "State" is the name of the state, e.g.:

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
    example:

    def filterRed(self, request, args):
        if request in ['Green']:
            return request
        return None

    def filterYellow(self, request, args):
        if request in ['Red']:
            return request
        return None

    def filterGreen(self, request, args):
        if request in ['Yellow']:
            return request
        return None

    As above, the filterState() functions are optional.  If any is
    omitted, the defaultFilter() method is called instead.  The
    default definition of defaultFilter() always returns None, thus
    disallowing all unexpected transitions.  This default behavior may
    be overridden in a derived class.

    FSM.state may be queried at any time other than during the
    handling of the enter() and exit() functions.  During these
    functions, FSM.state contains the value None (you are not really
    in any state during the transition).  At other times, FSM.state
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

    See the code in SampleFSM.py for further examples.
    """

    notify = DirectNotifyGlobal.directNotify.newCategory("FSM")

    def __init__(self, name):
        self.name = name
        # Initially, we are in the Off state by convention.
        self.state = 'Off'

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
        self.defaultTransitions = None

    def __del__(self):
        self.cleanup()

    def cleanup(self):
        # A convenience function to force the FSM to clean itself up
        # by transitioning to the "Off" state.
        assert(self.state)
        if self.state != 'Off':
            self.__setState('Off')

    def forceTransition(self, newState):
        """Changes unconditionally to the indicated state.  This
        bypasses the filterState() function, and just calls
        exitState() followed by enterState()."""

        assert(isinstance(newState, types.StringTypes))

        self.__setState(newState)

    def request(self, request, *args):
        """Requests a state transition (or other behavior).  The request
        parameter should be a string.  The request, along with any
        additional arguments, is passed to the current filterState()
        function.  If filterState() returns a string, the FSM
        transitions to that state.

        The return value is the same as the return value of
        filterState() (that is, None if the request does not provoke a
        state transition, otherwise it is a tuple containing the name
        of the state followed by any optional args.)
        
        If the FSM is currently in transition (i.e. in the middle of
        executing an enterState or exitState function), the request is
        denied and None is returned."""

        assert(isinstance(request, types.StringTypes))
        self.notify.debug("%s.request(%s, %s" % (self.name, request, str(args)[1:]))

        if not self.state:
            self.notify.warning("rejecting request %s while FSM is in transition." % (request))
            return None

        func = getattr(self, "filter" + self.state, None)
        if not func:
            # If there's no matching filterState() function, call
            # defaultFilter() instead.
            func = self.defaultFilter
        result = func(request, args)
        if result:
            if isinstance(result, types.StringTypes):
                # If the return value is a string, it's just the name
                # of the state.  Wrap it in a tuple for consistency.
                result = (result,)

            # Otherwise, assume it's a (name, *args) tuple
            self.__setState(*result)

        return result

    def defaultFilter(self, request, args):
        """This is the function that is called if there is no
        filterState() method for a particular state name.  By default,
        the filter defined here in the base class always returns
        None, disallowing any transition.  Specialized FSM's may wish
        to redefine this default filter (for instance, to always
        return the request itself, thus allowing any transition.)."""

        if request == 'Off':
            # We can always go to the "Off" state.
            return (request,) + args

        if self.defaultTransitions is None:
            # If self.defaultTransitions is None, it means to accept
            # all requests whose name begins with a capital letter.
            # These are direct requests to a particular state.
            if request[0] in string.uppercase:
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
            if request[0] in string.uppercase:
                self.notify.error("%s rejecting request %s from state %s." % (self.name, request, self.state))

        # In either case, we quietly ignore unhandled command
        # (lowercase) requests.
        assert(self.notify.debug("%s ignoring request %s from state %s." % (self.name, request, self.state)))
        return None

    def filterOff(self, request, args):
        """From the off state, we can always go directly to any other
        state."""
        if request[0] in string.uppercase:
            return (request,) + args
        return self.defaultFilter(request, args)
        

    def setStateArray(self, stateArray):
        """array of unique states to iterate through"""
        self.stateArray = stateArray

    def requestNext(self, *args):
        """request the 'next' state in the predefined state array"""
        assert (self.state in self.stateArray)

        curIndex = self.stateArray.index(self.state)
        newIndex = (curIndex + 1) % len(self.stateArray)

        self.request(self.stateArray[newIndex], args)

    def requestPrev(self, *args):
        """request the 'previous' state in the predefined state array"""
        assert (self.state in self.stateArray)

        curIndex = self.stateArray.index(self.state)
        newIndex = (curIndex - 1) % len(self.stateArray)

        self.request(self.stateArray[newIndex], args)
        

    def __setState(self, newState, *args):
        # Internal function to change unconditionally to the indicated
        # state.
        assert(self.state)
        
        self.oldState = self.state
        self.newState = newState
        self.state = None
        self.__callTransitionFunc("exit" + self.oldState)
        self.__callTransitionFunc("enter" + self.newState, *args)
        self.state = newState
        del self.oldState
        del self.newState
        

    def __callTransitionFunc(self, name, *args):
        # Calls the appropriate enter or exit function when
        # transitioning between states, if it exists.
        assert(self.state == None)

        func = getattr(self, name, None)
        if func:
            func(*args)
            
    def __repr__(self):
        return self.__str__()

    def __str__(self):
        """
        Print out something useful about the fsm
        """
        if self.state:
            str = ("FSM " + self.name + ' in state "' + self.state + '"')
        else:
            str = ("FSM " + self.name + ' not in any state')
        return str
