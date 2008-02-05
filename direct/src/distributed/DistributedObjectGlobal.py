"""DistributedObjectGlobal module: contains the DistributedObjectGlobal class"""

from direct.directnotify.DirectNotifyGlobal import directNotify
from direct.distributed.DistributedObject import DistributedObject

class DistributedObjectGlobal(DistributedObject):
    """
    The Distributed Object Global class is the base class for global
    network based (i.e. distributed) objects.
    """
    notify = directNotify.newCategory("DistributedObjectGlobal")

    # A few objects will set neverDisable to 1... Examples are
    # localToon, and anything that lives in the UberZone. This
    # keeps them from being disabled when you change zones,
    # even to the quiet zone.
    neverDisable = 1

    def __init__(self, cr):
        assert self.notify.debugStateCall(self)
        DistributedObject.__init__(self, cr)
        self.parentId = 0
        self.zoneId = 0

