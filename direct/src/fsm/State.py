
"""State module: contains State class"""

from DirectObject import *
import types

# This gets set by a dconfig variable in ShowBase.py
# We cannot put a dconfig in here because ClassicFSM is not
# dependent on Panda
FsmRedefine = 0

# Map function pointers back into states so the Finder
# can find a function and swap a newly redefined function
# back into the original state
# The map is keyed off function pointers which map to
# a list of states that have that function pointer defined
# as their enter function (or exit function as the case may be)
EnterFuncRedefineMap = {}
ExitFuncRedefineMap = {}


def redefineEnterFunc(oldMethod, newFunction):
    import new
    if not FsmRedefine:
        return
    for method in EnterFuncRedefineMap.keys():
        if (type(method) == types.MethodType):
            function = method.im_func
        else:
            function = method
        #print ('function: ' + `function` + '\n' +
        #       'method: ' + `method` + '\n' +
        #       'oldMethod: ' + `oldMethod` + '\n' +
        #       'newFunction: ' + `newFunction` + '\n')
        if (function == oldMethod):
            newMethod = new.instancemethod(newFunction,
                                           method.im_self,
                                           method.im_class)
            stateList = EnterFuncRedefineMap[method]
            for state in stateList:
                state.setEnterFunc(newMethod)
            return 1
    return 0


def redefineExitFunc(oldMethod, newFunction):
    import new
    if not FsmRedefine:
        return
    for method in ExitFuncRedefineMap.keys():
        if (type(method) == types.MethodType):
            function = method.im_func
        else:
            function = method
        #print ('function: ' + `function` + '\n' +
        #       'method: ' + `method` + '\n' +
        #       'oldMethod: ' + `oldMethod` + '\n' +
        #       'newFunction: ' + `newFunction` + '\n')
        if (function == oldMethod):
            newMethod = new.instancemethod(newFunction,
                                           method.im_self,
                                           method.im_class)
            stateList = ExitFuncRedefineMap[method]
            for state in stateList:
                state.setExitFunc(newMethod)
            return 1
    return 0


class State(DirectObject):
    notify = directNotify.newCategory("State")

    # this 'constant' can be used to specify that the state
    # can transition to any other state
    Any = 'ANY'

    """State class: """

    def __init__(self, name, enterFunc=None, exitFunc=None,
                 transitions=Any, inspectorPos = []):
        """__init__(self, string, func, func, string[], inspectorPos = [])
        State constructor: takes name, enter func, exit func, and
        a list of states it can transition to (or State.Any)."""
        self.__enterFunc = None
        self.__exitFunc = None

        self.setName(name)
        self.setEnterFunc(enterFunc)
        self.setExitFunc(exitFunc)
        self.setTransitions(transitions)
        self.setInspectorPos(inspectorPos)
        self.__FSMList = []


    # setters and getters

    def getName(self):
        """getName(self)"""
        return(self.__name)

    def setName(self, stateName):
        """setName(self, string)"""
        self.__name = stateName

    def getEnterFunc(self):
        """getEnterFunc(self)"""
        return(self.__enterFunc)

    def redefineFunc(self, oldMethod, newMethod, map):
        if not FsmRedefine:
            return
        # Methods are allowed to be None
        if oldMethod is None:
            return
        if map.has_key(oldMethod):
            # Get the list of states for the old function
            stateList = map[oldMethod]
            # Remove this state from that list of states
            stateList.remove(self)
            # If the stateList is now empty, remove this entry altogether
            if not stateList:
                del(map[oldMethod])
        # Now add the new function, creating a starter state list
        # if there is not one already
        stateList = map.get(newMethod, [])
        stateList.append(self)
        map[newMethod] = stateList

    def setEnterFunc(self, stateEnterFunc):
        self.redefineFunc(self.__enterFunc, stateEnterFunc, EnterFuncRedefineMap)
        self.__enterFunc = stateEnterFunc

    def getExitFunc(self):
        """getExitFunc(self)"""
        return(self.__exitFunc)

    def setExitFunc(self, stateExitFunc):
        """setExitFunc(self, func)"""
        self.redefineFunc(self.__exitFunc, stateExitFunc, ExitFuncRedefineMap)
        self.__exitFunc = stateExitFunc

    def transitionsToAny(self):
        """ returns true if State defines transitions to any other state """
        return self.__transitions is State.Any

    def getTransitions(self):
        """getTransitions(self)
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

    def getInspectorPos(self):
        """getInspectorPos(self)"""
        return(self.__inspectorPos)

    def setInspectorPos(self, inspectorPos):
        """setInspectorPos(self, [x, y])"""
        self.__inspectorPos = inspectorPos


    # support for HFSMs

    def getChildren(self):
        """getChildren(self)
        Return the list of child FSMs"""
        return(self.__FSMList)

    def setChildren(self, FSMList):
        """setChildren(self, ClassicFSM[])
        Set the children to given list of FSMs"""
        self.__FSMList = FSMList

    def addChild(self, ClassicFSM):
        """addChild(self, ClassicFSM)
        Add the given ClassicFSM to list of child FSMs"""
        self.__FSMList.append(ClassicFSM)

    def removeChild(self, ClassicFSM):
        """removeChild(self, ClassicFSM)
        Remove the given ClassicFSM from list of child FSMs"""
        if ClassicFSM in self.__FSMList:
            self.__FSMList.remove(ClassicFSM)

    def hasChildren(self):
        """hasChildren(self)
        Return true if state has child FSMs"""
        return len(self.__FSMList) > 0

    def __enterChildren(self, argList):
        """__enterChildren(self, argList)
        Enter all child FSMs"""
        for fsm in self.__FSMList:
            # Check to see if the child fsm is already in a state
            # if it is, politely request the initial state

            if fsm.getCurrentState():
                # made this 'conditional_request()' instead of 'request()' to avoid warning when
                # loading minigames where rules->frameworkInit transition doesnt exist and you
                # dont want to add it since it results in hanging the game
                fsm.conditional_request((fsm.getInitialState()).getName())

            # If it has no current state, I assume this means it
            # has never entered the initial state, so enter it
            # explicitly
            else:
                fsm.enterInitialState()

    def __exitChildren(self, argList):
        """__exitChildren(self, argList)
        Exit all child FSMs"""
        for fsm in self.__FSMList:
            fsm.request((fsm.getFinalState()).getName())


    # basic State functionality

    def enter(self, argList=[]):
        """enter(self)
        Call the enter function for this state"""

        # enter child FSMs first. It is assumed these have a start
        # state that is safe to enter
        self.__enterChildren(argList)

        if (self.__enterFunc != None):
            apply(self.__enterFunc, argList)

    def exit(self, argList=[]):
        """exit(self)
        Call the exit function for this state"""
        # first exit child FSMs
        self.__exitChildren(argList)

        # call exit function if it exists
        if (self.__exitFunc != None):
            apply(self.__exitFunc, argList)

    def __str__(self):
        """__str__(self)"""
        return "State: name = %s, enter = %s, exit = %s, trans = %s, children = %s" %\
               (self.__name, self.__enterFunc, self.__exitFunc, self.__transitions, self.__FSMList)















