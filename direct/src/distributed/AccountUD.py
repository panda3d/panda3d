"""
Account module: stub to fulfill the Account toon.dc Distributed Class
This is a class Roger needs for the server to be able to display these values
appropriately in the db web interface.
"""

from direct.directnotify import DirectNotifyGlobal
import DistributedObjectAI

class AccountUD(DistributedObjectAI.DistributedObjectAI):
    if __debug__:
        notify = DirectNotifyGlobal.directNotify.newCategory('AccountUD')

    pirateAvatars = [0,0,0,0,0,0]

    def __init__(self, air):
        assert air
        DistributedObjectAI.DistributedObjectAI.__init__(self, air)

    def setPirate(self, slot, avatarId):
        assert self.notify.debugCall()
        self.pirateAvatars[slot] = avatarId
        assert self.air
        self.sendUpdate('pirateAvatars', self.pirateAvatars)

    def getPirate(self, slot):
        assert self.notify.debugCall()
        return self.pirateAvatars[slot]

    def getSlotLimit(self):
        assert self.notify.debugCall()
        return 6

    def may(self, perm):
        """
        Ask whether the account has permission to <string>.
        """
        assert self.notify.debugCall()
        return 1
