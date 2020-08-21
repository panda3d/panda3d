"""Contains the BulletinBoardWatcher class."""

__all__ = ['BulletinBoardWatcher']

from direct.directnotify import DirectNotifyGlobal
from direct.showbase.PythonUtil import Functor, makeList
from direct.showbase import DirectObject

class BulletinBoardWatcher(DirectObject.DirectObject):
    """ This class allows you to wait for a set of posts to be made to (or
    removed from) the bulletin board, and gives you a notification when all
    posts have been made. Values of posts are not examined."""
    notify = DirectNotifyGlobal.directNotify.newCategory('BulletinBoardWatcher')

    def __init__(self, name, postNames, callback, removeNames=None):
        self.notify.debug('__init__: %s, %s, %s' % (name, postNames, callback))
        if removeNames is None:
            removeNames = []
        self.name = name
        self.postNames = makeList(postNames)
        self.removeNames = makeList(removeNames)
        self.callback = callback
        self.waitingOn = set()
        for name in self.postNames:
            if not bboard.has(name):
                eventName = bboard.getEvent(name)
                self.waitingOn.add(eventName)
                self.acceptOnce(eventName, Functor(self._handleEvent, eventName))
        for name in self.removeNames:
            if bboard.has(name):
                eventName = bboard.getRemoveEvent(name)
                self.waitingOn.add(eventName)
                self.acceptOnce(eventName, Functor(self._handleEvent, eventName))
        self._checkDone()

    def destroy(self):
        self.ignoreAll()
        if hasattr(self, 'callback'):
            del self.callback
            del self.waitingOn

    def isDone(self):
        return len(self.waitingOn) == 0

    def _checkDone(self):
        if self.isDone():
            self.notify.debug('%s: done' % self.name)
            self.callback()
            self.destroy()

    def _handleEvent(self, eventName):
        self.notify.debug('%s: handlePost(%s)' % (self.name, eventName))
        assert eventName in self.waitingOn
        self.waitingOn.remove(eventName)
        self._checkDone()
