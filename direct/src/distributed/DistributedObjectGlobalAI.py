

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
        self.air.registerForChannel(self.doId)
        DistributedObjectAI.announceGenerate(self)
    
    def delete(self):
        self.air.unregisterForChannel(self.doId)
        DistributedObjectAI.delete(self)
