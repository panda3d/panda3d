"""EditMgrAI module: contains the EditMgrAI class"""

import EditMgrBase
from PythonUtil import list2dict

class EditMgrAI(EditMgrBase.EditMgrBase):
    """This class handles AI-side editor-specific functionality"""
    def setRequestNewEntity(self, data):
        # pick an unused entId
        spec = self.level.levelSpec
        entIds = spec.getAllEntIds()
        entIdDict = list2dict(entIds)
        # dumb linear search for now
        id = 100
        while id in entIdDict:
            id += 1

        # OK, we've chosen an unused entId. Add the entId to the data
        # dict and do the insert
        data.update({'entId': id})
        self.level.setAttribChange(self.entId, 'insertEntity', data)
