"""
LogicGateAI.py

    Logic Gates:

         and: 0 0 = 0     or: 0 0 = 0    xor: 0 0 = 0
              0 1 = 0         0 1 = 1         0 1 = 1
              1 0 = 0         1 0 = 1         1 0 = 1
              1 1 = 1         1 1 = 1         1 1 = 0

        nand: 0 0 = 1    nor: 0 0 = 1   xnor: 0 0 = 1
              0 1 = 1         0 1 = 0         0 1 = 0
              1 0 = 1         1 0 = 0         1 0 = 0
              1 1 = 0         1 1 = 0         1 1 = 1

    In the following:
        1: send a true message
        0: send a false message
        -: don't send a message

        a b  and  or  xor  nand  nor  xnor
       (0 0)  (0) (0)  (0)   (1)  (1)   (1)  <--- initial state
        1 0    -   1    1     -    0     0
        0 0    -   0    0     -    1     1
        1 0    -   1    1     -    0     0
        1 1    1   -    0     0    -     1
        0 1    0   -    1     1    -     0
        1 1    1   -    0     0    -     1
        0 1    0   -    1     1    -     0
        0 0    -   0    0     -    1     1
"""

import PandaObject
import DirectNotifyGlobal
import Entity


def andTest(self, a, b):
    assert(self.debugPrint("andTest(a=%s, b=%s)"%(a, b)))
    if b:
        messenger.send(self.getName(), [a])

def orTest(self, a, b):
    assert(self.debugPrint("orTest(a=%s, b=%s)"%(a, b)))
    if not b:
        messenger.send(self.getName(), [a])
    # else: ...we already sent the messege when b was set.

def xorTest(self, a, b):
    assert(self.debugPrint("xorTest(a=%s, b=%s)"%(a, b)))
    messenger.send(self.getName(), [(not (a and b)) and (a or b)])

def nandTest(self, a, b):
    assert(self.debugPrint("nandTest(a=%s, b=%s)"%(a, b)))
    if b:
        messenger.send(self.getName(), [not (a and b)])

def norTest(self, a, b):
    assert(self.debugPrint("norTest(a=%s, b=%s)"%(a, b)))
    if not b:
        messenger.send(self.getName(), [not (a or b)])
    # else: ...we already sent the messege when b was set.

def xnorTest(self, a, b):
    assert(self.debugPrint("xnorTest(a=%s, b=%s)"%(a, b)))
    messenger.send(self.getName(), [(a and b) or (not (a or b))])


class LogicGateAI(Entity.Entity, PandaObject.PandaObject):
    if __debug__:
        notify = DirectNotifyGlobal.directNotify.newCategory(
                'LogicGateAI')
    logicTests={
        "and": andTest,
        "or": orTest,
        "xor": xorTest,
        "nand": nandTest,
        "nor": norTest,
        "xnor": xnorTest,
    }

    def __init__(self, level, entId):
        """entId: """
        assert(self.debugPrint(
                "LogicGateAI(entId=%s)"
                %(entId)))
        self.input1Event = None
        self.input2Event = None
        PandaObject.PandaObject.__init__(self)
        Entity.Entity.__init__(self, level, entId)
        self.setLogicType(self.logicType)
        self.setIsInput1(self.isInput1)
        self.setIsInput2(self.isInput2)
        self.setInput1Event(self.input1Event)
        self.setInput2Event(self.input2Event)

    def destroy(self):
        assert(self.debugPrint("destroy()"))
        self.ignore(self.input1)
        self.input1 = None
        self.ignore(self.input2)
        self.input2 = None
        Entity.Entity.destroy(self)
        PandaObject.PandaObject.destroy(self)
    
    def setLogicType(self, logicType):
        assert(self.debugPrint("setLogicType(logicType=%s)"%(logicType,)))
        self.logicType=logicType
        assert self.logicTests[logicType]
        self.logicTest=self.logicTests[logicType]
    
    def setIsInput1(self, isTrue):
        assert(self.debugPrint("setIsInput1(isTrue=%s)"%(isTrue,)))
        if 1 or (not isTrue) != (not self.input1):
            # ...the logical state of self.input1 has changed.
            self.isInput1=isTrue
            self.logicTest(self, isTrue, self.isInput2)
    
    def setIsInput2(self, isTrue):
        assert(self.debugPrint("setIsInput1(isTrue=%s)"%(isTrue,)))
        if 1 or (not isTrue) != (not self.input2):
            # ...the logical state of self.input2 has changed.
            self.isInput2=isTrue
            self.logicTest(self, isTrue, self.isInput1)
    
    def setInput1Event(self, event):
        assert(self.debugPrint("setInput1Event(event=%s)"%(event,)))
        if self.input1Event:
            self.ignore(self.input1Event)
        self.input1Event = "switch-%s"%(event,)
        if self.input1Event:
            self.accept(self.input1Event, self.setIsInput1)
    
    def setInput2Event(self, event):
        assert(self.debugPrint("setInput2Event(event=%s)"%(event,)))
        if self.input2Event:
            self.ignore(self.input2Event)
        self.input2Event = "switch-%s"%(event,)
        if self.input2Event:
            self.accept(self.input2Event, self.setIsInput2)
    
    def getName(self):
        #return "logicGate-%s"%(self.entId,)
        return "switch-%s"%(self.entId,)
