"""XorLoEntityAI.py: contains the XorLoEntity class"""


import PandaObject
import DirectNotifyGlobal
import Entity


class XorLoEntityAI(Entity.Entity, PandaObject.PandaObject):
    if __debug__:
        notify = DirectNotifyGlobal.directNotify.newCategory(
                'XorLoEntityAI')

    def __init__(self, air, levelDoId, entId, zoneId=None):
        """entId: """
        assert(self.debugPrint(
                "XorLoEntityAI(air=%s, levelDoId=%s, entId=%s, zoneId=%s)"
                %("the air", levelDoId, entId, zoneId)))
        self.input1 = None
        self.input2 = None
        self.levelDoId = levelDoId
        level = air.doId2do[self.levelDoId]
        Entity.Entity.__init__(self, level, entId)
        self.initializeEntity()
        self.setInput_input1_bool(self.input_input1_bool)
        self.setInput_input2_bool(self.input_input2_bool)
    
    def setIsInput1(self, isTrue):
        assert(self.debugPrint("setIsInput1(isTrue=%s)"%(isTrue,)))
        self.isInput1=isTrue
        messenger.send(self.getName(), [(not (isTrue and self.isInput2)) and (isTrue or self.isInput2)])
    
    def setIsInput2(self, isTrue):
        assert(self.debugPrint("setIsInput1(isTrue=%s)"%(isTrue,)))
        self.isInput2=isTrue
        messenger.send(self.getName(), [(not (isTrue and self.isInput2)) and (isTrue or self.isInput2)])
    
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
        #return "xorLoEntity-%s"%(self.entId,)
        return "switch-%s"%(self.entId,)
