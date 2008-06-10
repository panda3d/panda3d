"""DelayDelete module: contains the DelayDelete class"""

class DelayDelete:
    """
    The DelayDelete class is a special class whose sole purpose is
    management of the DistributedObject.delayDelete() counter.

    Normally, a DistributedObject has a delayDelete count of 0.  When
    we need to bracket a region of code that cannot tolerate a
    DistributedObject being deleted, we call do.delayDelete(1) to
    increment the delayDelete count by 1.  While the count is nonzero,
    the object will not be deleted.  Outside of our critical code, we
    call do.delayDelete(0) to decrement the delayDelete count and
    allow the object to be deleted once again.

    Explicit management of this counter is tedious and risky.  This
    class implicitly manages the counter by incrementing the count in
    the constructor, and decrementing it in the destructor.  This
    guarantees that every increment is matched up by a corresponding
    decrement.

    Thus, to temporarily protect a DistributedObject from deletion,
    simply create a DelayDelete object.  While the DelayDelete object
    exists, the DistributedObject will not be deleted; when the
    DelayDelete object ceases to exist, it may be deleted.
    """

    def __init__(self, distObj, name):
        self._distObj = distObj
        self._name = name
        self._token = self._distObj.acquireDelayDelete(name)

    def getObject(self):
        return self._distObj

    def getName(self):
        return self._name

    def destroy(self):
        token = self._token
        # do this first to catch cases where releaseDelayDelete causes
        # this method to be called again
        del self._token
        self._distObj.releaseDelayDelete(token)
        del self._distObj
        del self._name
