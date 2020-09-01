"""State module: contains State class"""

__all__ = ['State']

from direct.directnotify.DirectNotifyGlobal import directNotify
from direct.showbase.DirectObject import DirectObject
import sys


class State(DirectObject):
    notify = directNotify.newCategory("State")

    # this 'constant' can be used to specify that the state
    # can transition to any other state
    Any = 'ANY'

    # Keep a list of State objects currently in memory for
    # Control-C-Control-V redefining. These are just weakrefs so they
    # should not cause any leaks.
    if __debug__:
        import weakref
        States = weakref.WeakKeyDictionary()

        @classmethod
        def replaceMethod(self, oldFunction, newFunction):
            import types
            count = 0
            for state in self.States:
                # Note: you can only replace methods currently
                enterFunc = state.getEnterFunc()
                exitFunc = state.getExitFunc()
                # print 'testing: ', state, enterFunc, exitFunc, oldFunction
                if type(enterFunc) == types.MethodType:
                    if enterFunc.__func__ == oldFunction:
                        # print 'found: ', enterFunc, oldFunction
                        if sys.version_info >= (3, 0):
                            state.setEnterFunc(types.MethodType(newFunction,
                                                                enterFunc.__self__))
                        else:
                            state.setEnterFunc(types.MethodType(newFunction,
                                                                enterFunc.__self__,
                                                                enterFunc.__self__.__class__))
                        count += 1
                if type(exitFunc) == types.MethodType:
                    if exitFunc.__func__ == oldFunction:
                        # print 'found: ', exitFunc, oldFunction
                        if sys.version_info >= (3, 0):
                            state.setExitFunc(types.MethodType(newFunction,
                                                               exitFunc.__self__))
                        else:
                            state.setExitFunc(types.MethodType(newFunction,
                                                               exitFunc.__self__,
                                                               exitFunc.__self__.__class__))
                        count += 1
            return count


    def __init__(self, name, enterFunc=None, exitFunc=None,
                 transitions=Any, inspectorPos = []):
        """__init__(self, string, func, func, string[], inspectorPos = [])
        State constructor: takes name, enter func, exit func, and
        a list of states it can transition to (or State.Any)."""
        self.__name = name
        self.__enterFunc = enterFunc
        self.__exitFunc = exitFunc
        self.__transitions = transitions
        self.__FSMList = []
        if __debug__:
            self.setInspectorPos(inspectorPos)
            # For redefining
            self.States[self] = 1

    # setters and getters

    def getName(self):
        return(self.__name)

    def setName(self, stateName):
        self.__name = stateName

    def getEnterFunc(self):
        return(self.__enterFunc)

    def setEnterFunc(self, stateEnterFunc):
        self.__enterFunc = stateEnterFunc

    def getExitFunc(self):
        return(self.__exitFunc)

    def setExitFunc(self, stateExitFunc):
        self.__exitFunc = stateExitFunc

    def transitionsToAny(self):
        """ returns true if State defines transitions to any other state """
        return self.__transitions is State.Any

    def getTransitions(self):
        """
        warning -- if the state transitions to any other state,
        returns an empty list (falsely implying that the state
        has no transitions)
        see State.transitionsToAny()
        """
        if self.transitionsToAny():
            return []
        return self.__transitions

    def isTransitionDefined(self, otherState):
        if self.transitionsToAny():
            return 1

        # if we're given a state object, get its name instead
        if type(otherState) != type(''):
            otherState = otherState.getName()
        return (otherState in self.__transitions)

    def setTransitions(self, stateTransitions):
        """setTransitions(self, string[])"""
        self.__transitions = stateTransitions

    def addTransition(self, transition):
        """addTransitions(self, string)"""
        if not self.transitionsToAny():
            self.__transitions.append(transition)
        else:
            State.notify.warning(
                'attempted to add transition %s to state that '
                'transitions to any state')

    if __debug__:
        def getInspectorPos(self):
            """getInspectorPos(self)"""
            return(self.__inspectorPos)

        def setInspectorPos(self, inspectorPos):
            """setInspectorPos(self, [x, y])"""
            self.__inspectorPos = inspectorPos

    # support for HFSMs

    def getChildren(self):
        """
        Return the list of child FSMs
        """
        return(self.__FSMList)

    def setChildren(self, FSMList):
        """setChildren(self, ClassicFSM[])
        Set the children to given list of FSMs
        """
        self.__FSMList = FSMList

    def addChild(self, ClassicFSM):
        """
        Add the given ClassicFSM to list of child FSMs
        """
        self.__FSMList.append(ClassicFSM)

    def removeChild(self, ClassicFSM):
        """
        Remove the given ClassicFSM from list of child FSMs
        """
        if ClassicFSM in self.__FSMList:
            self.__FSMList.remove(ClassicFSM)

    def hasChildren(self):
        """
        Return true if state has child FSMs
        """
        return len(self.__FSMList) > 0

    def __enterChildren(self, argList):
        """
        Enter all child FSMs
        """
        for fsm in self.__FSMList:
            # Check to see if the child fsm is already in a state
            # if it is, politely request the initial state

            if fsm.getCurrentState():
                # made this 'conditional_request()' instead of 'request()' to avoid warning when
                # loading minigames where rules->frameworkInit transition doesnt exist and you
                # don't want to add it since it results in hanging the game
                fsm.conditional_request((fsm.getInitialState()).getName())

            # If it has no current state, I assume this means it
            # has never entered the initial state, so enter it
            # explicitly
            else:
                fsm.enterInitialState()

    def __exitChildren(self, argList):
        """
        Exit all child FSMs
        """
        for fsm in self.__FSMList:
            fsm.request((fsm.getFinalState()).getName())


    # basic State functionality

    def enter(self, argList=[]):
        """
        Call the enter function for this state
        """
        # enter child FSMs first. It is assumed these have a start
        # state that is safe to enter
        self.__enterChildren(argList)

        if (self.__enterFunc != None):
            self.__enterFunc(*argList)

    def exit(self, argList=[]):
        """
        Call the exit function for this state
        """
        # first exit child FSMs
        self.__exitChildren(argList)

        # call exit function if it exists
        if (self.__exitFunc != None):
            self.__exitFunc(*argList)

    def __str__(self):
        return "State: name = %s, enter = %s, exit = %s, trans = %s, children = %s" %\
               (self.__name, self.__enterFunc, self.__exitFunc, self.__transitions, self.__FSMList)
