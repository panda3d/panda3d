

from DistributedObjectUD import DistributedObjectUD
from direct.directnotify.DirectNotifyGlobal import directNotify


class DistributedObjectGlobalUD(DistributedObjectUD):
    notify = directNotify.newCategory('DistributedObjectGlobalUD')

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
