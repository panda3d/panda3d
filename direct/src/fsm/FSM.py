"""Finite State Machine module: contains the FSM class"""

from DirectObject import *

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


        # Enter the initial state.
        # It is assumed that the initial state takes no arguments.
        self.__currentState = self.__initialState
        #self.__enter(self.__initialState)

    def enterInitialState(self):
        self.__enter(self.__initialState)
        
    def __str__(self):
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
        FSM.notify.warning("getStateNamed: " + str(stateName) + " no such state")


    # basic FSM functionality
    
    def __exitCurrent(self, argList):
        """__exitCurrent(self)
        Exit the current state"""
        FSM.notify.debug("exiting %s" % self.__currentState.getName())
        self.__currentState.exit(argList)
        messenger.send(self.getName() + '_' +
                       self.__currentState.getName() + '_exited')
        self.__currentState = None
                    
    def __enter(self, aState, argList=[]):
        """__enter(self, State)
        Enter a given state, if it exists"""
        if (aState in self.__states):
            FSM.notify.debug("entering %s" % aState.getName())
            self.__currentState = aState
            messenger.send(self.getName() + '_' +
                           aState.getName() + '_entered')
            aState.enter(argList)
        else:
            FSM.notify.error("enter: no such state")

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
        if (aStateName in self.__currentState.getTransitions()):
            self.__transition(self.getStateNamed(aStateName),
                              enterArgList,
                              exitArgList)
            return 1
        # We can implicitly always transition to our final state.
        elif (aStateName == self.__finalState.getName()):
            FSM.notify.debug("implicit transition to final state: %s" % aStateName)
            self.__transition(self.getStateNamed(aStateName),
                              enterArgList,
                              exitArgList)
            return 1
        else:
            FSM.notify.debug("no transition exists to %s" % aStateName)
            return 0











