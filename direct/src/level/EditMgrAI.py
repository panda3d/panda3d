"""EditMgrAI module: contains the EditMgrAI class"""

import EditMgrBase
if __debug__:
    from PythonUtil import list2dict
    import EditorGlobals

class EditMgrAI(EditMgrBase.EditMgrBase):
    """This class handles AI-side editor-specific functionality"""
    if __debug__:
        def setRequestNewEntity(self, data):
            # pick an unused entId
            spec = self.level.levelSpec
            entIds = spec.getAllEntIds()
            entIdDict = list2dict(entIds)

            # dumb linear search for now
            # make this smarter (cache last-allocated id)
            # Note that this uses the ID range associated with the
            # AI's username, not the username of the user who requested
            # the new entity.
            for id in xrange(*EditorGlobals.getEntIdAllocRange()):
                if not id in entIdDict:
                    break
            else:
                self.notify.error('out of entIds')

            # OK, we've chosen an unused entId. Add the entId to the data
            # dict and do the insert
            data.update({'entId': id})
            self.level.setAttribChange(self.entId, 'insertEntity', data)

        def getSpecSaveEvent(self):
            return 'requestSave-%s' % self.level.levelId
        def setRequestSave(self, data):
            messenger.send(self.getSpecSaveEvent())
