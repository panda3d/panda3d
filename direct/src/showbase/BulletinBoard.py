"""Contains the BulletinBoard class."""

__all__ = ['BulletinBoard']

from direct.directnotify import DirectNotifyGlobal

class BulletinBoard:
    """This class implements a global location for key/value pairs to be
    stored. Intended to prevent coders from putting global variables directly
    on showbase, so that potential name collisions can be more easily
    detected."""
    notify = DirectNotifyGlobal.directNotify.newCategory('BulletinBoard')

    def __init__(self):
        self._dict = {}

    def get(self, postName, default=None):
        return self._dict.get(postName, default)

    def has(self, postName):
        return postName in self._dict

    def getEvent(self, postName):
        return 'bboard-%s' % postName

    def getRemoveEvent(self, postName):
        return 'bboard-remove-%s' % postName

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
        messenger.send(self.getEvent(postName))

    def remove(self, postName):
        if postName in self._dict:
            del self._dict[postName]
            messenger.send(self.getRemoveEvent(postName))

    def removeIfEqual(self, postName, value):
        # only remove the post if its value is a particular value
        if self.has(postName):
            if self.get(postName) == value:
                self.remove(postName)

    def __repr__(self):
        str  = 'Bulletin Board Contents\n'
        str += '======================='
        keys = list(self._dict.keys())
        keys.sort()
        for postName in keys:
            str += '\n%s: %s' % (postName, self._dict[postName])
        return str
