"""LogicGateAI.py: contains the OrLoEntity class"""


import PandaObject
import DirectNotifyGlobal
import Entity


# Logic Gates:
#
#  and: 0 0 = 0     or: 0 0 = 0    xor: 0 0 = 0
#       0 1 = 0         0 1 = 1         0 1 = 1
#       1 0 = 0         1 0 = 1         1 0 = 1
#       1 1 = 1         1 1 = 1         1 1 = 0
#
# nand: 0 0 = 1    nor: 0 0 = 1   xnor: 0 0 = 1
#       0 1 = 1         0 1 = 0         0 1 = 0
#       1 0 = 1         1 0 = 0         1 0 = 0
#       1 1 = 0         1 1 = 0         1 1 = 1
#
# In the following:
#   1: send a true message
#   0: send a false message
#   -: don't send a message
#
#   a b  and  or  xor  nand  nor  xnor
#  (0 0)  (0) (0)  (0)   (1)  (1)   (1)  <--- initial state
#   1 0    -   1    1     -    0     0
#   0 0    -   0    0     -    1     1
#   1 0    -   1    1     -    0     0
#   1 1    1   -    0     0    -     1
#   0 1    0   -    1     1    -     0
#   1 1    1   -    0     0    -     1
#   0 1    0   -    1     1    -     0
#   0 0    -   0    0     -    1     1

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
        self.input1 = None
        self.input2 = None
        Entity.Entity.__init__(self, level, entId)
        self.initializeEntity()
        self.setLogicType(self.logicType)
        self.setInput_input1_bool(self.input_input1_bool)
        self.setInput_input2_bool(self.input_input2_bool)

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
    
    def setInput_input1_bool(self, event):
        assert(self.debugPrint("setInput_input1_bool(event=%s)"%(event,)))
        if self.input1:
            self.ignore(self.input1)
        self.input1 = "switch-%s"%(event,)
        if self.input1:
            self.accept(self.input1, self.setIsInput1)
    
    def setInput_input2_bool(self, event):
        assert(self.debugPrint("setInput_input2_bool(event=%s)"%(event,)))
        if self.input2:
            self.ignore(self.input2)
        self.input2 = "switch-%s"%(event,)
        if self.input2:
            self.accept(self.input2, self.setIsInput2)
    
    def getName(self):
        #return "logicGate-%s"%(self.entId,)
        return "switch-%s"%(self.entId,)
