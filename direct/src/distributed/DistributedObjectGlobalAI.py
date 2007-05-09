
from DistributedObjectAI import DistributedObjectAI
from direct.directnotify.DirectNotifyGlobal import directNotify

if __debug__:
    notify = directNotify.newCategory('DistributedObjectGlobalAI')


class DistributedObjectGlobalAI(DistributedObjectAI):
    if __debug__:
        notify = notify

    doNotDeallocateChannel = 1
    isGlobalDistObj = 1

    def __init__(self, air): 
        DistributedObjectAI.__init__(self, air)

    def announceGenerate(self):
        DistributedObjectAI.announceGenerate(self)
        try:
            if not self.doNotListenToChannel:
                self.air.registerForChannel(self.doId)            
        except AttributeError:
                self.air.registerForChannel(self.doId)            
        return False        

    def delete(self):
       DistributedObjectAI.delete(self)
       try:
            if not self.doNotListenToChannel:
               self.air.unregisterForChannel(self.doId)
       except AttributeError:
           self.air.unregisterForChannel(self.doId)
        ## self.air.removeDOFromTables(self)
 