

import DirectNotifyGlobal
#import DistributedObject
import FSM
import State
import Task


class FourState:
    """
    Generic four state FSM base class.
    
    This is a mix-in class that expects that your derived class
    is a DistributedObject.
    
    Inherit from FourStateFSM and pass in your states.  Two of 
    the states should be oposites of each other and the other 
    two should be the transition states between the first two.
    E.g.
    
                    +--------+
                 -->| closed | --
                |   +--------+   |
                |                |
                |                v
          +---------+       +---------+
          | closing |<----->| opening |
          +---------+       +---------+
                ^                |
                |                |
                |    +------+    |
                 ----| open |<---
                     +------+
    
    There is a fifth off state, but that is an implementation
    detail (and that's why it's not called a five state FSM).
    
    I found that this pattern repeated in several things I was
    working on, so this base class was created.
    """
    if __debug__:
        notify = DirectNotifyGlobal.directNotify.newCategory(
                'FourStateFSM')

    def __init__(self, names, durations = [0, 1, None, 1, 1]):
        """
        names is a list of state names
        
        E.g.
            ['off', 'opening', 'open', 'closing', 'closed', ]
        
        e.g. 2:
            ['off', 'locking', 'locked', 'unlocking', 'unlocked', ]
        
        e.g. 3:
            ['off', 'deactivating', 'deactive', 'activating', 'activated', ]
        
        More Details
        
        Here is a diagram showing the where the names from the list
        are used:

            +---------+
            | 0 (off) |----> (any other state and vice versa).
            +---------+

                       +--------+
                    -->| 4 (on) |---
                   |   +--------+   |
                   |                |
                   |                v
             +---------+       +---------+
             | 3 (off) |<----->| 1 (off) |
             +---------+       +---------+
                   ^                |
                   |                |
                   |  +---------+   |
                    --| 2 (off) |<--
                      +---------+

        Each states also has an associated on or off value.  The only
        state that is 'on' is state 4.  So, the transition states
        between off and on (states 1 and 3) are also considered 
        off (and so is state 2 which is oposite of 4 and therefore 
        oposite of 'on').
        """
        assert(self.debugPrint("FourState(names=%s)"%(names)))
        self.doLaterTask = None
        self.names = names
        self.durations = durations
        self.states = {
            0: State.State(names[0],
                           self.enterState0,
                           self.exitState0,
                           [names[1],
                           names[2],
                           names[3],
                           names[4]]),
            1: State.State(names[1],
                           self.enterState1,
                           self.exitState1,
                           [names[2], names[3]]),
            2: State.State(names[2],
                           self.enterState2,
                           self.exitState2,
                           [names[3]]),
            3: State.State(names[3],
                           self.enterState3,
                           self.exitState3,
                           [names[4], names[1]]),
            4: State.State(names[4],
                           self.enterState4,
                           self.exitState4,
                           [names[1]]),
            }
        self.stateIndex = 0
        self.fsm = FSM.FSM('DistributedDoorEntity',
                           self.states.values(),
                           # Initial State
                           names[0],
                           # Final State
                           names[0],
                          )
        self.fsm.enterInitialState()
    
    #def setIsOn(self, isOn):
    #    assert(self.debugPrint("setIsOn(isOn=%s)"%(isOn,)))
    #    pass
    
    #def getIsOn(self):
    #    assert(self.debugPrint("getIsOn() returning %s"%(self.isOn,)))
    #    return self.stateIndex==4

    def changedOnState(self, isOn):
        """
        Allow derived classes to overide this.
        """
        assert(self.debugPrint("changedOnState(isOn=%s)"%(isOn,)))
    
    ##### state 0 #####
    
    def enterState0(self):
        assert(self.debugPrint("enter0()"))
        self.stateIndex = 0
    
    def exitState0(self):
        assert(self.debugPrint("exit0()"))
        self.changedOnState(0)
    
    ##### state 1 #####
    
    def enterState1(self):
        assert(self.debugPrint("enterState1()"))
        self.stateIndex = 1
    
    def exitState1(self):
        assert(self.debugPrint("exitState1()"))
    
    ##### state 2 #####
    
    def enterState2(self):
        assert(self.debugPrint("enterState2()"))
        self.stateIndex = 2
    
    def exitState2(self):
        assert(self.debugPrint("exitState2()"))
    
    ##### state 3 #####
    
    def enterState3(self):
        assert(self.debugPrint("enterState3()"))
        self.stateIndex = 2
    
    def exitState3(self):
        assert(self.debugPrint("exitState3()"))
    
    ##### state 4 #####
    
    def enterState4(self):
        assert(self.debugPrint("enterState4()"))
        self.stateIndex = 4
        self.changedOnState(1)
    
    def exitState4(self):
        assert(self.debugPrint("exitState4()"))
        self.changedOnState(0)
    
    if __debug__:
        def debugPrint(self, message):
            """for debugging"""
            return self.notify.debug("%d (%d) %s"%(
                    id(self), self.stateIndex==4, message))

