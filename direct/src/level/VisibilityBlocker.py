"""VisibilityBlocker module: contains the VisibilityBlocker class"""

import Entity

class VisibilityBlocker:
    """This is a mixin class for level entities (see Entity.py) that in some
    way 'block' visibility (such as doors) -- entities that can completely
    obscure what's behind them. It provides the blocker with a mechanism
    whereby they are informed of when it is safe for them to 'unblock' the
    visibility and show what's behind them. Without this mechanism, the
    blocker might show what's behind it before all of the distributed objects
    behind it have been generated."""
    def __init__(self):
        self.__nextSetZoneDoneEvent=None

    def destroy(self):
        self.cancelUnblockVis()

    def requestUnblockVis(self):
        """derived class should call this before the end of the frame in which
        they cause the visibility to be extended. okToUnblockVis (see below)
        will be called when it's safe to show the new zones."""
        if self.__nextSetZoneDoneEvent is None:
            self.__nextSetZoneDoneEvent = self.level.cr.getNextSetZoneDoneEvent()
            self.acceptOnce(self.__nextSetZoneDoneEvent, self.okToUnblockVis)
            # make sure that a setZone is sent this frame, even if the
            # visibility list doesn't change
            self.level.forceSetZoneThisFrame()

    def cancelUnblockVis(self):
        """
        derived class should call this if they have called
        requestUnblockVis, but no longer need that service.  For example
        the user could have canceled the request that started the
        visibility change.
        """
        if self.__nextSetZoneDoneEvent is not None:
            self.ignore(self.__nextSetZoneDoneEvent)
            self.__nextSetZoneDoneEvent = None

    def isWaitingForUnblockVis(self):
        """
        returns a boolean for whether there is a requestUnblockVis() pending.
        """
        return self.__nextSetZoneDoneEvent is not None

    def okToUnblockVis(self):
        """
        derived class should override this func and do the vis unblock
        (i.e. open the door, etc.)
        """
        self.cancelUnblockVis()
