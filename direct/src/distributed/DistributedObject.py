"""DistributedObject module: contains the DistributedObject class"""

from PandaObject import *

class DistributedObject(PandaObject):
    """Distributed Object class:"""
    def __init__(self):
        try:
            self.DistributedObject_initialized
        except:
            self.DistributedObject_initialized = 1
        return None
    
    def getDoId(self):
        """getDoId(self)
        Return the distributed object id
        """
        return self.doId
    
    def updateRequiredFields(self, cdc, di):
        for i in cdc.allRequiredCDU:
            i.updateField(cdc, self, di)

    def sendUpdate(self, fieldName, args):
        cr.sendupdate(self, fieldName, args)

    def taskName(self, taskString):
        return (taskString + "-" + str(self.getDoId))
    


