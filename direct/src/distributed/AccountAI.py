"""
Account module: stub to fulfill the Account toon.dc Distributed Class
This is a class Roger needs for the server to be able to display these values
appropriately in the db web interface.
"""

import DistributedObjectAI

class AccountAI(DistributedObjectAI.DistributedObjectAI):
    def setPirate(self, slot, avatarId):
        if (not hasattr(self,'pirateAvatars')):
            self.pirateAvatars = [0,0,0,0,0,0]
        self.pirateAvatars[slot] = avatarId
        self.sendUpdate('pirateAvatars', self.pirateAvatars)
