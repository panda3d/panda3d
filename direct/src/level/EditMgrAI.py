"""EditMgrAI module: contains the EditMgrAI class"""

import EditMgrBase
from PythonUtil import list2dict, uniqueElements

if __debug__:
    EntIdRange = 5000
    username2entIdBase = {
        'darren': 1*EntIdRange,
        'samir':  2*EntIdRange,
        'skyler': 3*EntIdRange,
        'joe':    4*EntIdRange,
        'mark':   5*EntIdRange,
        }
    assert uniqueElements(username2entIdBase.values())

    UndefinedUsername = 'UNDEFINED_USERNAME'
    UsernameConfigVar = 'factory-edit-username'
    username = config.GetString(UsernameConfigVar, UndefinedUsername)

class EditMgrAI(EditMgrBase.EditMgrBase):
    """This class handles AI-side editor-specific functionality"""
    if __debug__:
        def setRequestNewEntity(self, data):
            # pick an unused entId
            spec = self.level.levelSpec
            entIds = spec.getAllEntIds()
            entIdDict = list2dict(entIds)

            # Feel free to add your name to the table if it's not in there yet
            if username == UndefinedUsername:
                self.setRequestSave(None)
                self.notify.error("you must config '%s'" % UsernameConfigVar)
            if username not in username2entIdBase:
                self.notify.error("unknown editor username '%s'" % username)

            # dumb linear search for now
            # make this smarter (cache last-allocated id)
            baseId = username2entIdBase[username]
            id = baseId
            for id in xrange(baseId, baseId + EntIdRange):
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
