"""Finite State Machine module: contains the FSM class"""

from DirectObject import *
import types

class FSM(DirectObject):
    """FSM class: Finite State Machine class"""

    # create FSM DirectNotify category
    notify = directNotify.newCategory("FSM")

    # special methods
    
    def __init__(self, name, states=[], initialStateName=None,
                 finalStateName=None):
        """__init__(self, string, State[], string, string)

        FSM constructor: takes name, list of states, initial state and
        final state as:

        fsm = FSM.FSM('stopLight',
          [ State.State('red', enterRed, exitRed, ['green']),
            State.State('yellow', enterYellow, exitYellow, ['red']),
            State.State('green', enterGreen, exitGreen, ['yellow']) ],
          'red',
          'red')

        """

        self.setName(name)
        self.setStates(states)
        self.setInitialState(initialStateName)
        self.setFinalState(finalStateName)

        # Flag to see if we are inspecting
        self.inspecting = 0

        # We do not enter the initial state to separate
        # construction from activation
        self.__currentState = None
        return None

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
            str = ("FSM " + self.getName() + ' in state "' +
                   currentState.getName() + '"')
        else:
            str = ("FSM " + self.getName() + ' not in any state')
        return str

    def enterInitialState(self):
        self.__enter(self.__initialState)
        return None
        
    # Jesse decided that simpler was better with the __str__ function
    def __str_not__(self):
        """__str__(self)"""
        return "FSM: name = %s \n states = %s \n initial = %s \n final = %s \n current = %s" % (self.__name, self.__states, self.__initialState, self.__finalState, self.__currentState)


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
        FSM.notify.warning("[%s] : getStateNamed: %s, no such state" %
                           (self.__name, str(stateName)))


    # basic FSM functionality
    
    def __exitCurrent(self, argList):
        """__exitCurrent(self)
        Exit the current state"""
        FSM.notify.debug("[%s]: exiting %s" % (self.__name,
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
        if (aState in self.__states):
            FSM.notify.debug("[%s]: entering %s" % (self.__name,
                                                    aState.getName()))
            self.__currentState = aState
            # Only send the state change event if we are inspecting it
            # If this event turns out to be generally useful, we can
            # turn it on all the time, but for now nobody else is using it
            if self.inspecting:
                messenger.send(self.getName() + '_' +
                               aState.getName() + '_entered')
            aState.enter(argList)
        else:
            FSM.notify.error("[%s]: enter: no such state" % (self.__name))

    def __transition(self, aState, enterArgList=[], exitArgList=[]):
        """__transition(self, State, enterArgList, exitArgList)
        Exit currentState and enter given one"""
        self.__exitCurrent(exitArgList)
        self.__enter(aState, enterArgList)
        
    def request(self, aStateName, enterArgList=[], exitArgList=[]):
        """request(self, string)
        Attempt transition from currentState to given one.
        Return true is transition exists to given state,
        false otherwise. 
        """

        if not self.__currentState:
            # Make this a warning for now
            FSM.notify.warning("[%s]: request: never entered initial state" %
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
            FSM.notify.error("[%s]: request: %s, no such state" %
                             (self.__name, aStateName))

        if (aStateName in self.__currentState.getTransitions()):
            self.__transition(aState,
                              enterArgList,
                              exitArgList)
            return 1
        # We can implicitly always transition to our final state.
        elif (aStateName == self.__finalState.getName()):
            if (self.__currentState == self.__finalState):
                # Do not do the transition if we are already in the final state
                FSM.notify.debug("[%s]: already in final state: %s" %
                                 (self.__name, aStateName))
                return 1
            else:
                # Force a transition to allow for cleanup
                FSM.notify.debug("[%s]: implicit transition to final state: %s" %
                                 (self.__name, aStateName))
                self.__transition(aState,
                                  enterArgList,
                                  exitArgList)
                return 1
        else:
            FSM.notify.warning("[%s]: no transition exists from %s to %s" %
                               (self.__name,
                                self.__currentState.getName(),
                                aStateName))
            return 0











