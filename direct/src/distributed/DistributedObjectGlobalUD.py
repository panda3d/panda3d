

from DistributedObjectUD import DistributedObjectUD
from direct.directnotify.DirectNotifyGlobal import directNotify

if __debug__:
    notify = directNotify.newCategory('DistributedObjectGlobalUD')


class DistributedObjectGlobalUD(DistributedObjectUD):
    if __debug__:
        notify = notify

    doNotDeallocateChannel = 1
    isGlobalDistObj = 1

    def __init__(self, air):
        DistributedObjectUD.__init__(self, air)
    
    def announceGenerate(self):
        self.air.registerForChannel(self.doId)
        DistributedObjectUD.announceGenerate(self)
    
    def delete(self):
        self.air.unregisterForChannel(self.doId)
        ## self.air.removeDOFromTables(self)
        DistributedObjectUD.delete(self)
