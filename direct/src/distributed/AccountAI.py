"""
Account module: stub to fulfill the Account toon.dc Distributed Class
This is a class Roger needs for the server to be able to display these values
appropriately in the db web interface.
"""

import DistributedObjectAI

class AccountAI(DistributedObjectAI.DistributedObjectAI):
    pirateAvatars = [0,0,0,0,0,0]

    def setPirate(self, slot, avatarId):
        assert(0) # Ask AccountUD to setPirate

    def getPirate(self, slot):
        return self.pirateAvatars[slot]

    def getSlotLimit(self):
        return 6

    def may(self, perm):
        """
        Ask whether the account has permission to <string>.
        """
        return 1
