from direct.directnotify.DirectNotifyGlobal import directNotify
from direct.showbase.DirectObject import DirectObject
from direct.showbase.EventGroup import EventGroup

class InterestWatcher(DirectObject):
    """Object that observes all interests adds/removes over a period of time,
    and sends out an event when all of those interests have closed"""
    notify = directNotify.newCategory('InterestWatcher')
    
    def __init__(self, interestMgr, name, doneEvent=None,
                 recurse=True, start=True, mustCollect=False, doCollectionMgr=None):
        DirectObject.__init__(self)
        self._interestMgr = interestMgr
        if doCollectionMgr is None:
            doCollectionMgr = interestMgr
        self._doCollectionMgr = doCollectionMgr
        self._eGroup = EventGroup(name, doneEvent=doneEvent)
        self._doneEvent = self._eGroup.getDoneEvent()
        self._gotEvent = False
        self._recurse = recurse
        if self._recurse:
            # this will hold a dict of parentId to set(zoneIds) that are closing
            self.closingParent2zones = {}
        if start:
            self.startCollect(mustCollect)

    def startCollect(self, mustCollect=False):
        self._mustCollect = mustCollect

        self.accept(self._interestMgr._getAddInterestEvent(), self._handleInterestOpenEvent)
        self.accept(self._interestMgr._getRemoveInterestEvent(), self._handleInterestCloseEvent)
        
    def stopCollect(self):
        self.ignore(self._interestMgr._getAddInterestEvent())
        self.ignore(self._interestMgr._getRemoveInterestEvent())

        mustCollect = self._mustCollect
        del self._mustCollect
        if not self._gotEvent:
            if mustCollect:
                logFunc = self.notify.error
            else:
                logFunc = self.notify.warning
            logFunc('%s: empty interest-complete set' % self.getName())
            self.destroy()
        else:
            self.accept(self.getDoneEvent(), self.destroy)

    def destroy(self):
        if hasattr(self, '_eGroup'):
            self._eGroup.destroy()
            del self._eGroup
            del self._gotEvent
            del self._interestMgr
            self.ignoreAll()

    def getName(self):
        return self._eGroup.getName()
    def getDoneEvent(self):
        return self._doneEvent

    def _handleInterestOpenEvent(self, event):
        self._gotEvent = True
        self._eGroup.addEvent(event)
    def _handleInterestCloseEvent(self, event, parentId, zoneIdList):
        self._gotEvent = True
        self._eGroup.addEvent(event)
        """
        if self._recurse:
            # this interest is in the process of closing. If an interest
            # underneath any objects in that interest close, we need to know
            # about it.
            self.closingParent2zones.setdefault(parentId, set())
            self.closingParent2zones[parentId].union(set(zoneIdList))
            """
