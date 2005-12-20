
from direct.showbase.DirectObject import DirectObject
#from direct.directnotify.DirectNotifyGlobal import directNotify

class DistributedObjectBase(DirectObject):
    """
    The Distributed Object class is the base class for all network based
    (i.e. distributed) objects.  These will usually (always?) have a
    dclass entry in a *.dc file.
    """
    notify = directNotify.newCategory("DistributedObjectBase")

    def __init__(self, cr):
        assert self.notify.debugStateCall(self)
        self.cr = cr
        self.children = {}
        self.parentId = None
        self.zoneId = None

    if __debug__:
        def status(self, indent=0):
            """
            print out "doId(parentId,zoneId) className"
            """
            spaces=' '*(indent+2)
            try:
                print "%s%s:"%(
                    ' '*indent, self.__class__.__name__)
                print "%sfrom DistributedObject doId:%s, parent:%s, zone:%s"%(
                    spaces, 
                    self.doId, self.parentId, self.zoneId),
            except Exception, e: print "%serror printing status"%(spaces,), e

    def getLocation(self):
        try:
            if self.parentId == 0 and self.zoneId == 0:
                return None
            # This is a -1 stuffed into a uint32
            if self.parentId == 0xffffffff and self.zoneId == 0xffffffff:
                return None
            return (self.parentId, self.zoneId)
        except AttributeError:
            return None

    def handleChildArrive(self, childObj, zoneId):
        self.notify.debugCall()
        # A new child has just setLocation beneath us.  Give us a
        # chance to run code when a new child sets location to us. For
        # example, we may want to scene graph reparent the child to
        # some subnode we own.
        ## zone=self.children.setdefault(zoneId, {})
        ## zone[childObj.doId]=childObj

        # Inheritors should override
        pass

    def handleChildLeave(self, childObj, zoneId):
        self.notify.debugCall()
        # A child is about to setLocation away from us.  Give us a
        # chance to run code just before a child sets location away from us.
        ## zone=self.children[zoneId]
        ## del zone[childObj.doId]
        ## if not len(zone):
        ##     del self.children[zoneId]

        # Inheritors should override
        pass

    def getParentObj(self):
        if self.parentId is None:
            return None
        return self.cr.doId2do.get(self.parentId)
