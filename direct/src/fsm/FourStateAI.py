

import DirectNotifyGlobal
#import DistributedObjectAI
import FSM
import State
import Task


class FourStateAI:
    """
    Generic four state FSM base class.
    
    This is a mix-in class that expects that your derived class
    is a DistributedObjectAI.
    
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
        
        durations is a list of durations in seconds or None values.
            The list of duration values should be the same length
            as the list of state names and the lists correspond.
            For each state, after n seconds, the FSM will move to 
            the next state.  That does not happen for any duration
            values of None.
        
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
        off (and so is state 2 which is oposite of state 4 and therefore 
        oposite of 'on').
        """
        self.isOn = 0 # used in debugPrint()
        assert(self.debugPrint(
                "FourStateAI(names=%s, durations=%s)"
                %(names, durations)))
        self.doLaterTask = None
        self.stateIndex = 0
        assert len(names) == 5
        assert len(names) == len(durations)
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
        self.fsm = FSM.FSM('DistributedDoorEntity',
                           self.states.values(),
                           # Initial State
                           names[0],
                           # Final State
                           names[0],
                          )
        self.fsm.enterInitialState()

    def delete(self):
        assert(self.debugPrint("delete()"))
        if self.doLaterTask is not None:
            self.doLaterTask.remove()
            del self.doLaterTask
        del self.states
        del self.fsm
    
    def getInitialState(self):
        return self.stateIndex
    
    def setIsOn(self, isOn):
        assert(self.debugPrint("setIsOn(isOn=%s)"%(isOn,)))
        if self.isOn != isOn:
            self.isOn = isOn
            self.changedOnState()
    
    def getIsOn(self):
        assert(self.debugPrint("getIsOn() returning %s"%(self.isOn,)))
        return self.isOn

    def changedOnState(self):
        """
        Allow derived classes to overide this.
        """
        pass

    ##### states #####

    def switchToNextStateTask(self, task):
        assert(self.debugPrint("switchToState1Task()"))
        self.fsm.request(self.states[self.nextStateIndex])
        return Task.done

    def distributeStateChange(self):
        """
        This function is intentionaly simple so that derived classes 
        may easily alter the network message.
        """
        self.sendUpdate('setState', [self.stateIndex, globalClockDelta.getRealNetworkTime()])
    
    def enterStateN(self, isOn, stateIndex, nextStateIndex):
        assert(self.debugPrint("enterStateN(stateIndex=%s, nextStateIndex=%s)"%(
            stateIndex, nextStateIndex)))
        self.stateIndex = stateIndex
        self.nextStateIndex = nextStateIndex
        self.setIsOn(isOn)
        self.distributeStateChange()
        if self.durations[stateIndex] is not None:
            assert self.doLaterTask is None
            self.doLaterTask=taskMgr.doMethodLater(
                self.durations[stateIndex],
                self.switchToNextStateTask,
                "enterStateN-timer-%s"%id(self))
    
    def exitStateN(self):
        if self.doLaterTask:
            taskMgr.remove(self.doLaterTask)
            self.doLaterTask=None
    
    ##### state 0 #####
    
    def enterState0(self):
        assert(self.debugPrint("enter0()"))
        self.stateIndex = 0
        self.isOn = 0
    
    def exitState0(self):
        assert(self.debugPrint("exit0()"))
    
    ##### state 1 #####
    
    def enterState1(self):
        self.enterStateN(0, 1, 2)
    
    def exitState1(self):
        assert(self.debugPrint("exitState1()"))
        self.exitStateN()
    
    ##### state 2 #####
    
    def enterState2(self):
        self.enterStateN(0, 2, 3)
    
    def exitState2(self):
        assert(self.debugPrint("exitState2()"))
        self.exitStateN()
    
    ##### state 3 #####
    
    def enterState3(self):
        self.enterStateN(0, 3, 4)
    
    def exitState3(self):
        assert(self.debugPrint("exitState3()"))
        self.exitStateN()
    
    ##### state 4 #####
    
    def enterState4(self):
        self.enterStateN(1, 4, 1)
    
    def exitState4(self):
        assert(self.debugPrint("exitState4()"))
        self.exitStateN()
    
    if __debug__:
        def debugPrint(self, message):
            """for debugging"""
            return self.notify.debug("%d (%d) %s"%(
                    id(self), self.isOn, message))

