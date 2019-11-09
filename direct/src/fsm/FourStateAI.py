"""Contains the FourStateAI class.  See also :mod:`.FourState`."""

__all__ = ['FourStateAI']

from direct.directnotify import DirectNotifyGlobal
#import DistributedObjectAI
from . import ClassicFSM
from . import State
from direct.task import Task


class FourStateAI:
    """
    Generic four state ClassicFSM base class.

    This is a mix-in class that expects that your derived class
    is a DistributedObjectAI.

    Inherit from FourStateFSM and pass in your states.  Two of
    the states should be oposites of each other and the other
    two should be the transition states between the first two.
    E.g::

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
    detail (and that's why it's not called a five state ClassicFSM).

    I found that this pattern repeated in several things I was
    working on, so this base class was created.
    """
    notify = DirectNotifyGlobal.directNotify.newCategory('FourStateAI')

    def __init__(self, names, durations = [0, 1, None, 1, 1]):
        """
        Names is a list of state names.  Some examples are::

            ['off', 'opening', 'open', 'closing', 'closed',]

            ['off', 'locking', 'locked', 'unlocking', 'unlocked',]

            ['off', 'deactivating', 'deactive', 'activating', 'activated',]

        durations is a list of durations in seconds or None values.
        The list of duration values should be the same length
        as the list of state names and the lists correspond.
        For each state, after n seconds, the ClassicFSM will move to
        the next state.  That does not happen for any duration
        values of None.

        .. rubric:: More Details

        Here is a diagram showing the where the names from the list
        are used::

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
        self.stateIndex = 0
        assert self.__debugPrint(
                "FourStateAI(names=%s, durations=%s)"
                %(names, durations))
        self.doLaterTask = None
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
        self.fsm = ClassicFSM.ClassicFSM('FourState',
                           list(self.states.values()),
                           # Initial State
                           names[0],
                           # Final State
                           names[0],
                          )
        self.fsm.enterInitialState()

    def delete(self):
        assert self.__debugPrint("delete()")
        if self.doLaterTask is not None:
            self.doLaterTask.remove()
            del self.doLaterTask
        del self.states
        del self.fsm

    def getState(self):
        assert self.__debugPrint("getState() returning %s"%(self.stateIndex,))
        return [self.stateIndex]

    def sendState(self):
        assert self.__debugPrint("sendState()")
        self.sendUpdate('setState', self.getState())

    def setIsOn(self, isOn):
        assert self.__debugPrint("setIsOn(isOn=%s)"%(isOn,))
        if isOn:
            if self.stateIndex != 4:
                # ...if it's not On; request turning on:
                self.fsm.request(self.states[3])
        else:
            if self.stateIndex != 2:
                # ...if it's not Off; request turning off:
                self.fsm.request(self.states[1])
        #if isOn:
        #    nextState = (4, 3, 3, 4, None)[self.stateIndex]
        #else:
        #    nextState = (2, 2, None, 1, 1)[self.stateIndex]
        #if nextState is not None:
        #    self.fsm.request(self.states[nextState])

    def isOn(self):
        assert self.__debugPrint("isOn() returning %s (stateIndex=%s)"%(self.stateIndex==4, self.stateIndex))
        return self.stateIndex==4

    def changedOnState(self, isOn):
        """
        Allow derived classes to overide this.
        The self.isOn value has toggled.  Call getIsOn() to
        get the current state.
        """
        assert self.__debugPrint("changedOnState(isOn=%s)"%(isOn,))

    ##### states #####

    def switchToNextStateTask(self, task):
        assert self.__debugPrint("switchToNextStateTask()")
        self.fsm.request(self.states[self.nextStateIndex])
        return Task.done

    def distributeStateChange(self):
        """
        This function is intentionaly simple so that derived classes
        may easily alter the network message.
        """
        assert self.__debugPrint("distributeStateChange()")
        self.sendState()

    def enterStateN(self, stateIndex, nextStateIndex):
        assert self.__debugPrint(
            "enterStateN(stateIndex=%s, nextStateIndex=%s)"%
            (stateIndex, nextStateIndex))
        self.stateIndex = stateIndex
        self.nextStateIndex = nextStateIndex
        self.distributeStateChange()
        if self.durations[stateIndex] is not None:
            assert self.doLaterTask is None
            self.doLaterTask=taskMgr.doMethodLater(
                self.durations[stateIndex],
                self.switchToNextStateTask,
                "enterStateN-timer-%s"%id(self))

    def exitStateN(self):
        assert self.__debugPrint("exitStateN()")
        if self.doLaterTask:
            taskMgr.remove(self.doLaterTask)
            self.doLaterTask=None

    ##### state 0 #####

    def enterState0(self):
        assert self.__debugPrint("enter0()")
        self.enterStateN(0, 0)

    def exitState0(self):
        assert self.__debugPrint("exit0()")

    ##### state 1 #####

    def enterState1(self):
        #assert self.__debugPrint("enterState1()")
        self.enterStateN(1, 2)

    def exitState1(self):
        assert self.__debugPrint("exitState1()")
        self.exitStateN()

    ##### state 2 #####

    def enterState2(self):
        #assert self.__debugPrint("enterState2()")
        self.enterStateN(2, 3)

    def exitState2(self):
        assert self.__debugPrint("exitState2()")
        self.exitStateN()

    ##### state 3 #####

    def enterState3(self):
        #assert self.__debugPrint("enterState3()")
        self.enterStateN(3, 4)

    def exitState3(self):
        assert self.__debugPrint("exitState3()")
        self.exitStateN()

    ##### state 4 #####

    def enterState4(self):
        assert self.__debugPrint("enterState4()")
        self.enterStateN(4, 1)
        self.changedOnState(1)

    def exitState4(self):
        assert self.__debugPrint("exitState4()")
        self.exitStateN()
        self.changedOnState(0)

    if __debug__:
        def __debugPrint(self, message):
            """for debugging"""
            return self.notify.debug("%d (%d) %s"%(
                    id(self), self.stateIndex==4, message))

