"""EditMgrAI module: contains the EditMgrAI class"""

import EditMgrBase
if __dev__:
    from PythonUtil import list2dict
    import EditorGlobals

class EditMgrAI(EditMgrBase.EditMgrBase):
    """This class handles AI-side editor-specific functionality"""
    if __dev__:
        def setRequestNewEntity(self, data):
            # pick an unused entId
            spec = self.level.levelSpec
            entIds = spec.getAllEntIds()
            entIdDict = list2dict(entIds)

            # Note that this uses the ID range associated with the
            # AI's username, not the username of the user who requested
            # the new entity.
            allocRange = EditorGlobals.getEntIdAllocRange()

            if not hasattr(self, 'lastAllocatedEntId'):
                self.lastAllocatedEntId = allocRange[0]

            idChosen = 0
            while not idChosen:
                # linear search for an unused entId starting with the
                # last-allocated id
                for id in xrange(self.lastAllocatedEntId, allocRange[1]):
                    print id
                    if not id in entIdDict:
                        idChosen = 1
                        break
                else:
                    # we ran off the end of the range.
                    if self.lastAllocatedEntId != allocRange[0]:
                        # if we started in the middle, try again from
                        # the beginning
                        self.lastAllocatedEntId = allocRange[0]
                    else:
                        # every entId is used!!
                        self.notify.error('out of entIds')

            # OK, we've chosen an unused entId. Add the entId to the data
            # dict and do the insert
            data.update({'entId': id})
            self.lastAllocatedEntId = id
            self.level.setAttribChange(self.entId, 'insertEntity', data)

            # clear out the attrib, it shouldn't be kept in the spec
            self.level.levelSpec.doSetAttrib(self.entId, 'requestNewEntity',
                                             None)

        def getSpecSaveEvent(self):
            return 'requestSave-%s' % self.level.levelId
        def setRequestSave(self, data):
            messenger.send(self.getSpecSaveEvent())
            # clear out the attrib, it shouldn't be kept in the spec
            self.level.levelSpec.doSetAttrib(self.entId, 'requestSave',
                                             None)
