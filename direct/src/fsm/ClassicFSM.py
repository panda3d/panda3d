"""Finite State Machine module: contains the ClassicFSM class.

This module and class exist only for backward compatibility with
existing code.  New code should use the FSM module instead.

"""

from DirectObject import *
import types

class ClassicFSM(DirectObject):
    """ClassicFSM class: Finite State Machine class.

    This module and class exist only for backward compatibility with
    existing code.  New code should use the FSM class instead.
    """

    # create ClassicFSM DirectNotify category
    notify = directNotify.newCategory("ClassicFSM")

    # special methods

    # these are flags that tell the ClassicFSM what to do when an
    # undefined transition is requested:
    ALLOW = 0            # print a warning, and do the transition
    DISALLOW = 1         # silently ignore the request (don't do the transition)
    DISALLOW_VERBOSE = 2 # print a warning, and don't do the transition
    ERROR = 3            # print an error message and raise an exception

    def __init__(self, name, states=[], initialStateName=None,
                 finalStateName=None, onUndefTransition=DISALLOW_VERBOSE):
        """__init__(self, string, State[], string, string, int)

        ClassicFSM constructor: takes name, list of states, initial state and
        final state as:

        fsm = ClassicFSM.ClassicFSM('stopLight',
          [ State.State('red', enterRed, exitRed, ['green']),
            State.State('yellow', enterYellow, exitYellow, ['red']),
            State.State('green', enterGreen, exitGreen, ['yellow']) ],
          'red',
          'red')

        each state's last argument, a list of allowed state transitions,
        is optional; if left out (or explicitly specified to be
        State.State.Any) then any transition from the state is 'defined'
        and allowed

        'onUndefTransition' flag determines behavior when undefined
        transition is requested; see flag definitions above

        """

        self.setName(name)
        self.setStates(states)
        self.setInitialState(initialStateName)
        self.setFinalState(finalStateName)

        self.onUndefTransition = onUndefTransition

        # Flag to see if we are inspecting
        self.inspecting = 0

        # We do not enter the initial state to separate
        # construction from activation
        self.__currentState = None

        # We set this while we are modifying the state.  No one else
        # should recursively attempt to modify the state while we are
        # doing this.
        self.__internalStateInFlux = 0

    # I know this isn't how __repr__ is supposed to be used, but it
    # is nice and convenient.
    def __repr__(self):
        return self.__str__()

    def __str__(self):
        """__str__(self)
        Print out something useful about the fsm
        """
        currentState = self.getCurrentState()
        if currentState:
            str = ("ClassicFSM " + self.getName() + ' in state "' +
                   currentState.getName() + '"')
        else:
            str = ("ClassicFSM " + self.getName() + ' not in any state')
        return str

    def enterInitialState(self, argList=[]):
        assert(not self.__internalStateInFlux)
        if self.__currentState == self.__initialState:
            return
        
        assert(self.__currentState == None)
        self.__internalStateInFlux = 1
        self.__enter(self.__initialState, argList)
        assert(not self.__internalStateInFlux)

    # Jesse decided that simpler was better with the __str__ function
    def __str_not__(self):
        """__str__(self)"""
        return "ClassicFSM: name = %s \n states = %s \n initial = %s \n final = %s \n current = %s" \
            % (self.__name, self.__states, self.__initialState,
               self.__finalState, self.__currentState)


    # setters and getters

    def getName(self):
        """getName(self)"""
        return(self.__name)

    def setName(self, name):
        """setName(self, string)"""
        self.__name = name

    def getStates(self):
        """getStates(self)"""
        return(self.__states)

    def setStates(self, states):
        """setStates(self, State[])"""
        self.__states = states

    def addState(self, state):
        """addState(state)"""
        self.__states.append(state)

    def getInitialState(self):
        """getInitialState(self)"""
        return(self.__initialState)

    def setInitialState(self, initialStateName):
        """setInitialState(self, string)"""
        self.__initialState = self.getStateNamed(initialStateName)

    def getFinalState(self):
        """getFinalState(self)"""
        return(self.__finalState)

    def setFinalState(self, finalStateName):
        """setFinalState(self, string)"""
        self.__finalState = self.getStateNamed(finalStateName)

    def requestFinalState(self):
        self.request(self.__finalState.getName())

    def getCurrentState(self):
        """getCurrentState(self)"""
        return(self.__currentState)


    # lookup funcs

    def getStateNamed(self, stateName):
        """getStateNamed(self, string)
        Return the state with given name if found, issue warning otherwise"""
        for state in self.__states:
            if (state.getName() == stateName):
                return state
        ClassicFSM.notify.warning("[%s] : getStateNamed: %s, no such state" %
                           (self.__name, str(stateName)))


    # basic ClassicFSM functionality

    def __exitCurrent(self, argList):
        """__exitCurrent(self)
        Exit the current state"""
        assert(self.__internalStateInFlux)
        if ClassicFSM.notify.getDebug():
            ClassicFSM.notify.debug("[%s]: exiting %s" % (self.__name,
                                                   self.__currentState.getName()))
        self.__currentState.exit(argList)
        # Only send the state change event if we are inspecting it
        # If this event turns out to be generally useful, we can
        # turn it on all the time, but for now nobody else is using it
        if self.inspecting:
            messenger.send(self.getName() + '_' +
                           self.__currentState.getName() + '_exited')
        self.__currentState = None

    def __enter(self, aState, argList=[]):
        """__enter(self, State)
        Enter a given state, if it exists"""
        assert(self.__internalStateInFlux)
        if (aState in self.__states):
            if ClassicFSM.notify.getDebug():
                ClassicFSM.notify.debug("[%s]: entering %s" % (self.__name,
                                                        aState.getName()))
            self.__currentState = aState
            # Only send the state change event if we are inspecting it
            # If this event turns out to be generally useful, we can
            # turn it on all the time, but for now nobody else is using it
            if self.inspecting:
                messenger.send(self.getName() + '_' +
                               aState.getName() + '_entered')

            # Once we begin entering the new state, we're allow to
            # recursively request a transition to another state.
            # Indicate this by marking our internal state no longer in
            # flux.
            self.__internalStateInFlux = 0
            aState.enter(argList)
        else:
            # notify.error is going to raise an exception; reset the
            # flux flag first
            self.__internalStateInFlux = 0
            ClassicFSM.notify.error("[%s]: enter: no such state" % (self.__name))

    def __transition(self, aState, enterArgList=[], exitArgList=[]):
        """__transition(self, State, enterArgList, exitArgList)
        Exit currentState and enter given one"""
        assert(not self.__internalStateInFlux)
        self.__internalStateInFlux = 1
        self.__exitCurrent(exitArgList)
        self.__enter(aState, enterArgList)
        assert(not self.__internalStateInFlux)

    def request(self, aStateName, enterArgList=[], exitArgList=[],
                force=0):
        """request(self, string)
        Attempt transition from currentState to given one.
        Return true is transition exists to given state,
        false otherwise.
        """

        # If you trigger this assertion failure, you must have
        # recursively requested a state transition from within the
        # exitState() function for the previous state.  This is not
        # supported because we're not fully transitioned into the new
        # state yet.
        assert(not self.__internalStateInFlux)

        if not self.__currentState:
            # Make this a warning for now
            ClassicFSM.notify.warning("[%s]: request: never entered initial state" %
                               (self.__name))
            self.__currentState = self.__initialState

        if isinstance(aStateName, types.StringType):
            aState = self.getStateNamed(aStateName)
        else:
            # Allow the caller to pass in a state in itself, not just
            # the name of a state.
            aState = aStateName
            aStateName = aState.getName()

        if aState == None:
            ClassicFSM.notify.error("[%s]: request: %s, no such state" %
                             (self.__name, aStateName))

        # is the transition defined? if it isn't, should we allow it?
        transitionDefined = self.__currentState.isTransitionDefined(aStateName)
        transitionAllowed = transitionDefined

        if self.onUndefTransition == ClassicFSM.ALLOW:
            transitionAllowed = 1
            if not transitionDefined:
                # the transition is not defined, but we're going to do it
                # anyway. print a warning.
                ClassicFSM.notify.warning(
                    "[%s]: performing undefined transition from %s to %s" %
                    (self.__name,
                     self.__currentState.getName(),
                     aStateName))

        if transitionAllowed or force:
            self.__transition(aState,
                              enterArgList,
                              exitArgList)
            return 1
        # We can implicitly always transition to our final state.
        elif (aStateName == self.__finalState.getName()):
            if (self.__currentState == self.__finalState):
                # Do not do the transition if we are already in the final state
                if ClassicFSM.notify.getDebug():
                    ClassicFSM.notify.debug("[%s]: already in final state: %s" %
                                     (self.__name, aStateName))
                return 1
            else:
                # Force a transition to allow for cleanup
                if ClassicFSM.notify.getDebug():
                    ClassicFSM.notify.debug("[%s]: implicit transition to final state: %s" %
                                     (self.__name, aStateName))
                self.__transition(aState,
                                  enterArgList,
                                  exitArgList)
                return 1
        # are we already in this state?
        elif (aStateName == self.__currentState.getName()):
            if ClassicFSM.notify.getDebug():
                ClassicFSM.notify.debug("[%s]: already in state %s and no self transition" %
                                 (self.__name, aStateName))
            return 0
        else:
            msg = ("[%s]: no transition exists from %s to %s" %
                   (self.__name,
                    self.__currentState.getName(),
                    aStateName))
            if self.onUndefTransition == ClassicFSM.ERROR:
                ClassicFSM.notify.error(msg)
            elif self.onUndefTransition == ClassicFSM.DISALLOW_VERBOSE:
                ClassicFSM.notify.warning(msg)
            return 0


    def forceTransition(self, aStateName, enterArgList=[], exitArgList=[]):
        """ force a transition -- for debugging ONLY """
        self.request(aStateName, enterArgList, exitArgList, force=1)

    def conditional_request(self, aStateName, enterArgList=[], exitArgList=[]):
        """request(self, string)
        'if this transition is defined, do it'
        Attempt transition from currentState to given one, if it exists.
        Return true if transition exists to given state,
        false otherwise.  It is NOT an error/warning to attempt a cond_request
        if the transition doesn't exist.  This lets people be sloppy about
        ClassicFSM transitions, letting the same fn be used for different states
        that may not have the same out transitions.
        """
        
        assert(not self.__internalStateInFlux)
        if not self.__currentState:
            # Make this a warning for now
            ClassicFSM.notify.warning("[%s]: request: never entered initial state" %
                               (self.__name))
            self.__currentState = self.__initialState

        if isinstance(aStateName, types.StringType):
            aState = self.getStateNamed(aStateName)
        else:
            # Allow the caller to pass in a state in itself, not just
            # the name of a state.
            aState = aStateName
            aStateName = aState.getName()

        if aState == None:
            ClassicFSM.notify.error("[%s]: request: %s, no such state" %
                                (self.__name, aStateName))

        transitionDefined = (
            self.__currentState.isTransitionDefined(aStateName) or
            aStateName in [self.__currentState.getName(),
                           self.__finalState.getName()]
            )

        if transitionDefined:
            return self.request(aStateName, enterArgList, exitArgList)
        else:
            ClassicFSM.notify.debug("[%s]: condition_request: %s, transition doesnt exist" %
                             (self.__name, aStateName))
            return 0

    def view(self):
        import FSMInspector
        FSMInspector.FSMInspector(self)










