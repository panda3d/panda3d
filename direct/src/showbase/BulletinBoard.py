from direct.directnotify import DirectNotifyGlobal

# TODO: add callback mechanism when values change.
# Should we announce every change through the messenger?
# Should you be able to hang a hook on a particular name?

class BulletinBoard:
    """This class implements a global location for key/value pairs to be
    stored. Intended to prevent coders from putting global variables directly
    on showbase, so that potential name collisions can be more easily
    detected."""
    notify = DirectNotifyGlobal.directNotify.newCategory('BulletinBoard')

    def __init__(self):
        self._dict = {}

    def get(self, postName):
        return self._dict.get(postName)

    def has(self, postName):
        return postName in self._dict

    def getEventName(self, postName):
        return 'bboard-%s' % postName

    def post(self, postName, value=None):
        if postName in self._dict:
            BulletinBoard.notify.warning('changing %s from %s to %s' % (
                postName, self._dict[postName], value))
        self.update(postName, value)

    def update(self, postName, value):
        """can use this to set value the first time"""
        if postName in self._dict:
            BulletinBoard.notify.info('update: posting %s' % (postName))
        self._dict[postName] = value
        messenger.send(self.getEventName(postName))
        
    def remove(self, postName):
        if postName in self._dict:
            del self._dict[postName]

    def removeIfEqual(self, postName, value):
        # only remove the post if its value is a particular value
        if self.has(postName):
            if self.get(postName) == value:
                self.remove(postName)

    def __repr__(self):
        return str(self._dict)
