
"""State module: contains State class"""

from DirectObject import *


class State(DirectObject):

    """State class: """

    def __init__(self, name, enterFunc=None, exitFunc=None, transitions=[],
                 inspectorPos = []):
        """__init__(self, string, func, func, string[], inspectorPos = [])
        State constructor: takes name, enter func, exit func, and
        a list of states it can transition to."""
        self.setName(name)
        self.setEnterFunc(enterFunc)
        self.setExitFunc(exitFunc)
        self.setTransitions(transitions)
        self.setInspectorPos(inspectorPos)
        self.__FSMList = None


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

    def setEnterFunc(self, stateEnterFunc):
        """setEnterFunc(self, func)"""
        self.__enterFunc = stateEnterFunc

    def getExitFunc(self):
        """getExitFunc(self)"""
        return(self.__exitFunc)

    def setExitFunc(self, stateExitFunc):
        """setExitFunc(self, func)"""
        self.__exitFunc = stateExitFunc

    def getTransitions(self):
        """getTransitions(self)"""
        return(self.__transitions)

    def setTransitions(self, stateTransitions):
        """setTransitions(self, string[])"""
        self.__transitions = stateTransitions

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
        """setChildren(self, FSM[])
        Set the children to given list of FSMs"""
        self.__FSMList = FSMList

    def addChild(self, FSM):
        """addChild(self, FSM)
        Add the given FSM to list of child FSMs"""
        if (self.__FSMList == None):
            self.__FSMList = [FSM]
        else:
            self.__FSMList.append(FSM)

    def hasChildren(self):
        """hasChildren(self)
        Return true if state has child FSMs"""
        return(self.__FSMList != None)

    def __enterChildren(self):
        """__enterChildren(self)
        Enter all child FSMs"""
        if self.hasChildren():
            for fsm in self.__FSMList:
                fsm.request((fsm.getInitialState()).getName())

    def __exitChildren(self):
        """__exitChildren(self)
        Exit all child FSMs"""
        if self.hasChildren():
            for fsm in self.__FSMList:
                fsm.request((fsm.getFinalState()).getName())


    # basic State functionality
    
    def enter(self):
        """enter(self)
        Call the enter function for this state"""
        if (self.__enterFunc != None):
            apply(self.__enterFunc)
        
        #enter child FSMs
        self.__enterChildren()
        
    def exit(self):
        """exit(self)
        Call the exit function for this state"""
        #first exit child FSMs
        self.__exitChildren()

        #call exit function if it exists
        if (self.__exitFunc != None):
            apply(self.__exitFunc)
        
    def __str__(self):
        """__str__(self)"""
        return "State: name = %s, enter = %s, exit = %s, trans = %s, children = %s" %\
               (self.__name, self.__enterFunc, self.__exitFunc, self.__transitions, self.__FSMList)




            
        

    







