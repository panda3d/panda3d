"""DistributedObject module: contains the DistributedObject class"""

from PandaObject import *

class DistributedObject(PandaObject):
    """Distributed Object class:"""
    def __init__(self, cr):
        try:
            self.DistributedObject_initialized
        except:
            self.DistributedObject_initialized = 1
            self.cr = cr
            # A few objects will set neverDisable to 1... Examples are localToon, and anything
            # that lives in the UberZone. This keeps them from being disabled when you change
            # zones, even to the quiet zone.
            self.setNeverDisable(0)
        return None

    def setNeverDisable(self, bool):
        assert((bool == 1) or (bool == 0))
        self.neverDisable = bool
        return None

    def getNeverDisable(self):
        return self.neverDisable

    def disable(self):
        """disable(self)
        Inheritors should redefine this to take appropriate action on disable
        """
        pass

    def delete(self):
        """delete(self)
        Inheritors should redefine this to take appropriate action on delete
        """
        pass

    def generate(self):
        """generate(self)
        Inheritors should redefine this to take appropriate action on generate
        """
        pass
    
    def getDoId(self):
        """getDoId(self)
        Return the distributed object id
        """
        return self.doId
    
    def updateRequiredFields(self, cdc, di):
        for i in cdc.allRequiredCDU:
            i.updateField(cdc, self, di)

    def updateRequiredOtherFields(self, cdc, di):
        # First, update the required fields
        for i in cdc.allRequiredCDU:
            i.updateField(cdc, self, di)
        # Determine how many other fields there are
        numberOfOtherFields = di.getArg(STUint16)
        # Update each of the other fields
        for i in range(numberOfOtherFields):
            cdc.updateField(self, di)
        return None

    def sendUpdate(self, fieldName, args):
        self.cr.sendUpdate(self, fieldName, args)

    def taskName(self, taskString):
        return (taskString + "-" + str(self.getDoId()))
    


