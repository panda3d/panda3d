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
        # this may do something in the future; please call down to it
        pass

    def destroy(self):
        self.ignoreAll()

    def requestUnblockVis(self):
        """derived class should call this before the end of the frame in which
        they cause the visibility to be extended. okToUnblockVis (see below)
        will be called when it's safe to show the new zones."""
        self.accept(self.level.cr.getNextSetZoneDoneEvent(),
                    self.okToUnblockVis)

    def okToUnblockVis(self):
        """derived class should override this func and do the vis unblock
        (i.e. open the door, etc.)"""
        # this may do something in the future; please call down to it
        pass
